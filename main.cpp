#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <list>
#include <limits>
#include <set>

using namespace std;

#ifdef __APPLE__
    typedef unsigned int uint;
#endif
   
/**
 * USEFUL DEFINITIONS
 */
#define UNDEF                   -1
#define TRUE                    1
#define FALSE                   0
#define index(lit)              lit + numVars
#define var(lit)                abs(lit)
#define maxV(x, y)              x > y ? x : y
#define numLearntClauses()      (int)(learntClauses.size() - modelStack.size() + decisionLevel)

/**
 * CONFIG DEFINITIONS
 */
#define INIT_VARIABLE_BUMP      10
#define INIT_CLAUSE_BUMP        10

/**
 * TYPE DEFINITIONS
 */
struct Variable;
struct Clause;
struct LearntClause;
typedef long long unsigned int heuristic;

/**
 * GLOBAL VARIABLES
 */
uint numVars;                           // Number of variables
uint numClauses;                        // Number of clauses
uint indexOfNextLitToPropagate;         // Index of the next literal to propagate in model
uint decisionLevel;                     // Number of decisions made

vector<Variable> variables;             // Problem variables
vector<Clause*> clauses;                // Problem clauses
vector<LearntClause*> learntClauses;    // Clauses learnt from conflicts
vector<int> model;                      // Model that represents the solution to the problem
vector<int> modelStack;                 // Chronological trail of the literals set to true

/**
 * For every literal lit, it contains the clauses where -lit is a watched literal.
 * Every clause has two watched literals (the ones placed in the first and second positions).
 * If one watched literal is set to false, then we need to find a new literal in the clause
 * that is not false and update its position in the clause.
 */
vector< list<Clause*> > watches;

/**
 * For every literal lit: occurrences[index(lit)] contains the number of appearances of lit
 * in the problem clauses.
 */
vector<uint> occurrences;

/**
 * Heuristic configuration
 */
int maxLearntClauses = 10;              // Initial max number of clauses that can be learned
const int maxLearntClausesLimit = 300;  // Limits the maxLearntClauses number

/**
 * reduceCounter counts the times that the learnt clauses have been reduced before
 * incrementing the number of maxLearntClauses (triggered by reduceCounterLimit).
 */
int reduceCounter = 0;
const int reduceCounterLimit = 5;

heuristic variableBump = INIT_VARIABLE_BUMP;    // Initial variable bump
heuristic clauseBump   = INIT_CLAUSE_BUMP;      // Initial clause bump

/**
 * Maximum activity that any variable or clause can have.
 */
const heuristic maxActivity = numeric_limits<heuristic>::max();

/**
 * Represents the increment that should be performed to the variable/clause bumps.
 */
const heuristic activityInc = 1;

/**
 * HEADER DEFINITONS (needed in some classes)
 */
int currentValueInModel(int lit);
void bumpClauseActivity(LearntClause* clause);

/**
 * CLASS DEFINITIONS
 */

/**
 * Represents a problem variable.
 */
struct Variable
{
    /**
     * Activity heuristic of the variable
     */
    heuristic activity;
    
    /**
     * Decision level where variable was set
     */
    uint level;
    
    /**
     * Reason to set the variable
     */
    Clause* reasonClause;
    
    /**
     * Default constructor
     */
    Variable() : activity(0), level(-1), reasonClause(NULL)
    { }
};

struct VariableCompare
{
    bool operator()(const int &a, const int &b) const
    {
        if(variables[a].activity > variables[b].activity)
            return true;
        
        if(variables[b].activity > variables[a].activity)
            return false;
        
        return a < b;
    }
};

/**
 * Represents a problem clause.
 */
struct Clause
{
    /**
     * Literals of the clause
     */
    vector<int> literals;
    
    /**
     * Tries to find a new second watched literal.
     * @return True if found, false otherwise
     */
    inline bool findNewWatcher()
    {
        for(int i = 2; i < literals.size(); ++i)
        {
            if(currentValueInModel(literals[i]) != FALSE)
            {
                int literal = literals[1];
                literals[1] = literals[i];
                literals[i] = literal;

                watches[index(-literals[1])].push_back(this);

                return true;
            }
        }

        return false;
    }
    
    /**
     * Pushes in reason the literals of the clause negated.
     * @param lit If defined, first literal of the clause is skipped
     * @param reason Where to push the literals
     */
    virtual inline void calcReason(int lit, vector<int> &reason)
    {
        for(int i = (lit == UNDEF ? 0 : 1); i < literals.size(); ++i)
            reason.push_back(-literals[i]);
    }
    
    /**
     * Removes the clause, updating the watched literals structure accordingly.
     */
    inline void remove()
    {
        watches[index(-literals[0])].remove(this);
        watches[index(-literals[1])].remove(this);
        
        delete this;
    }
    
};

/**
 * A LearntClause is a Clause created by analysis of a conflict.
 */
struct LearntClause : Clause
{
    /**
     * Activity heuristic of the learnt clause.
     */
    heuristic activity;
    
    /**
     * Pushes in reason the literals of the clause negated.
     * Bumps the clause activity.
     * @param lit If defined, first literal of the clause is skipped
     * @param reason Where to push the literals
     */
    virtual inline void calcReason(int lit, vector<int>& reason)
    {
        Clause::calcReason(lit, reason);
        
        bumpClauseActivity(this);
    }
    
    /**
     * Tells whether or not the learnt clause is locked.
     * A learnt clause is locked if is the current reason for some variable.
     * A locked clause can not be removed when reducing.
     */
    bool locked() const
    {
        return variables[var(literals[0])].reasonClause == this;
    }
    
    /**
     * Sort function used to sort the learnt clauses by activity in a strict weak ordering
     * relation.
     * @param a First clause to compare
     * @param b Second clause to compare
     * @return True if a must be placed before b, false otherwise
     */
    static bool sortByActivityDesc(const LearntClause* a, const LearntClause* b)
    {
        bool lock_a = a->locked();
        bool lock_b = b->locked();
        
        if(lock_a && not lock_b)
            return true;
        
        if(lock_b && not lock_a)
            return false;
        
        if(a->activity > b->activity)
            return true;
        
        if(b->activity > a->activity)
            return false;
        
        return false;
    }
};

void readClauses()
{
    // Skip comments
    char c = cin.get();
    while(c == 'c')
    {
        while(c != '\n') c = cin.get();
        c = cin.get();
    }
    
    // Read "cnf numVars numClauses"
    string aux;
    cin >> aux >> numVars >> numClauses;
    
    // Init data structures
    variables.resize(numVars + 1);
    watches.resize(numVars*2 + 1);
    occurrences.resize(numVars*2 + 1, 0);
    
    // Read clauses
    for(uint i = 0; i < numClauses; ++i)
    {
        Clause* clause = new Clause();
        
        int lit;
        
        while(cin >> lit and lit != 0)
        {
            clause->literals.push_back(lit);
            
            if(clause->literals.size() <= 2)
                watches[index(-lit)].push_back(clause);
            
            occurrences[index(lit)]++;
        }
        
        clauses.push_back(clause);
    }
        
    for(int i = 1; i <= numVars; ++i)
        variables[i].activity = (occurrences[index(i)] + occurrences[index(-i)]) / (numVars / 60);
    
    variableBump = numClauses / 2;
}

inline int currentValueInModel(int lit)
{
    if(lit >= 0)
        return model[lit];
    
    if(model[-lit] == UNDEF)
        return UNDEF;
    
    return 1 - model[-lit];
}

inline void setLiteralToTrue(int lit)
{
    modelStack.push_back(lit);
    
    int id = abs(lit);
    
    if(lit > 0)
        model[id] = TRUE;
    else
        model[id] = FALSE;
    
    variables[var(lit)].level = decisionLevel;
}

void rescaleVariableActivity()
{
    heuristic scaleFactor = maxActivity / (numVars * 15);
    
    for(int i = 1; i <= numVars; ++i)
        variables[i].activity /= scaleFactor;
    
    variableBump = INIT_VARIABLE_BUMP * 15;
}

void bumpVariableActivity(int variable)
{
    if(variables[variable].activity > (maxActivity - variableBump))
        rescaleVariableActivity();
    
    variables[variable].activity += variableBump;
}

void rescaleClauseActivity()
{
    heuristic scaleFactor = maxActivity / (learntClauses.size() * 15);
    
    for(int i = 0; i < learntClauses.size(); ++i)
        learntClauses[i]->activity /= scaleFactor;
    
    clauseBump = INIT_CLAUSE_BUMP * 15;
}

void bumpClauseActivity(LearntClause* clause)
{
    if(clause->activity > (maxActivity - clauseBump))
        rescaleClauseActivity();
    
    clause->activity += clauseBump;
}

bool propagateGivesConflict(int literal, list<Clause*> &watchClauses, Clause* &conflictClause)
{
    list<Clause*>::iterator it = watchClauses.begin();
    
    while(it != watchClauses.end())
    {
        Clause &clause = *(*it); // it -> Clause pointer -> Clause
        
        // Make sure false literal is at second position
        if(clause.literals[0] == literal)
        {
            clause.literals[0] = clause.literals[1];
            clause.literals[1] = literal;
        }
        
        int firstWatchValue = currentValueInModel(clause.literals[0]);
        
        // If first watch is true, then clause is satisfied
        if(firstWatchValue == TRUE)
        {
            ++it;
            continue;
        }
        
        if(clause.findNewWatcher())
        {
            it = watchClauses.erase(it);
            continue;
        }
        
        if(firstWatchValue == FALSE)
        {
            conflictClause = *it;
            
            for(int i = 0; i < clause.literals.size(); ++i)
                bumpVariableActivity(var(clause.literals[i]));
            
            variableBump += activityInc; // Increment variable bump
            clauseBump   += activityInc;
            return true;
        }
        
        setLiteralToTrue(clause.literals[0]); // Propagate
        variables[var(clause.literals[0])].reasonClause = *it;
        ++it;
    }
    
    return false;
}

bool propagateGivesConflict(Clause* &conflictClause)
{
    while(indexOfNextLitToPropagate < modelStack.size())
    {
        int lit = modelStack[indexOfNextLitToPropagate];
        
        if(propagateGivesConflict(-lit, watches[index(lit)], conflictClause))
            return true;
        
        ++indexOfNextLitToPropagate;
    }
    
    return false;
}

inline void undoOne()
{
    int lit = modelStack.back();
    int x = var(lit);
    modelStack.pop_back();
    
    variables[x].level = -1;
    variables[x].reasonClause = NULL;
    model[x] = UNDEF;
}

inline void backtrack(int level)
{
    while(decisionLevel > level)
    {
        while(modelStack.back() != 0)
            undoOne();
        
        modelStack.pop_back(); // remove the DL mark
        indexOfNextLitToPropagate = modelStack.size();
        --decisionLevel;
    }
}

LearntClause* analyze(Clause* conflict, int &btLevel)
{
    vector<bool> seen(numVars + 1, false);
    int counter = 0;
    int lit = UNDEF;
    vector<int> lit_reason;
    
    btLevel = 0;
    
    LearntClause* learnt = new LearntClause();
    learnt->literals.push_back(0);
    
    int max_i = 1;
    
    do
    {
        lit_reason.clear();
        conflict->calcReason(lit, lit_reason);
        
        for(int i = 0; i < lit_reason.size(); ++i)
        {
            int q = lit_reason[i];
            Variable& vq = variables[var(q)];
            
            if(not seen[var(q)])
            {
                seen[var(q)] = true;
                
                if(vq.level == decisionLevel)
                    counter++;
                else if(vq.level > 0)
                {
                    learnt->literals.push_back(-q);
                    
                    if(btLevel < vq.level)
                    {
                        btLevel = vq.level;
                        max_i = learnt->literals.size();
                    }
                }
            }
        }
        
        do
        {
            lit = modelStack.back();
            conflict = variables[var(lit)].reasonClause;
            undoOne();
        }
        while(not seen[var(lit)]);
        
        counter--;
    }
    while(counter > 0);
    
    learnt->literals[0] = -lit;
    
    if(max_i > 1)
    {
        int aux = learnt->literals[1];
        learnt->literals[1] = learnt->literals[max_i-1];
        learnt->literals[max_i-1] = aux;
    }
    
    return learnt;
}

void learn(LearntClause* clause)
{
    if(clause->literals.size() > 1)
    {
        clause->activity = 0;
        bumpClauseActivity(clause);
        
        watches[index(-clause->literals[0])].push_back(clause);
        watches[index(-clause->literals[1])].push_back(clause);
        variables[var(clause->literals[0])].reasonClause = clause;
        
        learntClauses.push_back(clause);
    }
    
    setLiteralToTrue(clause->literals[0]);
}

int getNextDecisionLiteral()
{
    int var = 0;
    long long int max = 0;
    
    for(uint i = 1; i <= numVars; ++i)
    {
        if(model[i] == UNDEF)
        {
            if(max < variables[i].activity)
            {
                max = variables[i].activity;
                var = i;
            }
        }
    }
    
    if(var == 0)
        return 0;
    
    if(occurrences[index(var)] > occurrences[index(-var)])
        return var;
    else
        return -var;
}

void checkmodel()
{
    for(int i = 0; i < numClauses; ++i)
    {
        bool someTrue = false;
        
        for(int j = 0; not someTrue and j < clauses[i]->literals.size(); ++j)
            someTrue = (currentValueInModel(clauses[i]->literals[j]) == TRUE);
        
        if(not someTrue)
        {
            cout << "Error in model, clause is not satisfied:";
            
            for(int j = 0; j < clauses[i]->literals.size(); ++j)
                cout << clauses[i]->literals[j] << " ";
            
            cout << endl;
            exit(1);
        }
    }
}

void reduceLearntClauses()
{
    // Sort clauses
    sort(learntClauses.begin(), learntClauses.end(), LearntClause::sortByActivityDesc);

    int remove = maxLearntClauses / 2; // Remove the half part
    int i = 0;
    
    LearntClause* clause = learntClauses.back();
    while(i < remove and not clause->locked())
    {
        learntClauses.pop_back();
        clause->remove();
        
        clause = learntClauses.back();
        i++;
    }
}

int main()
{
    readClauses(); // reads numVars, numClauses and clauses
    model.resize(numVars + 1, UNDEF);
    indexOfNextLitToPropagate = 0;
    decisionLevel = 0;

    // Take care of initial unit clauses, if any
    for(uint i = 0; i < numClauses; ++i)
    {
        if(clauses[i]->literals.size() == 1)
        {
            int lit = clauses[i]->literals[0];
            int val = currentValueInModel(lit);
            
            if(val == FALSE)
            {
                cout << "UNSATISFIABLE" << endl;
                return 10;
            }
            else if(val == UNDEF)
                setLiteralToTrue(lit);
        }
    }
    
    Clause* conflictClause;
    int backtrackLevel;
    
    // DPLL algorithm
    while(true)
    {
        while(propagateGivesConflict(conflictClause))
        {
            if(decisionLevel == 0)
            {
                cout << "UNSATISFIABLE" << endl;
                return 10;
            }
            
            LearntClause* learntClause = analyze(conflictClause, backtrackLevel);
            backtrack(backtrackLevel);
            learn(learntClause);
        }

        if(numLearntClauses() > maxLearntClauses)
        {
            reduceLearntClauses();
            
            if(maxLearntClauses <= maxLearntClausesLimit)
            {
                if(reduceCounter >= reduceCounterLimit)
                {
                    maxLearntClauses ++;
                    reduceCounter = 0;
                }
                
                reduceCounter++;
            }
        }
        
        int decisionLit = getNextDecisionLiteral();
        
        if(decisionLit == 0)
        {
            checkmodel();
            cout << "SATISFIABLE" << endl;
            return 20;
        }
        
        // start new decision level:
        modelStack.push_back(0); // push mark indicating new DL
        ++indexOfNextLitToPropagate;
        ++decisionLevel;
        
        setLiteralToTrue(decisionLit); // now push decisionLit on top of the mark
    }
}

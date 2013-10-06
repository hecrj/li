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
#define INIT_VARIABLE_BUMP      (numClauses / 2)
#define INIT_CLAUSE_BUMP        10
#define VARS_TO_ENABLE_SET      3000

/**
 * TYPE DEFINITIONS
 */
struct Variable;
struct Clause;
struct LearntClause;
typedef unsigned int heuristic;

/**
 * GLOBAL VARIABLES
 */
uint numVars;                           // Number of variables
uint numClauses;                        // Number of clauses
uint indexOfNextLitToPropagate;         // Index of the next literal to propagate in model
uint decisionLevel;                     // Current decision level

#ifdef VERBOSE
uint decisionCount;                     // Total number of decisions made
uint conflictCount;                     // Total number of conflicts detected
uint propagationCount;                  // Total number of propagations performed
#endif

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

bool variableSetEnabled = false;

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

heuristic variableBump;                 // Variable heuristic bump
heuristic clauseBump;                   // Clause heuristic bump

/**
 * Maximum activity that any variable or clause can have.
 */
const heuristic maxActivity = numeric_limits<unsigned int>::max();

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

/**
 * Establishes a strict weak ordering relation between variable indexes.
 * @param a Variable index
 * @param b Variable index
 * @return True if a must be placed before b, false otherwise
 */
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
 * Set of variable indexes used when the problem has a huge number of variables to
 * avoid walking through the whole variable vector.
 */
set<int, VariableCompare> variableSet;

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

/**
 * Reads the clauses of the problem and initializes data structures.
 */
void readClauses()
{
    // Skip comments
    char c = cin.get();
    while(c == 'c')
    {
        while(c != '\n')
            c = cin.get();
        
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
    
    variableSetEnabled = (numVars >= VARS_TO_ENABLE_SET);
    
    // Initalize heuristics based in literal occurrences and the number of clauses
    int scaleFactor = (numVars / 60) ? : 1;
    
    for(int i = 1; i <= numVars; ++i)
    {
        variables[i].activity = (occurrences[index(i)] + occurrences[index(-i)]) / scaleFactor;
    }
    
    if(variableSetEnabled)
    {
        for(int i = 0; i <= numVars; ++i)
            variableSet.insert(i);
    }
    
    variableBump = INIT_VARIABLE_BUMP;
    clauseBump = INIT_CLAUSE_BUMP;
    
#ifdef VERBOSE
    decisionCount = propagationCount = conflictCount = 0;
#endif
}

/**
 * Returns the current value in model of a given literal.
 * @param lit A literal
 * @return The current value in model of lit
 */
inline int currentValueInModel(int lit)
{
    if(lit >= 0)
        return model[lit];
    
    if(model[-lit] == UNDEF)
        return UNDEF;
    
    return 1 - model[-lit];
}

/**
 * Sets a literal to true in the model.
 * @param lit A literal
 */
inline void setLiteralToTrue(int lit)
{
    modelStack.push_back(lit);
    
    int id = abs(lit);
    
    if(lit > 0)
        model[id] = TRUE;
    else
        model[id] = FALSE;
    
    variables[var(lit)].level = decisionLevel;
    
    if(variableSetEnabled)
    {
        set<int, VariableCompare>::const_iterator it = variableSet.find(var(lit));
        variableSet.erase(it);
    }
}

/**
 * Rescales all variable activity into smaller values.
 */
void rescaleVariableActivity()
{
    heuristic scaleFactor = maxActivity / (numVars * 15);
    
    for(int i = 1; i <= numVars; ++i)
        variables[i].activity /= scaleFactor;
    
    variableBump = INIT_VARIABLE_BUMP * 15;
}

/**
 * Bumps the activity of a variable.
 * @param variable A variable
 */
void bumpVariableActivity(int variable)
{
    if(variables[variable].activity > (maxActivity - variableBump))
        rescaleVariableActivity();
    
    variables[variable].activity += variableBump;
}

/**
 * Rescales all the activity of learnt clauses into smaller values.
 */
void rescaleClauseActivity()
{
    heuristic scaleFactor = maxActivity / (learntClauses.size() * 15);
    
    for(int i = 0; i < learntClauses.size(); ++i)
        learntClauses[i]->activity /= scaleFactor;
    
    clauseBump = INIT_CLAUSE_BUMP * 15;
}

/**
 * Bumps the activity of a learnt clause.
 * @param clause A learnt clause
 */
void bumpClauseActivity(LearntClause* clause)
{
    if(clause->activity > (maxActivity - clauseBump))
        rescaleClauseActivity();
    
    clause->activity += clauseBump;
}

/**
 * Given a literal that is currently false and its clauses to watch, for every clause:
 * - If the literal is in the first position of the clause, it's swapped with the
 *   second position.
 * - If the literal in the first position (first watched literal) is currently TRUE, then
 *   the clause is satisfied. Therefore, this clause is skipped.
 * - If first watched literal is not true, then it is needed to find a new second watched
 *   literal. If found, the clause can be skipped.
 * - If no second watched literal is found then it means that all literals from second
 *   position to last position are FALSE. Thus, two cases can occur:
 *      1. First watched literal is FALSE => Clause is false => Conflict detected
 *      2. First watched literal is UNDEF => First watched literal is set to TRUE
 * @param literal A literal that is currently false
 * @param watchClauses Clauses where literal is watched
 * @param conflictClause If a conflict is detected, this contains the conflicting clause
 * @return True if conflict detected, false otherwise
 */
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
        
        // Try to find a new second watched literal
        if(clause.findNewWatcher())
        {
            it = watchClauses.erase(it); // Erase cclause from watchClauses of literal
            continue;
        }
        
        // Conflict case
        if(firstWatchValue == FALSE)
        {
            conflictClause = *it;
            
            // Bump activity of the variables in the conflicting clause
            for(int i = 0; i < clause.literals.size(); ++i)
                bumpVariableActivity(var(clause.literals[i]));
            
            // Increment heuristic bumps
            variableBump += activityInc;
            clauseBump   += activityInc;
            
#ifdef VERBOSE
            conflictCount++;
#endif
            return true;
        }
        
        // firstWatchValue is UNDEF => set it to TRUE
        setLiteralToTrue(clause.literals[0]);
        
        // Update reason clause of the variable set
        variables[var(clause.literals[0])].reasonClause = *it;
        
#ifdef VERBOSE
        propagationCount++;
#endif
        ++it;
    }
    
    return false;
}

/**
 * Propagates the last decision until a new decision is needed or a conflict
 * is detected.
 * @param conflictClause If a conflict is detected it contains the conflicting clause
 * @return True if a conflict is detected, false otherwise
 */
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

/**
 * Sets as UNDEF the last literal set in the model.
 * Pre: modelStack.back() != 0
 */
inline void undoOne()
{
    int lit = modelStack.back();
    int x = var(lit);
    modelStack.pop_back();
    
    variables[x].level = -1;
    variables[x].reasonClause = NULL;
    model[x] = UNDEF;
    
    if(variableSetEnabled)
        variableSet.insert(x);
}

/**
 * Backjumps to the given level.
 * @param level Level where to jump
 */
inline void backjump(int level)
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

/**
 * Analyzes a conflicting clause and generates a new clause that can be added
 * to the problem to avoid the conflict in the future.
 * @param conflict Conflicting clause to analyze
 * @param btLevel Level where to jump
 * @return The generated clause
 */
LearntClause* analyze(Clause* conflict, int &backjumpLevel)
{
    // Keep track of what variables have been seen or not
    vector<bool> seen(numVars + 1, false);
    
    // Tells how much we need to keep analyzing
    int counter = 0;
    
    // Last literal assignment analyzed
    int lit = UNDEF;
    
    // Stores the reason of the last literal
    vector<int> litReason;
    
    // Initialize backjump level
    backjumpLevel = 0;
    
    // Create the new clause
    LearntClause* learnt = new LearntClause();
    
    // Make some space to the first literal
    learnt->literals.push_back(0);
    
    // Contains the index-1 of the literal with highest decision level
    int maxIndex = 1;
    
    do
    {
        litReason.clear();
        // Calculate litReason from conflict
        conflict->calcReason(lit, litReason);
        
        for(int i = 0; i < litReason.size(); ++i)
        {
            int q = litReason[i];
            Variable& vq = variables[var(q)];
            
            // If the variable has been seen, skip it
            if(seen[var(q)])
                continue;
            
            seen[var(q)] = true;
            
            if(vq.level == decisionLevel)
                counter++; // If variable assignment has been in the current decision level,
                           // we need to analyze further
            
            else if(vq.level > 0)
            {
                // Add reason literal to the clause, negated
                learnt->literals.push_back(-q);
                
                // Update backjump level and maxIndex accordingly
                if(backjumpLevel < vq.level)
                {
                    backjumpLevel = vq.level;
                    maxIndex = learnt->literals.size();
                }
            }
        }
        
        do
        {
            // Find next literal whose variable is already seen to analyze further
            lit = modelStack.back();
            conflict = variables[var(lit)].reasonClause;
            undoOne();
        }
        while(not seen[var(lit)]);
        
        counter--;
    }
    while(counter > 0);
    
    // Add last (non-analyzed) literal seen and found in the current decision level
    // to the first position in the clause (first watched literal)
    learnt->literals[0] = -lit;
    
    // If learntClause has more than one literal, move the literal with highest decision
    // level to the second position (second watched literal)
    // This is useful to improve performance.
    if(maxIndex > 1)
    {
        int aux = learnt->literals[1];
        learnt->literals[1] = learnt->literals[maxIndex-1];
        learnt->literals[maxIndex-1] = aux;
    }
    
    return learnt;
}

/**
 * Learns a clause and sets its first literal to true.
 * @param clause The clause to learn
 */
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

/**
 * Selects the most active variable and returns the variable id negated or not depending which
 * of that two literals has more occurrences.
 * @return The described literal
 */
int getNextDecisionLiteral()
{   
    int var = 0;
    
    if(variableSetEnabled)
    {
        set<int, VariableCompare>::const_iterator it = variableSet.begin();
    
        if(it != variableSet.end())
            var = *it;
    }
    else
    {
        heuristic max = 0;

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
    }
    
    if(var == 0)
        return 0;

    if(occurrences[index(var)] > occurrences[index(-var)])
        return var;
    else
        return -var;
}

/**
 * Checks that the current model satisfies the clauses.
 * If some clause is not satisfied, it gets printed and execution terminates.
 */
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

/**
 * Halves the set of learnt clauses, approximately, removing the ones that are less active.
 */
void reduceLearntClauses()
{
    // Sort clauses (locked clauses are on top)
    sort(learntClauses.begin(), learntClauses.end(), LearntClause::sortByActivityDesc);

    int remove = maxLearntClauses / 2; // Remove the half part
    int i = 0;
    
    LearntClause* clause = learntClauses.back();
    
    // Remove the clauses, if current clause is locked => all the rest are locked
    while(i < remove and not clause->locked())
    {
        learntClauses.pop_back();
        clause->remove();
        
        clause = learntClauses.back();
        i++;
    }
}

#ifdef VERBOSE
/**
 * Prints the total number of decisions, conflicts and propagations occurred while
 * solving the problem.
 */
void printSummary()
{
    cout << decisionCount << " decisions" << endl;
    cout << conflictCount << " conflicts" << endl;
    cout << propagationCount << " propagations" << endl;
}
#endif

/**
 * Reads a SAT problem and prints whether is satisfiable or not.
 * @return 10 if unsatisfiable, 20 otherwise
 */
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
    int backjumpLevel;
    
    // DPLL algorithm
    while(true)
    {
        while(propagateGivesConflict(conflictClause))
        {
            if(decisionLevel == 0)
            {
                cout << "UNSATISFIABLE" << endl;
                
#ifdef VERBOSE
                printSummary();
#endif
                
                return 10;
            }
            
            // Analyze conflict
            LearntClause* learntClause = analyze(conflictClause, backjumpLevel);
            
            // Backjump
            backjump(backjumpLevel);
            
            // Learn from conflict
            learn(learntClause);
        }
        
        if(numLearntClauses() > maxLearntClauses)
        {
            // Too much learnt clauses, reducing needed
            reduceLearntClauses();
            
            // If limit of max learnt clauses still not reached
            if(maxLearntClauses <= maxLearntClausesLimit)
            {
                // If we reduced an amount of reduceCounterLimit times
                if(reduceCounter >= reduceCounterLimit)
                {
                    // Increment number of max learnt clauses
                    maxLearntClauses ++;
                    // Reset counter
                    reduceCounter = 0;
                }
                
                reduceCounter++; // Increment counter
            }
        }
        
        int decisionLit = getNextDecisionLiteral();
        
        if(decisionLit == 0)
        {
            checkmodel();
            cout << "SATISFIABLE" << endl;
            
#ifdef VERBOSE
                printSummary();
#endif
            
            return 20;
        }
        
        // start new decision level:
        modelStack.push_back(0); // push mark indicating new DL
        ++indexOfNextLitToPropagate;
        ++decisionLevel;
        
#ifdef VERBOSE
        decisionCount++;
#endif
        setLiteralToTrue(decisionLit); // now push decisionLit on top of the mark
    }
}

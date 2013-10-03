#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <list>
#include <limits>
#include <set>

using namespace std;

#define UNDEF -1
#define TRUE   1
#define FALSE  0

#ifdef __APPLE__
    typedef unsigned int uint;
#endif
   
/**
 * Useful definitions
 */
#define index(lit)              lit + numVars
#define var(lit)                abs(lit)
#define maxV(x, y)              x > y ? x : y
#define numLearntClauses()      (int)(learntClauses.size() - modelStack.size() + decisionLevel)

/**
 * Config definitions
 */
#define MAX_LEARNT_CLAUSES      300
#define INIT_CLAUSE_BUMP        1
    
/**
 * GLOBAL VARIABLES
 */
uint numVars;
uint numClauses;
uint indexOfNextLitToPropagate;
uint decisionLevel;

struct Variable;
struct Clause;

vector<Variable> variables;
vector<Clause*> clauses;
vector<Clause*> learntClauses;

vector< list<Clause*> > watches;
vector<int> model;
vector<int> modelStack;

float clauseBump = INIT_CLAUSE_BUMP;
float clauseInc  = 1.1;
long long unsigned int variableBump = 1;

static float maxActivity = numeric_limits<float>::max();
static float rescaleActivity = numeric_limits<float>::min();

int currentValueInModel(int lit);
void bumpClauseActivity(Clause* clause);

/**
 * CLASSES
 */
struct Variable
{
    uint id;
    long long unsigned int activity;
    uint level;
    Clause* reasonClause;
    
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

set<int, VariableCompare> variableOrder;

struct Clause
{
    vector<int> literals;
    float activity;
    bool learnt;
    
    ~Clause() { }
    
    bool locked() const
    {
        return variables[var(literals[0])].reasonClause == this;
    }
    
    bool findNewWatcher(int literal)
    {
        for(int i = 2; i < literals.size(); ++i)
        {
            if(currentValueInModel(literals[i]) != FALSE)
            {
                literals[1] = literals[i];
                literals[i] = literal;

                watches[index(-literals[1])].push_back(this);

                return true;
            }
        }

        return false;
    }
    
    void calcReason(int lit, vector<int> &reason)
    {
        for(int i = (lit == UNDEF ? 0 : 1); i < literals.size(); ++i)
            reason.push_back(-literals[i]);
        
        if(learnt)
            bumpClauseActivity(this);
    }
    
    void remove()
    {
        watches[index(-literals[0])].remove(this);
        watches[index(-literals[1])].remove(this);
        
        delete this;
    }
    
    static bool sortByActivityDesc(const Clause* a, const Clause* b)
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
    
    // Read clauses
    for(uint i = 0; i < numClauses; ++i)
    {
        Clause* clause = new Clause();
        clause->learnt = false;
        
        int lit;
        
        while(cin >> lit and lit != 0)
        {
            clause->literals.push_back(lit);
            
            if(clause->literals.size() <= 2)
                watches[index(-lit)].push_back(clause);
            
            variables[var(lit)].activity++;
        }
        
        clauses.push_back(clause);
    }
        
    for(int i = 1; i <= numVars; ++i)
        variableOrder.insert(i);
}

int currentValueInModel(int lit)
{
    if(lit >= 0)
        return model[lit];
    
    if(model[-lit] == UNDEF)
        return UNDEF;
    
    return 1 - model[-lit];
}

void setLiteralToTrue(int lit)
{
    modelStack.push_back(lit);
    
    int id = abs(lit);
    
    if(lit > 0)
        model[id] = TRUE;
    else
        model[id] = FALSE;
    
    variables[var(lit)].level = decisionLevel;
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
        
        if(clause.findNewWatcher(literal))
        {
            it = watchClauses.erase(it);
            continue;
        }
        
        if(firstWatchValue == FALSE)
        {
            conflictClause = *it;
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

void undoOne()
{
    int lit = modelStack.back();
    int x = var(lit);
    modelStack.pop_back();
    
    variables[x].level = -1;
    variables[x].reasonClause = NULL;
    model[x] = UNDEF;
}

void backtrack(int level)
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

Clause* analyze(Clause* conflict, int &btLevel)
{
    vector<bool> seen(numVars + 1, false);
    int counter = 0;
    int lit = UNDEF;
    vector<int> lit_reason;
    
    btLevel = 0;
    
    Clause* learnt = new Clause();
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

void rescaleClauseActivity()
{
    for(int i = 0; i < learntClauses.size(); ++i)
        learntClauses[i]->activity *= rescaleActivity;
    
    clauseBump = INIT_CLAUSE_BUMP;
}

void bumpClauseActivity(Clause* clause)
{
    float activity = clause->activity + clauseBump;
    
    if(activity > maxActivity)
    {
        rescaleClauseActivity();
        
        clause->activity += clauseBump;
    }
    else
        clause->activity = activity;
}

void bumpVariableActivity(int variable)
{
    set<int>::const_iterator it = variableOrder.find(variable);
    variableOrder.erase(it);
    
    variables[variable].activity += clauseBump;
    variableOrder.insert(variable);
}

void learn(Clause* clause)
{   
    if(clause->literals.size() > 1)
    {
        clause->learnt = true;
        clause->activity = 0;
        bumpClauseActivity(clause);
        
        watches[index(-clause->literals[0])].push_back(clause);
        watches[index(-clause->literals[1])].push_back(clause);
        variables[var(clause->literals[0])].reasonClause = clause;
        
        learntClauses.push_back(clause);
    }
    
    for(int i = 0; i < clause->literals.size(); ++i)
        bumpVariableActivity(var(clause->literals[i]));
    
    setLiteralToTrue(clause->literals[0]);
}

int getNextDecisionLiteral()
{
    set<int>::const_iterator it = variableOrder.begin();
    
    while(it != variableOrder.end())
    {
        if(model[*it] == UNDEF)
            return *it;
        
        ++it;
    }
    
    return 0; // returns 0 when all literals are defined
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
    sort(learntClauses.begin(), learntClauses.end(), Clause::sortByActivityDesc);

    int remove = MAX_LEARNT_CLAUSES / 2; // Remove the half part
    int i = 0;
    
    Clause* clause = learntClauses.back();
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
            
            Clause* learntClause = analyze(conflictClause, backtrackLevel);
            backtrack(backtrackLevel);
            learn(learntClause);
            clauseBump *= clauseInc;
            variableBump++;
        }
        
        if(numLearntClauses() > MAX_LEARNT_CLAUSES)
            reduceLearntClauses();
            
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

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <list>

using namespace std;

#define UNDEF -1
#define TRUE   1
#define FALSE  0

#ifdef __APPLE__
    typedef unsigned int uint;
#endif
    
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
vector< list<uint> > watches;
vector<Clause> clauses;
vector<int> model;
vector<int> modelStack;


/**
 * CLASSES
 */
struct Variable
{
    uint id;
    uint occurrences;
    uint level;
    uint reasonClause;
    
    Variable()
    {
        id = 0;
        occurrences = 0;
        level = -1;
        reasonClause = -1;
    }
};

struct Clause
{
    uint id;
    vector<int> literals;
};

/**
 * Useful definitions
 */
#define index(lit) lit + numVars
#define var(lit)   abs(lit)
#define max(x, y)  x > y ? x : y   

bool sortLiteralsByOccurrencesDesc(const Variable &a, const Variable &b)
{
    return a.occurrences > b.occurrences;
}

template<class T> void print(vector<T> &v)
{
    for(int i = 0; i < v.size(); ++i)
        cout << v[i] << ' ';
    
    cout << endl;
}

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
    clauses.resize(numClauses);
    variables.resize(numVars + 1);
    watches.resize(numVars*2 + 1);
    
    for(int i = 1; i <= numVars; ++i)
        variables[i].id = i;
    
    // Read clauses
    for(uint i = 0; i < numClauses; ++i)
    {
        clauses[i].id = i;
        
        int lit;
        
        while(cin >> lit and lit != 0)
        {
            clauses[i].literals.push_back(lit);
            
            if(clauses[i].literals.size() <= 2)
                watches[index(-lit)].push_back(i);
            
            variables[var(lit)].occurrences++;
        }
    }
    
    // Sort literals by number of occurrences
    sort(variables.begin(), variables.end(), sortLiteralsByOccurrencesDesc);
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

bool findNewWatcher(int literal, Clause &clause)
{
    for(int i = 2; i < clause.literals.size(); ++i)
    {
        if(currentValueInModel(clause.literals[i]) != FALSE)
        {
            clause.literals[1] = clause.literals[i];
            clause.literals[i] = literal;
            
            watches[index(-clause.literals[1])].push_back(clause.id);

            return true;
        }
    }

    return false;
}

bool propagateGivesConflict(int literal, list<uint> &watchClauses, int &conflictClause)
{
    list<uint>::iterator it = watchClauses.begin();
    
    while(it != watchClauses.end())
    {
        Clause &clause = clauses[*it];
        
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
        
        if(findNewWatcher(literal, clause))
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

bool propagateGivesConflict(int &conflictClause)
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
    variables[x].reasonClause = -1;
    model[x] = UNDEF;
    
    if(modelStack.back() == 0)
    {
        modelStack.pop_back(); // remove the DL mark
        --decisionLevel;
    }
    
    indexOfNextLitToPropagate = modelStack.size();
}

void backtrack(int level)
{
    while(decisionLevel > level)
        undoOne();
}

void calcReason(int conflict, int lit, vector<int> &reason)
{
    Clause &c = clauses[conflict];
    
    for(int i = (lit == UNDEF ? 0 : 1); i < c.literals.size(); ++i)
        reason.push_back(-c.literals[i]);
}

void analyze(int conflict, Clause &learnt, int &btLevel)
{
    vector<bool> seen(numVars + 1, false);
    int counter = 0;
    int lit = UNDEF;
    vector<int> lit_reason;
    
    btLevel = 0;
    learnt.literals.push_back(0);
    
    do
    {
        lit_reason.clear();
        calcReason(conflict, lit, lit_reason);
        
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
                    learnt.literals.push_back(-q);
                    btLevel = max(btLevel, vq.level);
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
    
    learnt.literals[0] = -lit;
}
    
void learn(Clause &clause)
{
    clause.id = clauses.size();
    clauses.push_back(clause);
    watches[index(-clause.literals[0])].push_back(clause.id);
    
    if(clause.literals.size() > 1)
        watches[index(-clause.literals[1])].push_back(clause.id);
    
    variables[var(clause.literals[0])].reasonClause = clause.id;
    setLiteralToTrue(clause.literals[0]);
}

int getNextDecisionLiteral()
{
    for(uint i = 1; i <= numVars; ++i)
        if(model[variables[i].id] == UNDEF)
            return variables[i].id;
    
    return 0; // returns 0 when all literals are defined
}

void checkmodel()
{
    for(int i = 0; i < numClauses; ++i)
    {
        bool someTrue = false;
        
        for(int j = 0; not someTrue and j < clauses[i].literals.size(); ++j)
            someTrue = (currentValueInModel(clauses[i].literals[j]) == TRUE);
        
        if(not someTrue)
        {
            cout << "Error in model, clause is not satisfied:";
            
            for(int j = 0; j < clauses[i].literals.size(); ++j)
                cout << clauses[i].literals[j] << " ";
            
            cout << endl;
            exit(1);
        }
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
        if(clauses[i].literals.size() == 1)
        {
            int lit = clauses[i].literals[0];
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
    
    int conflictClause;
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
            
            Clause learntClause;
            analyze(conflictClause, learntClause, backtrackLevel);
            backtrack(backtrackLevel);
            learn(learntClause);
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

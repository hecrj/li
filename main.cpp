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

void printList(const list<uint> &v)
{
    for(list<uint>::const_iterator it = v.begin(); it != v.end(); ++it)
        cout << *it << " ";
    
    cout << endl;
}

struct Clause
{
    uint id;
    vector<int> literals;
    
    Clause()
    {
        id = 0;
    }
    
};

// App variables
uint numVars;
uint numClauses;
uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<uint> variables;
vector<uint> occurrences;
vector< list<uint> > watches;
vector<Clause> clauses;
vector<Clause> learntClauses;
vector<int> model;
vector<int> modelStack;

#define index(lit) lit + numVars
#define var(lit) lit < 0 ? -lit : lit

bool sortLiteralsByOccurrencesDesc(const int &a, const int &b)
{
    return occurrences[a] > occurrences[b];
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
    occurrences.resize(numVars + 1);
    watches.resize(numVars*2 + 1);
    
    for(int i = 1; i <= numVars; ++i)
    {
        variables[i] = i;
        occurrences[i] = 0;
    }
    
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
            
            occurrences[var(lit)]++;
        }
    }
    
    // Sort literals by number of occurrences
    sort(variables.begin(), variables.end(), sortLiteralsByOccurrencesDesc);
    
#ifdef DEBUG
    cout << "Literal info:" << endl;
    
    for(int i = 1; i <= numVars; ++i)
        literals[literalOrder[i]].print();
#endif
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

bool propagateGivesConflict(int literal, list<uint> &watchClauses)
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
            return true;
        
        setLiteralToTrue(clause.literals[0]); // Propagate
        ++it;
    }
    
    return false;
}

bool propagateGivesConflict()
{
    while(indexOfNextLitToPropagate < modelStack.size())
    {
        int lit = modelStack[indexOfNextLitToPropagate];
        
        if(propagateGivesConflict(-lit, watches[index(lit)]))
            return true;
        
        ++indexOfNextLitToPropagate;
    }
    
    return false;
}

void backtrack()
{
    uint i = modelStack.size() - 1;
    int lit = 0;
    
    while(modelStack[i] != 0) // 0 is the DL mark
    {
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit); // reverse last decision
}


// Heuristic for finding the next decision literal:

int getNextDecisionLiteral()
{
    for(uint i = 1; i <= numVars; ++i)
        if(model[variables[i]] == UNDEF)
            return variables[i];
    
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

    // DPLL algorithm
    while(true)
    {
        while(propagateGivesConflict())
        {
            if(decisionLevel == 0)
            {
                cout << "UNSATISFIABLE" << endl;
                return 10;
            }
            
            backtrack();
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

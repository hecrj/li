#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <list>

using namespace std;

#define UNDEF -1
#define TRUE   1
#define FALSE  0
#define DEBUG

#ifdef __APPLE__
    typedef unsigned int uint;
#endif

void printList(const list<uint> &v)
{
    for(list<uint>::const_iterator it = v.begin(); it != v.end(); ++it)
        cout << *it << " ";
    
    cout << endl;
}

struct Literal
{
    uint id;
    list<uint> normalClausesWatched;
    list<uint> negatedClausesWatched;
    int occurrences;
    
    void print() const
    {
        cout << "----------------" << endl;
        cout << "ID: " << id << endl;
        cout << "Occurrences: " << occurrences << endl;
        
        cout << "Normal clauses:" << endl;
        cout << "    ";
        printList(normalClausesWatched);
        
        cout << "Negated clauses:" << endl;
        cout << "    ";
        printList(negatedClausesWatched);
    }
};

struct Clause
{
    vector<int> literals;
    int watchedLiteral1;
    int watchedLiteral2;
    
    Clause()
    {
        watchedLiteral1 = 0;
        watchedLiteral2 = 0;
    }
    
    bool isWatched(int literal)
    {
        return (watchedLiteral1 == literal || watchedLiteral2 == literal);
    }
    
    void replaceWatch(int oldWatch, int newWatch)
    {
        if(watchedLiteral1 == oldWatch)
            watchedLiteral1 = newWatch;
        else
        {

            watchedLiteral2 = newWatch;
        }
    }
};

// App variables
uint numVars;
uint numClauses;
uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<Literal> literals;
vector<Clause> clauses;
vector<int> literalOrder;
vector<int> model;
vector<int> modelStack;

bool sortLiteralsByOccurrencesDesc(const int &a, const int &b)
{
    return literals[a].occurrences > literals[b].occurrences;
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
    literals = vector<Literal>(numVars + 1);
    literalOrder = vector<int>(numVars + 1);
    
    for(int i = 1; i <= numVars; ++i)
    {
        literals[i].id = i;
        literalOrder[i] = i;
    }
    
    // Read clauses
    for(uint i = 0; i < numClauses; ++i)
    {
        int lit;
        
        while(cin >> lit and lit != 0)
        {
            clauses[i].literals.push_back(lit);
            
            int lit_id = abs(lit);
            
            if(clauses[i].watchedLiteral1 == 0 || clauses[i].watchedLiteral2 == 0)
            {
                if(lit > 0)
                    literals[lit_id].normalClausesWatched.push_back(i);
                else
                    literals[lit_id].negatedClausesWatched.push_back(i);
                
                if(clauses[i].watchedLiteral1 == 0)
                    clauses[i].watchedLiteral1 = lit;
                else
                    clauses[i].watchedLiteral2 = lit;
            }
            
            literals[lit_id].occurrences += 1;
        }
    }
    
    // Sort literals by number of occurrences
    sort(literalOrder.begin(), literalOrder.end(), sortLiteralsByOccurrencesDesc);
    
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

bool propagateGivesConflict(int setLiteral, list<uint> &watchClauses)
{
    list<uint>::iterator it = watchClauses.begin();
    
    while(it != watchClauses.end())
    {
        // Reference variable, just to avoid long names
        Clause& clause = clauses[*it];
        vector<int> undefLits;
        
        int lastVal = UNDEF;
        
        for(uint k = 0; lastVal != TRUE and k < clause.literals.size(); ++k)
        {
            lastVal = currentValueInModel(clause.literals[k]);
            
            if(lastVal == UNDEF)
                undefLits.push_back(clause.literals[k]);
        }
        
        if(lastVal == TRUE)
        {
            ++it;
            continue;
        }
        
        if(undefLits.size() == 0)
            return true; // CONFLICT! Clause is false!
        
        if(undefLits.size() == 1)
        {
            setLiteralToTrue(undefLits[0]);
            ++it;
        }
        else
        {
            // Find new watch literal
            for(int i = 0; i < undefLits.size(); ++i)
            {
                if(!clause.isWatched(undefLits[i]))
                {
                    clause.replaceWatch(setLiteral, undefLits[i]);
                    
                    if(undefLits[i] > 0)
                        literals[undefLits[i]].normalClausesWatched.push_back(*it);
                    else
                        literals[-undefLits[i]].negatedClausesWatched.push_back(*it);
                    
                    break;
                }
            }
            
            it = watchClauses.erase(it);
        }
    }
    
    return false;
}

bool propagateGivesConflict()
{
    while(indexOfNextLitToPropagate < modelStack.size())
    {
        int lit = modelStack[indexOfNextLitToPropagate];
        int id = abs(lit);

        if(model[id] == TRUE)
        {
            // Using some immersion to avoid duplicated code!
            if(propagateGivesConflict(-id, literals[id].negatedClausesWatched))
                return true;
        }
        else if(propagateGivesConflict(id, literals[id].normalClausesWatched))
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
        if(model[literalOrder[i]] == UNDEF)
            return literalOrder[i];
    
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

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>

using namespace std;

#define UNDEF -1
#define TRUE   1
#define FALSE  0

void printVector(const vector<uint> &v)
{
    for(int i = 0; i < v.size(); ++i)
        cout << v[i] << " ";
    
    cout << endl;
}

struct Literal
{
    uint id;
    vector<uint> normalClauses;
    vector<uint> negatedClauses;
    vector<uint> satisfiesClauses;
    
    int getOccurrences() const
    {
        return normalClauses.size() + negatedClauses.size();
    }
    
    void print() const
    {
        cout << "----------------" << endl;
        cout << "ID: " << id << endl;
        cout << "Occurrences: " << getOccurrences() << endl;
        
        cout << "Normal clauses:" << endl;
        cout << "    ";
        printVector(normalClauses);
        
        cout << "Negated clauses:" << endl;
        cout << "    ";
        printVector(negatedClauses);
    }
};

// App variables
uint numVars;
uint numClauses;
uint indexOfNextLitToPropagate;
uint decisionLevel;
vector<Literal> literals;
vector<int> literalOrder;
vector< vector<int> > clauses;
vector<bool> clausesSatisfied;
vector<int> model;
vector<int> modelStack;

bool sortLiteralsByOccurrencesDesc(const int &a, const int &b)
{
    return literals[a].getOccurrences() > literals[b].getOccurrences();
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
    clausesSatisfied = vector<bool>(numClauses, false);
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
            clauses[i].push_back(lit);
            
            int lit_id = abs(lit);
            
            if(lit > 0)
                literals[lit_id].normalClauses.push_back(i);
            else
                literals[lit_id].negatedClauses.push_back(i);
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

void addSatisfiedClauses(int id, const vector<uint> &toSatisfy)
{
    for(int i = 0; i < toSatisfy.size(); ++i)
    {
        if(!clausesSatisfied[toSatisfy[i]])
        {
            literals[id].satisfiesClauses.push_back(toSatisfy[i]);
            clausesSatisfied[toSatisfy[i]] = true;
        }
    }
}

void removeSatisfiedClauses(int id)
{
    for(int i = 0; i < literals[id].satisfiesClauses.size(); ++i)
        clausesSatisfied[literals[id].satisfiesClauses[i]] = false;
    
    literals[id].satisfiesClauses.clear();
}

void setLiteralToTrue(int lit)
{
    modelStack.push_back(lit);
    
    if(lit > 0)
    {
        model[lit] = TRUE;
        addSatisfiedClauses(lit, literals[lit].normalClauses);
    }
    else
    {
        model[-lit] = FALSE;
        addSatisfiedClauses(-lit, literals[-lit].negatedClauses);
    }
}

bool propagateGivesConflict(const vector<uint> &propagationClauses)
{   
    for(uint i = 0; i < propagationClauses.size(); ++i)
    {
        int clause = propagationClauses[i];
        
        if(clausesSatisfied[clause])
            continue;
        
        bool someLitTrue = false;
        int numUndefs = 0;
        int lastLitUndef = 0;

        for(uint k = 0; not someLitTrue and k < clauses[clause].size(); ++k)
        {
            int val = currentValueInModel(clauses[clause][k]);

            if(val == TRUE)
                someLitTrue = true;
            else if(val == UNDEF)
            {
                ++numUndefs;
                lastLitUndef = clauses[clause][k];
            }
        }

        if(not someLitTrue and numUndefs == 0)
            return true; // conflict! all lits false

        if(not someLitTrue and numUndefs == 1)
            setLiteralToTrue(lastLitUndef);
    }
    
    return false;
}

bool propagateGivesConflict()
{
    while(indexOfNextLitToPropagate < modelStack.size())
    {
        int id = abs(modelStack[indexOfNextLitToPropagate]);
        int value = model[id];
        
        if(value == TRUE)
        {
            // Using some immersion to avoid duplicated code!
            if(propagateGivesConflict(literals[id].negatedClauses))
                return true;
        }
        else if(propagateGivesConflict(literals[id].normalClauses))
            return true;
        
        ++indexOfNextLitToPropagate;
    }
    
    return false;
}

void backtrack()
{
    uint i = modelStack.size() - 1;
    int lit = 0;
    int id;
    
    while(modelStack[i] != 0) // 0 is the DL mark
    {
        lit = modelStack[i];
        id = abs(lit);
        
        removeSatisfiedClauses(id);
        model[id] = UNDEF;
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
    for(uint i = 1; i <= numVars; ++i) // stupid heuristic:
        if(model[literalOrder[i]] == UNDEF)
            return literalOrder[i]; // returns first UNDEF var, positively
    
    return 0; // reurns 0 when all literals are defined
}

void checkmodel()
{
    for(int i = 0; i < numClauses; ++i)
    {
        bool someTrue = false;
        
        for(int j = 0; not someTrue and j < clauses[i].size(); ++j)
            someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
        
        if(not someTrue)
        {
            cout << "Error in model, clause is not satisfied:";
            
            for(int j = 0; j < clauses[i].size(); ++j)
                cout << clauses[i][j] << " ";
            
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
        if(clauses[i].size() == 1)
        {
            int lit = clauses[i][0];
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

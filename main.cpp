#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

enum gateTypes
{
    INV,
    AND,
    OR,
    NAND,
    NOR,
    NOT,
    XOR,
    XNOR,
    BUF,
    INPUT,
    OUTPUT
};

struct gateNode
{
    int ip1, ip2;
    int op;
    int gateType;
    int gateID;
    vector<int> signalList;
};

struct signalNode
{
    int signalID;
    vector<int> gateIDs;
    int op;
};

struct less_than_key_signalNode
{
    inline bool operator()(const signalNode &struct1, const signalNode &struct2)
    {
        return (struct1.signalID < struct2.signalID);
    }
};

bool split(string inputString, vector<string> &tokens, char delimiter)
{
    // Check if empty
    if (inputString.empty())
    {
        return false;
    }
    else
    {
        string token;
        istringstream tokenStream(inputString);

        // iterate through all words seperated by space and add to vector
        while (getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return true;
    }
}

int gateType(string inputString)
{
    vector<string> allowedGates{"INV", "AND", "OR", "NAND", "NOR", "NOT", "XOR", "XNOR", "BUF", "INPUT", "OUTPUT"};

    for (int i = 0; i < allowedGates.size(); ++i)
    {
        if (!allowedGates[i].compare(inputString))
        {
            return i;
        }
    }

    return -1;
}

void addToSignalList(int signal, int gateID, vector<signalNode> &signals)
{

    // search if signal exists
    // [OPTIMISABLE]
    for (int i = 0; i < signals.size(); ++i)
    {
        // if it exists, add gateID to corresponding signal
        if (signals[i].signalID == signal)
        {
            signals[i].gateIDs.push_back(gateID);
            return;
        }
    }

    // if it does not, create new signalNode and add gateID
    signalNode newSignal;
    newSignal.signalID = signal;
    newSignal.gateIDs.push_back(gateID);

    signals.push_back(newSignal);
}

void readInputFile(string inputFile, vector<string> &inputString)
{
    ifstream file(inputFile);
    // vector<string> tokens;

    while (file)
    {
        string line;
        getline(file, line);

        if (line.empty())
        {
            break;
        }
        inputString.push_back(line);
    }
    file.close();
}

void assignValues(string inputs, vector<signalNode> &signals, vector<int> inputSignals)
{
    // Iterate through inputSignals list and assign values according to inputs
    for (int i = 0; i < inputSignals.size(); ++i)
    {

        // Assigning value to signal
        for (int j = 0; j < signals.size(); ++j)
        {
            signals[inputSignals[i]].op = inputs[i] - '0';
        }
    }
}

void addToStack(vector<int> inputSignals, vector<int> &gateStack, vector<gateNode> gates, vector<int> &evaluatedGates)
{
    // Going through all gates
    for (int i = 0; i < gates.size(); ++i)
    {
        // bool inputsDefined = false;

        // Ensure gate has not been added to stack previously
        if (find(evaluatedGates.begin(), evaluatedGates.end(), i) != evaluatedGates.end())
        {
            continue;
        }

        // check if inputs are in inputSignals list

        switch (gates[i].gateType)
        {
        case INV:
        case BUF:
        case NOT:
            // Check for ip1
            if (find(inputSignals.begin(), inputSignals.end(), gates[i].ip1) != inputSignals.end())
            {
                gateStack.push_back(i);
                // adding to evaluated list to ensure that it is not added to stack again
                evaluatedGates.push_back(i);
            }
            break;
        default:
            // Chec for both ip1 and ip2
            if (std::find(inputSignals.begin(), inputSignals.end(), gates[i].ip1) != inputSignals.end())
            {
                if (std::find(inputSignals.begin(), inputSignals.end(), gates[i].ip2) != inputSignals.end())
                {
                    gateStack.push_back(i);
                    // adding to evaluated list to ensure it is not added to stack again
                    evaluatedGates.push_back(i);
                }
            }
        }
    }
}

void evaluateGate(gateNode gate, vector<signalNode> &signals)
{
    switch (gate.gateType)
    {
    case INV:
    case NOT:
        signals[gate.op].op = !signals[gate.ip1].op;
        break;
    case AND:
        signals[gate.op].op = signals[gate.ip1].op & signals[gate.ip2].op;
        break;
    case OR:
        signals[gate.op].op = signals[gate.ip1].op | signals[gate.ip2].op;
        break;
    case XOR:
        signals[gate.op].op = signals[gate.ip1].op ^ signals[gate.ip2].op;
        break;
    case XNOR:
        signals[gate.op].op = !(signals[gate.ip1].op ^ signals[gate.ip2].op);
        break;
    case NAND:
        signals[gate.op].op = !(signals[gate.ip1].op & signals[gate.ip2].op);
        break;
    case NOR:
        signals[gate.op].op = !(signals[gate.ip1].op | signals[gate.ip2].op);
        break;
    case BUF:
        signals[gate.op].op = signals[gate.ip1].op;
        break;
    }
}

void readNetlist(string inputFile, vector<gateNode> &gates, vector<signalNode> &signals, vector<int> &inputSignals, vector<int> &outputSignals)
{

    ifstream file(inputFile);
    vector<string> tokens;
    int GateID = 0;

    while (file)
    {
        tokens.clear();
        string line;
        int inputGate;

        gateNode gate;

        getline(file, line);

        if (line.empty())
        {
            break;
        }

        // Removing last character from string
        line.pop_back();

        // Splitting into tokens
        split(line, tokens, ' ');

        // Switching according to gate type
        inputGate = gateType(tokens[0]);

        gate.gateID = GateID;
        gate.gateType = inputGate;

        switch (inputGate)
        {
        case INV:
        case NOT:
        case BUF:
            gate.ip1 = stoi(tokens[1]) - 1;
            gate.op = stoi(tokens[2]) - 1;

            // Adding gates to signal list
            addToSignalList(gate.ip1, GateID, signals);
            addToSignalList(gate.op, GateID, signals);
            gates.push_back(gate);
            break;
        case INPUT:
            for (int i = 1; i < tokens.size(); i++)
            {
                if (!tokens[i].compare("-1\\") || !tokens[i].compare("-1\\"))
                    break;
                if (tokens[i].empty())
                    continue;
                inputSignals.push_back(stoi(tokens[i]) - 1);
            }

            break;
        case OUTPUT:
            for (int i = 1; i < tokens.size(); i++)
            {
                if (!tokens[i].compare("-1\\") || !tokens[i].compare("-1"))
                    break;
                if (tokens[i].empty())
                    continue;
                outputSignals.push_back(stoi(tokens[i]) - 1);
            }
            break;
        default:
            gate.ip1 = stoi(tokens[1]) - 1;
            gate.ip2 = stoi(tokens[2]) - 1;
            gate.op = stoi(tokens[3]) - 1;

            // Adding gates to signal list
            addToSignalList(gate.ip1, GateID, signals);
            addToSignalList(gate.ip2, GateID, signals);
            addToSignalList(gate.op, GateID, signals);
            gates.push_back(gate);
            break;
        }

        GateID++;
        // cout << line;
    }
    file.close();
}

void writeOutputFile(string fileName, vector<string> outputString)
{
    ofstream file(fileName);

    for (auto output : outputString)
    {
        file << output << "\n";
    }
    file.close();
}

int main(int argc, char *argv[])
{
    string netlistFile = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    vector<signalNode> signals;
    vector<gateNode> gates;
    vector<string> inputString;

    vector<int> gateStack, inputSignals, outputSignals;
    vector<int> definedSignals;

    vector<string> outputString;

    // Read Netlist
    // ifstream netlist(inputFile);

    // Reading Netlist
    readNetlist(netlistFile, gates, signals, inputSignals, outputSignals);

    sort(signals.begin(), signals.end(), less_than_key_signalNode());

    definedSignals = inputSignals;

    // Reading inputs
    readInputFile(inputFile, inputString);

    // iterating through all values of inputs
    for (int i = 0; i < inputString.size(); ++i)
    {
        definedSignals = inputSignals;
        vector<int> evaluatedGates;
        // Initialising input signals with inputs
        assignValues(inputString[i], signals, inputSignals);

        // Pushing gate to stack whose inputs are defined
        addToStack(definedSignals, gateStack, gates, evaluatedGates);

        // Evaluating the value of gates in stack
        while (!gateStack.empty())
        {
            int currentGate = gateStack.back();

            // Evaluating the current gate on top of stack
            evaluateGate(gates[currentGate], signals);

            // adding the output signals to the list of defined signals
            definedSignals.push_back(gates[currentGate].op);

            // Popping the Stack
            gateStack.pop_back();

            // Addding new gates whose inputs are now defined to stack
            addToStack(definedSignals, gateStack, gates, evaluatedGates);
        }

        // Saving output signal list to string
        string output;
        for (int signal : outputSignals)
        {
            output = output + to_string(signals[signal].op);
        }

        outputString.push_back(output);
    }

    //

    cout << "yeah boi";
    writeOutputFile(outputFile, outputString);
    // cout << outputString;
    // continue until stack empty
}

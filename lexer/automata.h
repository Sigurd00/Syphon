//
// Created by jskad on 28-02-2025.
//

#ifndef SYPHON_AUTOMATA_H
#define SYPHON_AUTOMATA_H


#include <set>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <iostream>
#include <queue>

class State;
class NFA;
class DFA;
static const char EPSILON = '\0'; // Epsilon transition symbol

struct NFAFragment {
    int startState;
    std::set<int> acceptStates;
    std::map<std::pair<int, char>, std::set<int>> transitions;

    explicit NFAFragment(int start) : startState(start) {}
};

class FiniteAutomaton {
protected:
    std::set<int> states;
    std::set<char> alphabet;
    int startState;
    std::set<int> acceptStates;

public:
    FiniteAutomaton() : startState(0) {}
    virtual ~FiniteAutomaton() = default;

    // Add a state to the automaton
    virtual void addState(int state, bool isAccept = false) {
        states.insert(state);
        if (isAccept) {
            acceptStates.insert(state);
        }
    }

    // Add a symbol to the alphabet
    virtual void addSymbol(char symbol) {
        alphabet.insert(symbol);
    }

    // Set the start state
    virtual void setStartState(int state) {
        if (states.find(state) == states.end()) {
            states.insert(state);
        }
        startState = state;
    }

    virtual void displayTransitionTable() const = 0;

    [[nodiscard]] std::set<int> getStates() const {
        return states;
    }
    [[nodiscard]] int getStartState() const {
        return startState;
    };

    [[nodiscard]] const std::set<int>& getAcceptState() const {
        return acceptStates;
    }

    [[nodiscard]] const std::set<char>& getAlphabet() const{
        return alphabet;
    }


};

class DFA : public FiniteAutomaton {
private:
    std::map<std::pair<int, char>, int> transitionTable;

public:
    DFA() : FiniteAutomaton() {}

    void addTransition(int fromState, char symbol, int toState) {
        states.insert(fromState);
        states.insert(toState);
        alphabet.insert(symbol);
        transitionTable[{fromState, symbol}] = toState;
    }

    [[nodiscard]] const std::map<std::pair<int, char>, int> getTransitionTable() const {
        return transitionTable;
    }

    void displayTransitionTable() const override {
        std::cout << "DFA Transition Table:" << std::endl;

        // Display header with alphabet symbols
        std::cout << "State\t";
        for (char c : alphabet) {
            std::cout << c << "\t";
        }
        std::cout << "Accept?" << std::endl;

        // Display each state and its transitions
        for (int state : states) {
            std::cout << state << (state == startState ? "(S)" : "") << "\t";

            for (char symbol : alphabet) {
                auto transition = std::make_pair(state, symbol);
                auto it = transitionTable.find(transition);
                if (it != transitionTable.end()) {
                    std::cout << it->second << "\t";
                } else {
                    std::cout << "-\t";
                }
            }

            std::cout << (acceptStates.find(state) != acceptStates.end() ? "Yes" : "No") << std::endl;
        }
    }
};

class NFA : public FiniteAutomaton {
private:
    std::map<std::pair<int, char>, std::set<int>> transitionTable;


public:
    NFA() : FiniteAutomaton() {
        // Add epsilon to the alphabet for NFAs
        alphabet.insert(EPSILON);
    }

    explicit NFA(const NFAFragment& fragment) : FiniteAutomaton() {
        alphabet.insert(EPSILON);

        // Copy transitions from fragment
        for (auto &transition : fragment.transitions) {
            auto &fromState = transition.first.first;
            auto &symbol = transition.first.second;
            for (auto &toState : transition.second) {
                addTransition(fromState, symbol, toState);
            }
        }

        // Set start and accept states
        startState = fragment.startState;
        acceptStates = fragment.acceptStates;
    }
    [[nodiscard]] const std::map<std::pair<int, char>, std::set<int>>& getTransitionTable() const {
        return transitionTable;
    }

    void addTransition(int fromState, char symbol, int toState) {
        states.insert(fromState);
        states.insert(toState);
        if (symbol != EPSILON) {
            alphabet.insert(symbol);
        }
        transitionTable[{fromState, symbol}].insert(toState);
    }

    std::set<int> epsilonClosure(int state) const {
        std::set<int> closure = {state};
        std::queue<int> queue;
        queue.push(state);

        while (!queue.empty()) {
            int currentState = queue.front();
            queue.pop();

            auto it = transitionTable.find({currentState, EPSILON});
            if (it != transitionTable.end()) {
                for (int nextState : it->second) {
                    if (closure.find(nextState) == closure.end()) {
                        closure.insert(nextState);
                        queue.push(nextState);
                    }
                }
            }
        }

        return closure;
    }

    std::set<int> epsilonClosure(const std::set<int>& states) const {
        std::set<int> closure;
        for (int state : states) {
            std::set<int> stateClosure = epsilonClosure(state);
            closure.insert(stateClosure.begin(), stateClosure.end());
        }
        return closure;
    }

    // Display the transition table
    void displayTransitionTable() const override {
        std::cout << "NFA Transition Table:" << std::endl;

        // Display header with alphabet symbols
        std::cout << "State\t";
        for (char c : alphabet) {
            if (c == EPSILON) {
                std::cout << "epsilon\t";
            } else {
                std::cout << c << "\t";
            }
        }
        std::cout << "Accept?" << std::endl;

        // Display each state and its transitions
        for (int state : states) {
            std::cout << state << (state == startState ? "(S)" : "") << "\t";

            for (char symbol : alphabet) {
                auto transition = std::make_pair(state, symbol);
                auto it = transitionTable.find(transition);
                if (it != transitionTable.end()) {
                    std::cout << "{";
                    bool first = true;
                    for (int nextState : it->second) {
                        if (!first) std::cout << ",";
                        std::cout << nextState;
                        first = false;
                    }
                    std::cout << "}\t";
                } else {
                    std::cout << "-\t";
                }
            }

            std::cout << (acceptStates.find(state) != acceptStates.end() ? "Yes" : "No") << std::endl;
        }
    }
};


#endif //SYPHON_AUTOMATA_H

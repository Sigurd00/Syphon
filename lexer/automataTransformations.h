//
// Created by jskad on 05-03-2025.
//

#ifndef SYPHON_AUTOMATATRANSFORMATIONS_H
#define SYPHON_AUTOMATATRANSFORMATIONS_H


#include <algorithm>
#include "automata.h"

class AutomataTransformations {
public:
    // This uses the subset construction algorithm to convert an NFA to a DFA
    static DFA nfa_to_dfa(const NFA& nfa) {
        DFA dfa;
        std::map<std::set<int>, int> stateMapping;  // Maps NFA state sets to DFA states
        std::queue<std::set<int>> stateQueue;

        if (nfa.getStates().empty()) {
            return dfa;
        }

        // Compute epsilon closure of start state
        std::set<int> startState = nfa.epsilonClosure(nfa.getStartState());

        // Assign the first DFA state
        int nextDFAState = 0;
        stateMapping[startState] = nextDFAState;
        dfa.setStartState(nextDFAState);
        dfa.addState(nextDFAState,
                // Check if the start state set contains any accept states
                     std::any_of(startState.begin(), startState.end(),
                                 [&nfa](int state) {
                                     return nfa.getAcceptState().find(state) != nfa.getAcceptState().end();
                                 })
        );
        stateQueue.push(startState);
        nextDFAState++;

        while (!stateQueue.empty()) {
            std::set<int> currentStateSet = stateQueue.front();
            stateQueue.pop();
            int currentDFAState = stateMapping[currentStateSet];

            // Iterate through non-epsilon alphabet symbols
            for (char symbol : nfa.getAlphabet()) {
                if (symbol == EPSILON) continue;

                // Compute next state set
                std::set<int> nextStateSet;
                for (int state : currentStateSet) {
                    auto transitions = nfa.getTransitionTable().find({state, symbol});
                    if (transitions != nfa.getTransitionTable().end()) {
                        for (int targetState : transitions->second) {
                            // Compute epsilon closure for each target state
                            std::set<int> closure = nfa.epsilonClosure(targetState);
                            nextStateSet.insert(closure.begin(), closure.end());
                        }
                    }
                }

                // If next state set is not empty
                if (!nextStateSet.empty()) {
                    // Check if this state set is already mapped
                    auto mappingIt = stateMapping.find(nextStateSet);
                    int nextDFAStateIndex;

                    if (mappingIt == stateMapping.end()) {
                        // Create a new DFA state
                        nextDFAStateIndex = nextDFAState;
                        stateMapping[nextStateSet] = nextDFAStateIndex;

                        // Check if the new state set contains any accept states
                        bool isAcceptState = std::any_of(nextStateSet.begin(), nextStateSet.end(),
                                                         [&nfa](int state) {
                                                             return nfa.getAcceptState().find(state) != nfa.getAcceptState().end();
                                                         });

                        dfa.addState(nextDFAStateIndex, isAcceptState);
                        stateQueue.push(nextStateSet);
                        nextDFAState++;
                    } else {
                        // Use existing mapped state
                        nextDFAStateIndex = mappingIt->second;
                    }

                    // Add transition to the DFA
                    dfa.addTransition(currentDFAState, symbol, nextDFAStateIndex);
                }
            }
        }

        return dfa;
    }

    // Minimize a DFA using Hopcroft's algorithm
    static DFA minimize_dfa(const DFA& originalDfa) {
        const auto& transitionTable = originalDfa.getTransitionTable();

        // Step 1: Partition states into accept and non-accept states
        std::vector<std::set<int>> partition;
        std::set<int> acceptStates, nonAcceptStates;

        // Separate states into accept and non-accept groups
        for (auto& state: originalDfa.getStates()) {
            if (originalDfa.getAcceptState().find(state) != originalDfa.getAcceptState().end()) {
                acceptStates.insert(state);
            } else {
                nonAcceptStates.insert(state);
            }
        }

        // Only add non-empty sets to the partition
        if (!acceptStates.empty()) partition.push_back(acceptStates);
        if (!nonAcceptStates.empty()) partition.push_back(nonAcceptStates);

        // Step 2: Iteratively refine the partition
        bool partitionChanged;
        do {
            partitionChanged = false;
            std::vector<std::set<int>> newPartition;

            for (const auto &group: partition) {
                // If group has only one state, keep it as is
                if (group.size() <= 1) {
                    newPartition.push_back(group);
                    continue;
                }

                // Try to split the current group
                std::vector<std::set<int>> splitGroups;
                splitGroups.push_back({*group.begin()});

                for (int state: group) {
                    bool foundGroup = false;
                    for (auto &potentialGroup: splitGroups) {
                        // Check if the current state is equivalent to the first state in this group
                        if (are_states_equivalent(originalDfa, *potentialGroup.begin(), state, partition)) {
                            potentialGroup.insert(state);
                            foundGroup = true;
                            break;
                        }
                    }

                    // If no equivalent group found, create a new group
                    if (!foundGroup) {
                        splitGroups.push_back({state});
                        partitionChanged = true;
                    }
                }

                // Add split groups to new partition
                newPartition.insert(
                        newPartition.end(),
                        splitGroups.begin(),
                        splitGroups.end()
                );
            }

            // Update partition
            partition = newPartition;
        } while (partitionChanged);

        // Step 3: Create the minimized DFA
        DFA minimizedDfa;
        std::map<int, int> stateMapping;

        // Assign new state numbers and map original states to minimized states
        int newStateIndex = 0;
        for (const auto &group: partition) {
            // Find a representative state for the group
            int representativeState = *group.begin();

            // Map all states in the group to the new state index
            for (int state: group) {
                stateMapping[state] = newStateIndex;
            }

            // Add the new state to the minimized DFA
            minimizedDfa.addState(newStateIndex,
                                  originalDfa.getAcceptState().find(representativeState) != originalDfa.getAcceptState().end()
            );

            // Set start state if the group contains the original start state
            if (group.find(originalDfa.getStartState()) != group.end()) {
                minimizedDfa.setStartState(newStateIndex);
            }

            newStateIndex++;
        }

        // Step 4: Add transitions to the minimized DFA
        for (const auto &group: partition) {
            int fromState = stateMapping[*group.begin()];

            // Use the first state in the group to check transitions
            int representativeState = *group.begin();

            // Check transitions for each symbol
            for (char symbol: originalDfa.getAlphabet()) {
                // Find the transition for the representative state
                auto transition = transitionTable.find({representativeState, symbol});
                if (transition != originalDfa.getTransitionTable().end()) {
                    // Map the destination state to its new minimized state
                    int originalDestState = transition->second;
                    int destState = stateMapping[originalDestState];

                    // Add the transition to the minimized DFA
                    minimizedDfa.addTransition(fromState, symbol, destState);
                }
            }
        }

        return minimizedDfa;
    }

    private:
    // Check if two states are equivalent in the current partition
    static bool are_states_equivalent(const DFA& dfa, int state1, int state2,
                                      const std::vector<std::set<int>>& partition) {
        const auto& transitionTable = dfa.getTransitionTable();

        // States must be in the same accept/non-accept group
        bool state1Accept = dfa.getAcceptState().find(state1) != dfa.getAcceptState().end();
        bool state2Accept = dfa.getAcceptState().find(state2) != dfa.getAcceptState().end();
        if (state1Accept != state2Accept) return false;

        // Check transitions for each symbol
        for (char symbol : dfa.getAlphabet()) {
            // Find where each state transitions for this symbol
            auto transition1 = transitionTable.find({state1, symbol});
            auto transition2 = transitionTable.find({state2, symbol});

            // If transitions are different
            if (transition1 == transitionTable.end() ||
                transition2 == transitionTable.end()) {
                // If both lack a transition, they're equivalent
                if (transition1 == transitionTable.end() &&
                    transition2 == transitionTable.end())
                    continue;
                return false;
            }

            // Find the groups of the destination states
            int dest1 = transition1->second;
            int dest2 = transition2->second;

            // Check if destination states are in the same partition group
            bool dest1InSameGroup = false;
            bool dest2InSameGroup = false;

            for (const auto& group : partition) {
                bool dest1Found = group.find(dest1) != group.end();
                bool dest2Found = group.find(dest2) != group.end();

                // If destinations are in different groups, states are not equivalent
                if (dest1Found != dest2Found) {
                    return false;
                }
            }
        }
        return true;
    }
};


#endif //SYPHON_AUTOMATATRANSFORMATIONS_H

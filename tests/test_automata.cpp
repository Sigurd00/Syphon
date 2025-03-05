#include <gtest/gtest.h>
#include "automata.h"
#include "regexToNFA.h"
#include "automataTransformations.h"

// DFA Tests
TEST(DFATest, AddTransition) {
    DFA dfa;
    dfa.addState(0, false);
    dfa.addState(1, true);
    dfa.setStartState(0);
    dfa.addTransition(0, 'a', 1);

    // Checking transition
    std::stringstream output;
    std::streambuf* oldCout = std::cout.rdbuf(output.rdbuf());
    dfa.displayTransitionTable();
    std::cout.rdbuf(oldCout);

    std::string result = output.str();
    EXPECT_NE(result.find("0(S)\t1\tNo"), std::string::npos);
    EXPECT_NE(result.find("1\t-\tYes"), std::string::npos);
}

// NFA Tests
TEST(NFATest, EpsilonClosure) {
    NFA nfa;
    nfa.addState(0, false);
    nfa.addState(1, false);
    nfa.addState(2, true);
    nfa.setStartState(0);
    nfa.addTransition(0, EPSILON, 1);
    nfa.addTransition(1, EPSILON, 2);

    std::set<int> closure = nfa.epsilonClosure(0);
    EXPECT_EQ(closure, std::set<int>({0, 1, 2}));
}

// RegexToNFA Tests

// AutomataTransformations tests
class AutomataTransformationsTest : public ::testing::Test {
protected:
    static NFA createSimpleNFA() {
        NFA nfa;
        nfa.setStartState(0);
        nfa.addState(0);
        nfa.addState(1, true);
        nfa.addState(2);

        // Epsilon transition from 0 to 1
        nfa.addTransition(0, EPSILON, 1);
        // Transition from 0 to 2 on 'a'
        nfa.addTransition(0, 'a', 2);
        // Transition from 2 to 1 on 'b'
        nfa.addTransition(2, 'b', 1);

        return nfa;
    }

    static DFA createMinimizationDFA() {
        DFA dfa;
        dfa.setStartState(0);

        // Add states
        dfa.addState(0);
        dfa.addState(1);
        dfa.addState(2);
        dfa.addState(3, true);  // Accept state
        dfa.addState(4, true);  // Another accept state

        // Add alphabet
        dfa.addSymbol('0');
        dfa.addSymbol('1');

        // Add transitions (simulating a DFA with equivalent states)
        dfa.addTransition(0, '0', 1);
        dfa.addTransition(0, '1', 2);
        dfa.addTransition(1, '0', 3);
        dfa.addTransition(1, '1', 4);
        dfa.addTransition(2, '0', 3);
        dfa.addTransition(2, '1', 4);

        return dfa;
    }
};

// Subset Construction Tests
TEST_F(AutomataTransformationsTest, SubsetConstructionBasicNFA) {
    NFA nfa = createSimpleNFA();

    // Convert NFA to DFA
    DFA dfa = AutomataTransformations::nfa_to_dfa(nfa);

    // Basic validations
    EXPECT_GT(dfa.getStates().size(), 0);
    EXPECT_GT(dfa.getAcceptState().size(), 0);
}

TEST_F(AutomataTransformationsTest, SubsetConstructionComplexNFA) {
    NFA nfa;
    nfa.setStartState(0);
    nfa.addState(0);
    nfa.addState(1);
    nfa.addState(2, true);
    nfa.addState(3);

    // Epsilon transitions
    nfa.addTransition(0, EPSILON, 1);
    nfa.addTransition(1, EPSILON, 3);

    // Symbol transitions
    nfa.addTransition(0, 'a', 2);
    nfa.addTransition(1, 'b', 2);

    // Convert NFA to DFA
    DFA dfa = AutomataTransformations::nfa_to_dfa(nfa);

    // Validations
    EXPECT_GT(dfa.getStates().size(), 0);
    EXPECT_GT(dfa.getAcceptState().size(), 0);
}

// DFA Minimization Tests
TEST_F(AutomataTransformationsTest, DFAMinimizationBasic) {
    DFA originalDfa = createMinimizationDFA();

    // Minimize the DFA
    DFA minimizedDfa = AutomataTransformations::minimize_dfa(originalDfa);

    // Validations
    EXPECT_LE(minimizedDfa.getStates().size(), originalDfa.getStates().size());
    EXPECT_FALSE(minimizedDfa.getAcceptState().empty());
}

TEST_F(AutomataTransformationsTest, DFAMinimizationEvenOnes) {
    DFA originalDfa;
    originalDfa.setStartState(0);

    // Add states
    originalDfa.addState(0, true);  // Start state is accept state
    originalDfa.addState(1);

    // Add alphabet
    originalDfa.addSymbol('0');
    originalDfa.addSymbol('1');

    // Add transitions
    originalDfa.addTransition(0, '0', 0);
    originalDfa.addTransition(0, '1', 1);
    originalDfa.addTransition(1, '0', 1);
    originalDfa.addTransition(1, '1', 0);

    // Minimize the DFA
    DFA minimizedDfa = AutomataTransformations::minimize_dfa(originalDfa);

    // Validations
    EXPECT_LE(minimizedDfa.getStates().size(), originalDfa.getStates().size());
    EXPECT_FALSE(minimizedDfa.getAcceptState().empty());
}

// Edge Case Tests
TEST_F(AutomataTransformationsTest, SubsetConstructionEmptyNFA) {
    NFA nfa;

    // Convert empty NFA to DFA
    DFA dfa = AutomataTransformations::nfa_to_dfa(nfa);

    // Validations
    EXPECT_EQ(dfa.getStates().size(), 0);
    EXPECT_EQ(dfa.getAcceptState().size(), 0);
}

TEST_F(AutomataTransformationsTest, DFAMinimizationSingleState) {
    DFA originalDfa;
    originalDfa.setStartState(0);
    originalDfa.addState(0, true);
    originalDfa.addSymbol('0');
    originalDfa.addSymbol('1');
    originalDfa.addTransition(0, '0', 0);
    originalDfa.addTransition(0, '1', 0);

    // Minimize the DFA
    DFA minimizedDfa = AutomataTransformations::minimize_dfa(originalDfa);

    // Validations
    EXPECT_EQ(minimizedDfa.getStates().size(), 1);
    EXPECT_TRUE(minimizedDfa.getAcceptState().find(0) != minimizedDfa.getAcceptState().end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

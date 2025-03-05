#include <iostream>
#include "lexer/regexToNFA.h"
#include "lexer/automataTransformations.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto nfa = RegexToNFA::fromRegex("a|b");
    nfa.displayTransitionTable();
    auto dfa = AutomataTransformations::nfa_to_dfa(nfa);
    dfa.displayTransitionTable();

    auto minimizable_dba = DFA();
    minimizable_dba.addState(0, true);
    minimizable_dba.addTransition(0, 'a', 1);
    minimizable_dba.addTransition(0, 'b', 2);
    minimizable_dba.addTransition(1, 'a', 2);
    minimizable_dba.addTransition(1, 'b', 2);
    minimizable_dba.addTransition(2, 'a', 3);
    minimizable_dba.addTransition(2, 'b', 3);
    minimizable_dba.addTransition(3, 'a', 4);
    minimizable_dba.addTransition(3, 'b', 0);
    minimizable_dba.addTransition(4, 'a', 3);
    minimizable_dba.addTransition(4, 'b', 0);
    auto minimized_dfa = AutomataTransformations::minimize_dfa(minimizable_dba);

    minimized_dfa.displayTransitionTable();
}
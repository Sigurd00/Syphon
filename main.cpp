#include <iostream>
#include "lexer/regexToNFA.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    auto nfa = RegexToNFA::fromRegex("a|b");
    nfa.displayTransitionTable();
}
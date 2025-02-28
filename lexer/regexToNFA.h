//
// Created by jskad on 28-02-2025.
//

#ifndef SYPHON_REGEXTONFA_H
#define SYPHON_REGEXTONFA_H


#include <stack>
#include "automata.h"

class RegexToNFA {
public:
    static NFA fromRegex(const std::string& regex) {
        std::string postfix = infixToPostfix(regex);
        return buildNFAFromPostfix(postfix);
    }

private:
    static bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static int precedence(char op) {
        if (op == '*') return 3;
        if (op == '.') return 2;
        if (op == '|') return 1;
        return 0;
    }

    static std::string infixToPostfix(const std::string& regex) {
        std::string postfix;
        std::stack<char> operators;

        for (size_t i = 0; i < regex.length(); ++i) {
            char c = regex[i];

            if (isAlpha(c)) {
                postfix += c;
                if (i + 1 < regex.length() && (isAlpha(regex[i + 1]) || regex[i + 1] == '(')) {
                    postfix += '.';
                }
            } else if (c == '(') {
                operators.push(c);
            } else if (c == ')') {
                while (!operators.empty() && operators.top() != '(') {
                    postfix += operators.top();
                    operators.pop();
                }
                operators.pop();
                if (i + 1 < regex.length() && (isAlpha(regex[i + 1]) || regex[i + 1] == '(')) {
                    postfix += '.';
                }
            } else { // Operators * . |
                while (!operators.empty() && precedence(operators.top()) >= precedence(c)) {
                    postfix += operators.top();
                    operators.pop();
                }
                operators.push(c);
            }
        }

        while (!operators.empty()) {
            postfix += operators.top();
            operators.pop();
        }

        return postfix;
    }

    static NFA buildNFAFromPostfix(const std::string& postfix) {
        std::stack<NFAFragment> stack;
        int stateCounter = 0;

        for (char symbol : postfix) {
            if (isAlpha(symbol)) { // Single character transition
                int start = stateCounter++;
                int end = stateCounter++;
                NFAFragment fragment(start);
                fragment.acceptStates.insert(end);
                fragment.transitions[{start, symbol}].insert(end);
                stack.push(fragment);
            } else if (symbol == '|') { // Union
                if (stack.size() < 2) throw std::runtime_error("Invalid regex: insufficient operands for '|'");
                NFAFragment right = stack.top(); stack.pop();
                NFAFragment left = stack.top(); stack.pop();
                int start = stateCounter++;
                int end = stateCounter++;

                NFAFragment fragment(start);
                fragment.transitions[{start, EPSILON}].insert(left.startState);
                fragment.transitions[{start, EPSILON}].insert(right.startState);

                for (int state : left.acceptStates) {
                    fragment.transitions[{state, EPSILON}].insert(end);
                }
                for (int state : right.acceptStates) {
                    fragment.transitions[{state, EPSILON}].insert(end);
                }

                fragment.acceptStates.insert(end);
                fragment.transitions.insert(left.transitions.begin(), left.transitions.end());
                fragment.transitions.insert(right.transitions.begin(), right.transitions.end());
                stack.push(fragment);
            } else if (symbol == '.') { // Concatenation
                if (stack.size() < 2) throw std::runtime_error("Invalid regex: insufficient operands for '.'");
                NFAFragment right = stack.top(); stack.pop();
                NFAFragment left = stack.top(); stack.pop();

                for (int state : left.acceptStates) {
                    left.transitions[{state, EPSILON}].insert(right.startState);
                }

                left.acceptStates = right.acceptStates;
                left.transitions.insert(right.transitions.begin(), right.transitions.end());
                stack.push(left);
            } else if (symbol == '*') { // Kleene star
                if (stack.empty()) throw std::runtime_error("Invalid regex: insufficient operand for '*'");
                NFAFragment fragment = stack.top(); stack.pop();
                int start = stateCounter++;
                int end = stateCounter++;

                fragment.transitions[{start, EPSILON}].insert(fragment.startState);
                fragment.transitions[{start, EPSILON}].insert(end);

                for (int state : fragment.acceptStates) {
                    fragment.transitions[{state, EPSILON}].insert(fragment.startState);
                    fragment.transitions[{state, EPSILON}].insert(end);
                }

                fragment.startState = start;
                fragment.acceptStates = {end};
                stack.push(fragment);
            }
        }

        if (stack.size() != 1) throw std::runtime_error("Invalid regex: malformed expression");
        return NFA(stack.top());
    }
};

#endif //SYPHON_REGEXTONFA_H

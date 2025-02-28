cmake_minimum_required(VERSION 3.30)
project(Syphon)

set(CMAKE_CXX_STANDARD 20)

add_executable(Syphon main.cpp
        lexer/automata.cpp
        lexer/automata.h
        lexer/regexToNFA.cpp
        lexer/regexToNFA.h)

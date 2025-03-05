cmake_minimum_required(VERSION 3.30)
project(Syphon)

set(CMAKE_CXX_STANDARD 20)

# Enable testing
enable_testing()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Include lexer directory for headers
include_directories(${PROJECT_SOURCE_DIR}/lexer)

# Add main executable
add_executable(Syphon main.cpp lexer/automata.cpp lexer/regexToNFA.cpp
        lexer/automataTransformations.cpp
        lexer/automataTransformations.h)

# Add test executable
add_executable(test_automata tests/test_automata.cpp lexer/automata.cpp lexer/regexToNFA.cpp)
target_link_libraries(test_automata gtest gtest_main)

# Register tests
add_test(NAME AutomataTests COMMAND test_automata)

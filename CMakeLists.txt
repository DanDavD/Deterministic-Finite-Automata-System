cmake_minimum_required(VERSION 3.16)
project(SistemaAutomatasDFA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(dfa_system
    src/main.cpp
    src/Automata.cpp
    src/ValidadorAutomata.cpp
    src/UnionAutomatas.cpp
)

target_include_directories(dfa_system PRIVATE include)

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

# logica del automata, compartida por la app de consola y por el servidor
add_library(dfa_core
    src/Automata.cpp
    src/ValidadorAutomata.cpp
    src/UnionAutomatas.cpp
)
target_include_directories(dfa_core PUBLIC include)

# app de consola
add_executable(dfa_system src/main.cpp)
target_link_libraries(dfa_system PRIVATE dfa_core)

# servidor http que expone la misma logica al front
find_package(Threads REQUIRED)

add_executable(dfa_server
    server/main_server.cpp
    server/ApiHandlers.cpp
)
target_include_directories(dfa_server PRIVATE server)
target_include_directories(dfa_server SYSTEM PRIVATE server/vendor)
target_link_libraries(dfa_server PRIVATE dfa_core Threads::Threads)
target_compile_definitions(dfa_server PRIVATE
    RUTA_ESTATICOS="${CMAKE_CURRENT_SOURCE_DIR}/server/static"
)

if(WIN32)
    # httplib pide apis de windows 10 en adelante
    target_compile_definitions(dfa_server PRIVATE
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
        NTDDI_VERSION=0x0A000000
    )
    target_link_libraries(dfa_server PRIVATE ws2_32)
endif()

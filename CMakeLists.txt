cmake_minimum_required(VERSION 3.20)
project(UltimateTicTacToeAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to an optimised build: the engines are search-heavy.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Both engines evaluate candidate moves in parallel with std::async.
find_package(Threads REQUIRED)

if(MSVC)
  set(UTTT_WARNINGS /W4 /utf-8)
else()
  set(UTTT_WARNINGS -Wall -Wextra)
endif()

# The team's engine: rules (Morpion, Plateau) and the two AIs (Minimax, MonteCarlo).
# The headers include "main.h" (src/main.h) for the GameMove type and the TAILLE constant.
add_library(uttt_engine STATIC
  src/Morpion.cpp
  src/Plateau.cpp
  src/Minimax.cpp
  src/MonteCarlo.cpp
)
target_include_directories(uttt_engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(uttt_engine PUBLIC Threads::Threads)
target_compile_options(uttt_engine PRIVATE ${UTTT_WARNINGS})

# Standalone match runner (added for the public version).
add_executable(uttt_match standalone/uttt_match.cpp)
target_link_libraries(uttt_match PRIVATE uttt_engine)
target_compile_options(uttt_match PRIVATE ${UTTT_WARNINGS})

# Unit tests (added for the public version).
add_executable(uttt_tests tests/test_main.cpp)
target_link_libraries(uttt_tests PRIVATE uttt_engine)
target_compile_options(uttt_tests PRIVATE ${UTTT_WARNINGS})

enable_testing()
add_test(NAME unit_tests COMMAND uttt_tests)
add_test(NAME match_smoke
  COMMAND uttt_match --a montecarlo --b minimax --games 4 --sims 500 --depth 3 --seed 1 --opening 2)

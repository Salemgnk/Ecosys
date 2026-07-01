cmake_minimum_required(VERSION 3.16)
project(Ecosys LANGUAGES CXX)

# --- Standard C++ ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Génère build/compile_commands.json pour que l'éditeur (clangd) connaisse
# les include paths et arrête d'afficher de fausses erreurs.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Bibliothèque moteur ---------------------------------------------------
# On récolte automatiquement tous les .cpp sous engine/.
# (GLOB par confort d'apprentissage ; en projet « pro » on liste plutôt les
#  fichiers à la main. CONFIGURE_DEPENDS fait re-scanner CMake au build.)
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp)

add_library(ecosys_engine ${ENGINE_SOURCES})

# Chaque sous-dossier du moteur devient un chemin d'include, pour que tes
# `#include "Species.hpp"` (sans préfixe) fonctionnent depuis n'importe où.
target_include_directories(ecosys_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/core
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/ecology
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/observer)

target_compile_options(ecosys_engine PRIVATE -Wall -Wextra)

# --- Exécutable de démo (apps/cli) -----------------------------------------
# S'active tout seul dès que apps/cli/main.cpp existera.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/apps/cli/main.cpp)
    add_executable(ecosys apps/cli/main.cpp)
    target_link_libraries(ecosys PRIVATE ecosys_engine)
    target_compile_options(ecosys PRIVATE -Wall -Wextra)
endif()

# --- Exécutable d'observation (mode miroir /proc) ---------------------------
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/apps/cli/observe.cpp)
    add_executable(ecosys-observe apps/cli/observe.cpp)
    target_link_libraries(ecosys-observe PRIVATE ecosys_engine)
    target_compile_options(ecosys-observe PRIVATE -Wall -Wextra)
endif()

# --- Tests ------------------------------------------------------------------
# S'active tout seul dès que tests/engine_tests.cpp existera.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/engine_tests.cpp)
    enable_testing()
    add_executable(engine_tests tests/engine_tests.cpp)
    target_link_libraries(engine_tests PRIVATE ecosys_engine)
    target_compile_options(engine_tests PRIVATE -Wall -Wextra)
    add_test(NAME engine_tests COMMAND engine_tests)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/observer_tests.cpp)
    enable_testing()
    add_executable(observer_tests tests/observer_tests.cpp)
    target_link_libraries(observer_tests PRIVATE ecosys_engine)
    target_compile_options(observer_tests PRIVATE -Wall -Wextra)
    add_test(NAME observer_tests COMMAND observer_tests)
endif()

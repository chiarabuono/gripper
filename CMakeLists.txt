cmake_minimum_required(VERSION 3.18)
project(grasp_sim LANGUAGES CXX)

# --- Standard e tipo di build -------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Utile per l'autocompletamento in VS Code / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- MuJoCo -------------------------------------------------------------
# Il release standalone non include mujocoConfig.cmake, quindi cerchiamo
# a mano header e libreria. Indica la cartella di MuJoCo (quella che
# contiene include/ e lib/) in uno di questi modi:
#   cmake -B build -DMUJOCO_ROOT=$HOME/libs/mujoco-X.Y.Z
#   oppure: export MUJOCO_ROOT=$HOME/libs/mujoco-X.Y.Z
set(MUJOCO_ROOT "$ENV{MUJOCO_ROOT}" CACHE PATH "Cartella di MuJoCo (con include/ e lib/)")

find_path(MUJOCO_INCLUDE_DIR mujoco/mujoco.h
  HINTS "${MUJOCO_ROOT}/include"
  REQUIRED)
find_library(MUJOCO_LIBRARY mujoco
  HINTS "${MUJOCO_ROOT}/lib"
  REQUIRED)

# Target con lo stesso nome che userebbe find_package
add_library(mujoco::mujoco UNKNOWN IMPORTED)
set_target_properties(mujoco::mujoco PROPERTIES
  IMPORTED_LOCATION "${MUJOCO_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${MUJOCO_INCLUDE_DIR}")

# --- Eseguibile principale ----------------------------------------------
# Aggiungi qui i nuovi .cpp man mano che li scrivi.
add_executable(gripper_demo
  src/main.cpp
)

target_include_directories(gripper_demo PRIVATE include)
target_link_libraries(gripper_demo PRIVATE mujoco::mujoco)

target_compile_options(gripper_demo PRIVATE -Wall -Wextra -Wpedantic)

# --- Test (GoogleTest) --------------------------------------------------
# Si attivano da soli quando crei dei file in tests/*.cpp
option(BUILD_TESTS "Compila i test" ON)

if(BUILD_TESTS)
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
  if(TEST_SOURCES)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(unit_tests ${TEST_SOURCES})
    target_include_directories(unit_tests PRIVATE include)
    target_link_libraries(unit_tests PRIVATE GTest::gtest_main mujoco::mujoco)
    gtest_discover_tests(unit_tests)
  endif()
endif()
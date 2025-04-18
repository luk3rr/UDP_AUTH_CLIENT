# Filename: CMakeLists.txt
# Created on: November  5, 2023
# Author: Lucas Araújo <araujolucas@dcc.ufmg.br>

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# Set default parameters
PROJECT(udp_client_auth)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags for different build types
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(EXECUTABLE_NAME "program_debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -pedantic")
ELSE()
    SET(EXECUTABLE_NAME "program")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic")
ENDIF()

MESSAGE(STATUS "C++ Compiler Flags:${CMAKE_CXX_FLAGS}")

SET(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
#SET(UNIT_TEST_DIR ${CMAKE_SOURCE_DIR}/test/unit)
SET(INC_DIR ${CMAKE_SOURCE_DIR}/include)
#SET(BENCHMARK_DIR ${CMAKE_SOURCE_DIR}/test/benchmark)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/libs)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release)

# Manager submodules
## Submodules X (example)
# SET(MODULE_X_DIR ${CMAKE_SOURCE_DIR}/submodules/module_x)
# AUX_SOURCE_DIRECTORY(${MODULE_X_DIR}/src MODULE_X_PROGRAM)
# INCLUDE_DIRECTORIES(${MODULE_X_DIR}/include)
# ADD_LIBRARY(ModuleX ${MODULE_X_PROGRAM})

# Get all files in the folders SRC_DIR and UNIT_TEST_DIR
AUX_SOURCE_DIRECTORY(${SRC_DIR} PROGRAM)
#AUX_SOURCE_DIRECTORY(${UNIT_TEST_DIR} UNIT_TESTS)

INCLUDE_DIRECTORIES(${INC_DIR})
INCLUDE_DIRECTORIES(${INC_DIR}/lib)

# Define the program lib
ADD_LIBRARY(udp_client ${PROGRAM})

# Make executables
ADD_EXECUTABLE(program ${PROGRAM})
#ADD_EXECUTABLE(unit_test ${UNIT_TESTS})

# Link libs
TARGET_LINK_LIBRARIES(udp_client)

cmake_minimum_required(VERSION 3.22)

project(CodeWeekend-1-public)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(".")

file(GLOB SOURCES
        Objects/Basic/*.cpp
        Objects/Complex/*.cpp
        Objects/Tools/*.cpp
        )

add_executable(solution main.cpp ${SOURCES})

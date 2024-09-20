cmake_minimum_required(VERSION 3.22)

project(CodeWeekend-1-public)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(OBJECT_SOURCES OBJECT
        Objects/randomizer.cpp
        Objects/timer.cpp
        Objects/test_data.cpp
        Objects/answer.cpp
        )

add_executable(solution main.cpp $<TARGET_OBJECTS:OBJECT_SOURCES>)

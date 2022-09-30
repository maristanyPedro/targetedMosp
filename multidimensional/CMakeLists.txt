cmake_minimum_required(VERSION 3.10)
set(EXEC_NAME "labelSettingMosp.o")
project(${EXEC_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

include_directories(datastructures/includes)
include_directories(graph/includes)
include_directories(preprocessing/includes)
include_directories(search/includes)

add_compile_options(-std=c++14 -g -Wall -Wextra -pedantic)

add_executable(${EXEC_NAME}
        graph/src/graph.cpp
        preprocessing/src/Preprocessor.cpp
        search/src/SolutionsList.cpp
        src/main.cpp)
    target_link_libraries(${EXEC_NAME})

cmake_minimum_required(VERSION 3.10)
project(targetedMdaVsMda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

include_directories(datastructures/includes)
include_directories(graph/includes)
include_directories(preprocessing/includes)
include_directories(search/includes)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-std=c++14 -O3 -g -Winline -Wall -Wextra -pedantic)
endif()


add_executable(targetedMdaVsMda
        datastructures/includes/BinaryHeapMosp.h
        datastructures/includes/Label.h
        graph/includes/graph.h
        graph/src/graph.cpp
        preprocessing/includes/LexDijkstra.h
        preprocessing/includes/Preprocessor.h
        preprocessing/src/Preprocessor.cpp
        search/includes/Solution.h
        search/includes/SolutionsList.h
        search/includes/MDA-Star.h
        search/includes/MDA.h
        src/main.cpp
        )
target_link_libraries(targetedMdaVsMda)

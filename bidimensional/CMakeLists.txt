cmake_minimum_required(VERSION 3.10)
project(targetedBda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenMP REQUIRED)

include_directories(datastructures/includes)
include_directories(graph/includes)
include_directories(preprocessing/includes)
include_directories(search/includes)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
	#add_compile_options(-std=c++14 -O3 -Winline -Wall -Wextra -pedantic -fopenmp -pthread)
    add_compile_options(-std=c++14 -Winline -Wall -Wextra -pedantic -fopenmp -pthread)
endif()


add_executable(targetedBda
        datastructures/includes/BinaryHeap.h
        datastructures/includes/BucketsQueue.h
        datastructures/includes/Label.h
        datastructures/includes/Label_Buckets.h
        graph/includes/graph.h
        graph/src/graph.cpp
        preprocessing/includes/LexDijkstra.h
        preprocessing/includes/Preprocessor.h
        preprocessing/src/Preprocessor.cpp
        search/includes/Solution.h
        search/src/BOA.cpp
        search/includes/BDA.h
        src/main.cpp
        datastructures/src/NodeInfoContainer.cpp)
target_link_libraries(targetedBda PRIVATE OpenMP::OpenMP_CXX)

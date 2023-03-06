# targetedMosp
Implementations of the (Targeted) Multiobjective Dijkstra Algorithm developed jointly at the Zuse Institute Berlin in Germany and the Universidad de La Laguna in Spain. This repository is complementary material to the manuscript https://arxiv.org/abs/2110.10978. In case you have any questions, contact Pedro Maristany de las Casas -- maristany [a t] zib.de

# Referencing/Citing the Code
To cite this code, please always use the permanent bibliographic information specified here: https://doi.org/10.5281/zenodo.7702018

## Disclaimer 
We have tested the code and this manual on UNIX based systems. Some basic support can be provided if users encounter problems on these platforms. Users on Windows based systems will have to figure out how to compile and run these algorithms on their own.

## Generate executables
This is a CMake project, the source code is written in C++ and uses some C++14 features. If not install, start installing the newest CMake version available for your system. Once CMake is installed, use the terminal to navigate either to the 'multidimensional' or the 'bidimensional' folder in the project, depending on your needs. The instructions are the same in both cases; from now on, we assume that you navigate into the 'multidimensional' folder. Now create a new folder called 'build' (name can be chosen arbitrarily) and navigate into the folder. This step is not mandatory but it helps to keep the root folders clean; CMake users consider it a best practice. From the new 'build' folder, call `cmake .. -DCMAKE_BUILD_TYPE=Release` to generate the Makefile for the project. In case this step succeeds, you should now have a 'Makefile' in the 'build' folder. Call `make`. This will generate an executable called 'labelSettingMosp.o' in the 'build' folder.

## Running an example
From the 'build' folder described in the last section, call `./labelSettingMosp.o ../instances/NY.gr 0 4`. Here, the first argument is the name of the executable, the second argument is a (relative) path to a graph with 3-dimensional arc costs, the third argument is the id of the source node, and the fourth argument is the id of the target node. If everything works well, you should see an output like this:
```
T-MDA;NY.gr;0;10100;2297421;2297421;6055;1.4252;25.76;3089
NAMOA;NY.gr;0;10100;2297407;2297407;6055;5.0103;5.91;3089
NAMOA_LAZY;NY.gr;0;10100;2559950;2870672;6055;1.4661;7.80;160658
```
Each line's entries are: algo-name, graph-name, sourceId, targetId, number of extracions, number of iterations, number of efficient paths at target, time to solve, memory,max number of paths in prio. queue.

## Graph files
The first line of a graph file has to have the following self explanatory format (compare to the file 'aStarExample.gr' in the folder 'exampleGraphs')
```
p sp number_of_nodes number_of_arcs
```
The line should be followed by number_of_arcs many lines, each of them specifying an arc information as follows for the 3-dimensional case
```
a tailId headId c1 c2 c3
```

The ids of the nodes are assumed to be numbered consecutively from 0 to number_of_nodes-1. In case you use a 2-dimensional instance for the multi-dimensional code (currently only enabled for 3 dimensions), the program internally creates a third cost dimension with costs 1 for every arc in the graph.

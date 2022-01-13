# targetedMosp
Implementations of the (Targeted) Multiobjective Dijkstra Algorithm. Includes variants for biobjective and multiobjective scenarios.

## Generate executables
This is a CMake project. If not install, start installing the newest CMake version available for your system. Once CMake is installed, use the terminal to navigate either to the 'multidimensional' or the 'bidimensional' folder in the project, depending on your needs. The instructions are the same in both cases; from now on, we assume that you navigate into the 'multidimensional' folder. Now create a new folder called 'build' (name can be chosen arbitrarily) and navigate in to the folder. This step is not mandatory but it helps to keep the root folders clean; CMake users consider it a best practice. From the new 'build' folder, call `cmake .. -DCMAKE_BUILD_TYPE=Release` to generate the Makefile for the project. In case this step succeeds, you should now have a 'Makefile' in the 'build' folder. Call `make`. This will generate an executable called 'targetedMdaVsMda' in the 'build' folder.

## Running an example
From the 'build' folder described in the last section, call `./targetedMdaVsMda ../exampleGraphs/aStarExample.gr 0 4`. Here, the first argument is the name of the executable, the second argument is a (relative) path to a graph with 3-dimensional arc costs, the third argument is the id of the source node, and the fourth argument is the id of the target node. If everything works well, you should see an output like this:
```
T-MDA;aStarExample.gr;0;4;5;5;2;0.0000
MDA;aStarExample.gr;0;4;5;5;2;0.0000
```
Each line's entries are: algo-name, graph-name, sourceId, targetId, number of extracions, number of iterations, number of efficient paths at target, time to solve.

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

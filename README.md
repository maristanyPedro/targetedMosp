# targetedMosp
Implementations of the (Targeted) Multiobjective Dijkstra Algorithm. Includes variants for biobjective and multiobjective scenarios.

## Generate executables
This is a CMake project. If not install, start installing the newest CMake version available for your system. Once CMake is installed, use the terminal to navigate either to the 'multidimensional' or the 'bidimensional' folder in the project, depending on your needs. The instructions are the same in both cases; from now on, we assume that you navigate into the 'multidimensional' folder. Now create a new folder called 'build' (name can be chosen arbitrarily) and navigate in to the folder. This step is not mandatory but it helps to keep the root folders clean; CMake users consider it a best practice. From the new 'build' folder, call `cmake .. -DCMAKE_BUILD_TYPE=Release` to generate the Makefile for the project. In case this step succeeds, you should now have a 'Makefile' in the 'build' folder. Call `make`. This will generate an executable called 'targetedMdaVsMda' in the 'build' folder.

## Running an example
From the 'build' folder described in the last section, call `./targetedMdaVsMda ../exampleGraphs/aStarExample.gr 0 4`. Here, the first argument is the name of the executable, the second argument is a (relative) path to a graph with 3-dimensional arc costs, the third argument is the id of the source node, and the fourth argument is the id of the target node.



To users that have some prior experience with CMake: it is regarded as best practice to now create a 'build' folder and continue creating the Makefile and the executable from there. If you do so, remember to adapt the relative path to files in the remainder of the instructions.

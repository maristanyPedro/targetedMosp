    #include <chrono>  // for high_resolution_clock

    #include <iostream>
    #include <memory>
    #include <sstream>
    #include <cinttypes>

    #include "../datastructures/includes/NodeInfoContainer.h"
    #include "../datastructures/includes/NodeInfo.h"

    #include "../preprocessing/includes/Preprocessor.h"
    #include "../search/includes/MDA-Star.h"
    #include "../search/includes/NAMOA.h"
    #include "../search/includes/NAMOA_lazy.h"

    using namespace std;

    int main(int argc, char *argv[]) {
        if (argc != 4) {
            printf("The program is meant to receive three arguments: file-directory, id of the source node, and id of the target node.\n");
            exit(1);
        }

        //printNumLimits();

        Node sourceId = INVALID_NODE;
        Node targetId = INVALID_NODE;
        stringstream(argv[2]) >> sourceId;
        stringstream(argv[3]) >> targetId;

        unique_ptr<Graph> G_ptr = setupGraph(argv[1], sourceId, targetId);
        Graph& G = *G_ptr;
        const string graphName{split(argv[1], '/').back()};

        if (G.nodesCount < sourceId || G.nodesCount < targetId) {
            throw;
        }

        Preprocessor preprocessor(G);
        NodeInfoContainer<NodeInfo> prepInfo(G); //For the search!
        preprocessor.run(prepInfo);
        std::vector<CostArray> potential(G.nodesCount);
        for (Node n = 0; n < G.nodesCount; ++n) {
            for (Arc& a : G.outgoingArcs(n)) {
                const auto& pi = prepInfo.getInfo(a.n).potential;
                addInPlace(a.c, pi);
            }
            potential[n] = prepInfo.getInfo(n).potential;
        }

        TargetedMDA bda{G, potential};
        Solution sol_bda_forward(graphName, sourceId, targetId);
        bda.run(sol_bda_forward);
        sol_bda_forward.print("T-MDA");

        {
            Solution sol(graphName, sourceId, targetId);
            NAMOA namoa{G, potential};
            namoa.run(sol);
            sol.print("NAMOA");
        }


        {
            Solution sol(graphName, sourceId, targetId);
            NAMOA_LAZY namoa{G, potential};
            namoa.run(sol);
            sol.print("NAMOA_LAZY");
        }

        //assert(namoa_solutions == mda_solutions);
    }

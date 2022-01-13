    #include <chrono>  // for high_resolution_clock

    #include <iostream>
    #include <memory>
    #include <sstream>
    #include <cinttypes>

    #include "../datastructures/includes/NodeInfoContainer.h"

    #include "../preprocessing/includes/Preprocessor.h"
    #include "../search/includes/MDA-Star.h"
    #include "../search/includes/MDA.h"

    #include "valgrind/callgrind.h"

    //#define PARALLEL_PREPROCESSOR

    using namespace std;

    void printNumLimits() {
        cout << "Size_t: " << numeric_limits<size_t>::max() << endl;
        cout << "uint32: " << numeric_limits<uint32_t>::max() << endl;
        cout << "uint16: " << numeric_limits<uint16_t>::max() << endl;
        cout << "uint8: " << unsigned(numeric_limits<uint8_t>::max()) << endl;
        cout << "long: " << numeric_limits<long>::max() << endl;
        cout << "Size_t: " << numeric_limits<size_t>::max() << endl;
    }

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
        const Graph& G = *G_ptr;
        const string graphName{split(argv[1], '/').back()};

        if (G.nodesCount < sourceId || G.nodesCount < targetId) {
            throw;
        }
    //
    //    CALLGRIND_START_INSTRUMENTATION;
    //

        {
            NodeInfoContainer expander_bda(G); //For the search!

            Preprocessor preprocessor_bda(G);
            preprocessor_bda.run(expander_bda);
            auto start_bda = std::chrono::high_resolution_clock::now();
            Solution sol_bda_forward(graphName, sourceId, targetId);
            TargetedMDA bda{G, expander_bda, preprocessor_bda};
            bda.run(sol_bda_forward);

            //
            //
            auto end_bda = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration_bda = end_bda - start_bda;
            sol_bda_forward.duration = duration_bda.count();
            sol_bda_forward.print("MDA-STAR");
        }

    //    CALLGRIND_STOP_INSTRUMENTATION;
    //    CALLGRIND_DUMP_STATS;

        NodeInfoContainer expander_mda(G); //For the search!

        Preprocessor preprocessor_mda(G);
        preprocessor_mda.run(expander_mda);
        auto start_mda = std::chrono::high_resolution_clock::now();
        Solution sol_mda(graphName, sourceId, targetId);
        MDA mda{G, expander_mda, preprocessor_mda};
        mda.run(sol_mda);

        auto end_mda = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_mda = end_mda - start_mda;
        sol_mda.duration = duration_mda.count();
        sol_mda.print("MDA");
    }

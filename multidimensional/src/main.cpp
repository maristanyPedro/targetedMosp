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
            NodeInfoContainer nic_t_mda(G);

            Preprocessor preprocessor_t_mda(G);
            preprocessor_t_mda.run(nic_t_mda);
            auto start_t_mda = std::chrono::high_resolution_clock::now();
            Solution sol_t_mda(graphName, sourceId, targetId);
            TargetedMDA t_mda{G, nic_t_mda, preprocessor_t_mda};
            t_mda.run(sol_t_mda);

            //
            //
            auto end_t_mda = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration_t_mda = end_t_mda - start_t_mda;
            sol_t_mda.duration = duration_t_mda.count();
            sol_t_mda.print("T-MDA");
        }

    //    CALLGRIND_STOP_INSTRUMENTATION;
    //    CALLGRIND_DUMP_STATS;

        NodeInfoContainer nic_mda(G); //For the search!

        Preprocessor preprocessor_mda(G);
        preprocessor_mda.run(nic_mda);
        auto start_mda = std::chrono::high_resolution_clock::now();
        Solution sol_mda(graphName, sourceId, targetId);
        MDA mda{G, nic_mda, preprocessor_mda};
        mda.run(sol_mda);

        auto end_mda = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_mda = end_mda - start_mda;
        sol_mda.duration = duration_mda.count();
        sol_mda.print("MDA");
    }

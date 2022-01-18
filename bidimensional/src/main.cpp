#include <cassert>
#include <chrono>  // for high_resolution_clock

#include <iostream>
#include <memory>
#include <sstream>
#include <cinttypes>
#include <omp.h>

#include "../datastructures/includes/NodeInfoContainer.h"

#include "../graph/includes/graph.h"
#include "../preprocessing/includes/Preprocessor.h"
#include "../search/includes/BDA.h"
#include "../search/includes/Solution.h"

//#define PARALLEL_PREPROCESSOR

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("The program is meant to receive three arguments: file-directory, id of the source node, and id of the target node.\n");
        exit(1);
    }

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

    NodeInfoContainer s_t_expander_bda(G, true);
    NodeInfoContainer t_s_expander_bda(G, false);

    Preprocessor preprocessor_bda(G);
    preprocessor_bda.run(s_t_expander_bda, t_s_expander_bda);

    auto start_bda = std::chrono::high_resolution_clock::now();
#define BUCKETS_QUEUE
#ifdef BUCKETS_QUEUE
    BDA<LabelBuckets, Buckets_BDA> bda_forward{false, G, s_t_expander_bda, t_s_expander_bda, preprocessor_bda};
    BDA<LabelBuckets, Buckets_BDA> bda_backward{true, G, t_s_expander_bda, s_t_expander_bda, preprocessor_bda};
#else
    BDA<Label, BinaryHeap> bda_forward{false, G, s_t_expander_bda, t_s_expander_bda, preprocessor_bda};
    BDA<Label, BinaryHeap> bda_backward{true, G, t_s_expander_bda, s_t_expander_bda, preprocessor_bda};
#endif

    Solution sol_bda_forward{graphName, G.source, G.target};
    Solution sol_bda_backward{graphName, G.target, G.source};
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            bda_forward.run(sol_bda_forward);
        }
        #pragma omp section
        {
            bda_backward.run(sol_bda_backward );
        }
    }
    auto end_bda = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_bda = end_bda - start_bda;
    Solution sol_bda_parallel{graphName, G.source, G.target};
    sol_bda_parallel.duration = duration_bda.count();
    sol_bda_parallel.solutionsCt = sol_bda_forward.solutionsCt + sol_bda_backward.solutionsCt;
    sol_bda_parallel.iterations = sol_bda_forward.iterations + sol_bda_backward.iterations;
    sol_bda_parallel.extractions = sol_bda_forward.extractions + sol_bda_backward.extractions;

#ifdef BUCKETS_QUEUE
    sol_bda_parallel.print("BBDA-bucket");
#else
    sol_bda_parallel.print("BDA-heap");
#endif
}

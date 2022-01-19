#include <iostream>
#include <omp.h>

#include "../includes/Preprocessor.h"
#include "../includes/LexDijkstra.h"

Preprocessor::Preprocessor(const Graph &G) :
    G{G} {}

void Preprocessor::run (NodeInfoContainer& s_t_infos, NodeInfoContainer& t_s_infos) {

    //Since false = 0, this will actually use the first cost component for optimizations.
    constexpr bool useFirstCostComponent = false;
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            //first cost component forward
            LexDijkstra dijkstra1{this->G, s_t_infos, t_s_infos};
            dijkstra1.runNew(useFirstCostComponent, dominanceBound[0], dominanceBound[1], false);
        }
        #pragma omp section
        {
            //second cost component backward
            LexDijkstra dijkstra2{this->G, t_s_infos, s_t_infos};
            dijkstra2.runNew(!useFirstCostComponent, dominanceBound[1], dominanceBound[0], false);
        }
    }
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            //first cost component backward
            LexDijkstra dijkstra3{this->G, t_s_infos, s_t_infos};
            dijkstra3.runNew(useFirstCostComponent, dominanceBound[0], dominanceBound[1], true);
        }
        #pragma omp section
        {
            //second cost component forward
            LexDijkstra dijkstra4{this->G, s_t_infos, t_s_infos};
            dijkstra4.runNew(!useFirstCostComponent, dominanceBound[1], dominanceBound[0], true);
        }
    }
}

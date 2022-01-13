#include <algorithm>
#include <iostream>
#include <omp.h>
#include <numeric>

#include "../includes/Preprocessor.h"
#include "../includes/LexDijkstra.h"

Preprocessor::Preprocessor(const Graph &G) :
    G{G} {}

inline CostArray max(const CostArray& c1, const CostArray& c2, const DimensionsVector& dimOrdering) {
    CostArray result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[dimOrdering[i]] = std::max(c1[dimOrdering[i]], c2[i]);
    }
    return result;
}

void Preprocessor::run (NodeInfoContainer& nic) {

    DimensionsVector dimOrdering;
    //fill dimOrderings vector from 0 to dimOrdering.size()-1.
    std::iota(std::begin(dimOrdering), std::end(dimOrdering), 0);
    Info<bool> processedMainDimensions;
    std::fill(processedMainDimensions.begin(), processedMainDimensions.end(), false);

    do {
        //Avoid multiple lex. searches with the same first optimization criterion.
        if (!processedMainDimensions[dimOrdering[0]]) {
            LexDijkstra dijkstra{this->G, nic};
            const CostArray result = dijkstra.run(dimOrdering);
            this->targetUb = max(this->targetUb, result, dimOrdering);
            processedMainDimensions[dimOrdering[0]] = true;
        }
    } while (std::next_permutation(dimOrdering.begin(), dimOrdering.end()));
}

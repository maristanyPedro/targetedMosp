#include <iostream>


#include "../includes/Preprocessor.h"

Preprocessor::Preprocessor(const Graph &G) :
    G{G} {}

//template <typename LabelType>
//void Preprocessor::run (NodeInfoContainer& nic) {
//
//    DimensionsVector dimOrdering;
//    //fill dimOrderings vector from 0 to dimOrdering.size()-1.
//    std::iota(std::begin(dimOrdering), std::end(dimOrdering), 0);
//    Info<bool> processedMainDimensions;
//    std::fill(processedMainDimensions.begin(), processedMainDimensions.end(), false);
//
//    do {
//        //Avoid multiple lex. searches with the same first optimization criterion.
//        if (!processedMainDimensions[dimOrdering[0]]) {
//            LexDijkstra dijkstra{this->G, nic};
//            const CostArray result = dijkstra.run(dimOrdering);
//            this->targetUb = max(this->targetUb, result, dimOrdering);
//            processedMainDimensions[dimOrdering[0]] = true;
//        }
//    } while (std::next_permutation(dimOrdering.begin(), dimOrdering.end()));
//}

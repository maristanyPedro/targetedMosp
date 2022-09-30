#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_
#include <numeric>

#include "typedefs.h"
#include "../includes/LexDijkstra.h"

class Graph;

/**
 * Run lexicographic searches according to a Set of Preprocessing Orders. These searches give lower bounds and
 * dominance bounds on the costs of efficient v-t-paths for any node v in the graph. These bounds can be used to ignore
 * irrelevant subpaths during the search. More details can be read in https://arxiv.org/abs/2110.10978.
 */
class Preprocessor{
public:
    Preprocessor() = delete;
    explicit Preprocessor(const Graph &G);

    /**
     * Runs two lexicographic Dijkstra queries and store the resultin values in the member-vectors of this class.
     *
     * @param getArcOrientation Returns a pair of nodes representing the end-nodes of an arc. The second node in this
     * pair will be used to expand labels along the arc. I.e., ,if pair.second is an arc's source node, the preprocessor
     * runs on the reversed subgraph of G.
     * @param getNeighborhood If the preprocessor is meant to uniDirSearch on G as oriented originally, this function is supposed
     * to return a node's forward star. If the preprocessor is meant to uniDirSearch on reversed G, the function returns a node's
     * backward star.
     */
    template <typename LabelType>
    void run (NodeInfoContainer<LabelType>& nic) {

        DimensionsVector dimOrdering;
        //fill dimOrderings vector from 0 to dimOrdering.size()-1.
        std::iota(std::begin(dimOrdering), std::end(dimOrdering), 0);
        Info<bool> processedMainDimensions;
        std::fill(processedMainDimensions.begin(), processedMainDimensions.end(), false);

        do {
            //Avoid multiple lex. searches with the same first optimization criterion.
            if (!processedMainDimensions[dimOrdering[0]]) {
                LexDijkstra<LabelType> dijkstra{this->G, nic};
                const CostArray result = dijkstra.run(dimOrdering);
                this->targetUb = max(this->targetUb, result, dimOrdering);
                processedMainDimensions[dimOrdering[0]] = true;
            }
        } while (std::next_permutation(dimOrdering.begin(), dimOrdering.end()));
    }

    const CostArray& getUpperBounds() const;
private:
    const Graph& G;
    CostArray targetUb{0, 0, 0};
};

inline const CostArray& Preprocessor::getUpperBounds() const {
    return this->targetUb;
}


#endif
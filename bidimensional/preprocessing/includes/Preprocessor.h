#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_

#include "typedefs.h"

class Graph;
class NodeInfoContainer;

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
    void run(NodeInfoContainer& s_t_infos, NodeInfoContainer& t_s_infos);
    Info<CostType>& getUpperBounds();
private:
    const Graph& G;
    Info<CostType> dominanceBound{MAX_COST, MAX_COST};
};

inline Info<CostType>& Preprocessor::getUpperBounds() {
    return this->dominanceBound;
}


#endif
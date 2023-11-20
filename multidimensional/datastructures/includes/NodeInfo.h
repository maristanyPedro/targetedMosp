#ifndef NODE_INFO_H
#define NODE_INFO_H

#include <vector>

/**
 * Bundles all relevant preprocessing related to a node in the graph.
 */
struct NodeInfo {
    //A dxd matrix storing the results of the d preprocessing lexicographic searches.
    Info<CostArray> preprocessingResult;
    //Computed during preprocessing. This is a lowerbound from the current node to the target node w.r.t. every cost dimension.
    CostArray potential{generate(MAX_COST)};
    //Current node for which 'this' stored information.
    Node n{INVALID_NODE};
    Info<uint32_t> priority; ///< for heap operations in preprocessing!
    //Used in preprocessing to flag if the node has been reached by the lex searches. The ith entry of the vector specifies
    //whether the node has been reached by a lex. search in which the ith cost component was the main optimization component.
    Info<bool> reached;


    NodeInfo() {
        for (size_t i = 0; i < DIM; ++i) {
            for (size_t j = 0; j < DIM; ++j) {
                preprocessingResult[i][j] = MAX_COST;
            }
        }
        std::fill(reached.begin(), reached.end(), false);
    }

    inline void updatePreprocessingInfo(const DimensionsVector& dimOrdering, CostArray c) {
        Dimension mainOptIndex = dimOrdering[0];
        CostArray& result = preprocessingResult[mainOptIndex];
        result = c;
    }

    inline uint32_t get_priority(Dimension index) const{
        return priority[index];
    }

    inline void set_priority(Dimension index, uint32_t prio) {
        priority[index] = prio;
    }
};

#endif //NODE_INFO_H

#ifndef LEX_DIJKSTRA_H_
#define LEX_DIJKSTRA_H_

#include <cassert>
#include <vector>

#include "../../graph/includes/graph.h"
#include "../../datastructures/includes/NodeInfoContainer.h"
#include "../../datastructures/includes/NodeInfo.h"

/**
 * Classical MIN binary heap implementation including a decreaseKey operation. The elements in the heap are supposed to be
 * allocated/to 'live' somewhere else. This implementation only stores POINTERS TO THE ELEMENTS in the heap. These pointers
 * are stored in a vector. The size of the vector grows as needed; it is originally one. The binary tree is ordered according
 * to the DimensionsVector passed to the constructor.
 */
class BinaryHeapPreprocessing {
public:
    BinaryHeapPreprocessing(size_t size, const DimensionsVector &dimOrdering) :
            elementsContainer(0),
            comparator(dimOrdering),
            lastElementIndex(0),
            mainOptIndex{dimOrdering[0]} {
        this->elementsContainer.resize(size);
    }

    void decreaseKey(NodeInfo *val) {
        assert(val->get_priority(mainOptIndex) < lastElementIndex);
        heapifyUp(val->get_priority(mainOptIndex));
    }

    void push(NodeInfo *val) {
        if (lastElementIndex + 1 > elementsContainer.size()) {
            elementsContainer.resize(elementsContainer.size() * 2);
        }
        size_t priority = lastElementIndex;
        elementsContainer[priority] = val;

        val->set_priority(mainOptIndex, priority);
        lastElementIndex++;
        heapifyUp(priority);
    }

    inline NodeInfo *pop() {
        assert(this->size() != 0);

        NodeInfo *ans = elementsContainer[0];
        lastElementIndex--;

        if (lastElementIndex > 0) {
            elementsContainer[0] = elementsContainer[lastElementIndex];
            elementsContainer[0]->set_priority(mainOptIndex, 0);
            heapifyDown(0);
        }
        return ans;
    }

    inline bool contains(NodeInfo *n) {
        size_t priority = n->get_priority(mainOptIndex);
        if (priority < lastElementIndex && n == elementsContainer[priority]) {
            return true;
        }
        return false;
    }

    inline size_t size() const {
        return lastElementIndex;
    }

private:
    std::vector<NodeInfo *> elementsContainer;
    const PotentialComparison comparator;
    size_t lastElementIndex;
    const Dimension mainOptIndex;

    void heapifyUp(size_t index) {
        assert(index < lastElementIndex);
        while (index > 0) {
            size_t parent = (index - 1) >> 1; //Shift right dividing by 2

            if (comparator(elementsContainer[index], elementsContainer[parent])) {
                swap(parent, index);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapifyDown(size_t index) {
        size_t first_leaf_index = lastElementIndex >> 1;
        while (index < first_leaf_index) {
            size_t child1 = (index << 1) + 1;
            size_t child2 = (index << 1) + 2;
            size_t swapCandidate = child1;
            if ((child2 < lastElementIndex) && comparator(elementsContainer[child2], elementsContainer[child1])) {
                swapCandidate = child2;
            }

            if (comparator(elementsContainer[swapCandidate], elementsContainer[index])) {
                swap(index, swapCandidate);
                index = swapCandidate;
            } else {
                break;
            }
        }
    }

    inline void swap(size_t id1, size_t id2) {
        NodeInfo *tempElement = elementsContainer[id1];
        elementsContainer[id1] = elementsContainer[id2];
        elementsContainer[id2] = tempElement;
        elementsContainer[id1]->set_priority(mainOptIndex, id1);
        elementsContainer[id2]->set_priority(mainOptIndex, id2);
    }
};

/**
 * Lexicographic Dijkstra algorithm. The graph is passed to the constructor and also contains the ids for the source
 * and target nodes. The run function becomes the permutation of the arc costs w.r.t. which the paths are ordered.
 * Hence, in a three dimensional scenario, if the permutation is (1,0,2), the algorithm will find a cost minimal s-t-path
 * w.r.t. the 1st arc cost dimension, using the 0th and 2nd cost dimensions as tie breakers. Since the binary heap used
 * in the search is ordered according to the permutation, the found s-t-path is guaranteed to be lex. minimal w.r.t. the
 * (1,0,2) permutation.
 */
class LexDijkstra {
public:

    LexDijkstra() = delete;

    LexDijkstra(const Graph &G, NodeInfoContainer &tracker) :
            G{G},
            tracker{tracker} {}

    CostArray run(const DimensionsVector &dimOrdering) {
        //printf("Running lex Dijkstra (%u, %u, %u)\n", dimOrdering[0], dimOrdering[1], dimOrdering[2]);
        const Dimension firstCostComponent{dimOrdering[0]};
        auto sumCosts =
                [&dimOrdering](const CostArray &s_t_costs, const CostArray &arcCosts) ->
                        CostArray {
                    return CostArray{s_t_costs[0] + arcCosts[dimOrdering[0]], s_t_costs[1] + arcCosts[dimOrdering[1]],
                                     s_t_costs[2] + arcCosts[dimOrdering[2]]};
                };
        BinaryHeapPreprocessing heapNew{1, dimOrdering};
        NodeInfo &startNode = this->tracker.getInfo(G.target);
        startNode.preprocessingResult[dimOrdering[0]] = {0, 0, 0};
        NodeInfo &targetNode = this->tracker.getInfo(G.source);
        CostArray targetCosts{MAX_COST, MAX_COST, MAX_COST};

        heapNew.push(&startNode);
        while (heapNew.size()) {
            NodeInfo *minElement = heapNew.pop();
            minElement->reached[firstCostComponent] = true;
            minElement->potential[firstCostComponent] = minElement->preprocessingResult[firstCostComponent][0];
            const CostArray &s_t_costs{minElement->preprocessingResult[firstCostComponent]};
            if (minElement->n == targetNode.n) {
                targetCosts = s_t_costs;
            }
            const Neighborhood &outgoingArcs{G.incomingArcs(minElement->n)};
            for (const Arc &a: outgoingArcs) {
                const Node successorNode = a.n;
                NodeInfo &successorInfo{this->tracker.getInfo(successorNode)};
                assert(successorInfo.n == successorNode);
                if (successorInfo.reached[firstCostComponent]) {
                    continue;
                }
                const CostArray expandedCosts = sumCosts(s_t_costs, a.c);
                if (heapNew.contains(&successorInfo)) {
                    const CostArray &currentSuccessorCosts{successorInfo.preprocessingResult[firstCostComponent]};
                    if (lexSmaller(expandedCosts, currentSuccessorCosts)) {
                        successorInfo.updatePreprocessingInfo(dimOrdering, expandedCosts);
                        heapNew.decreaseKey(&successorInfo);
                    }
                } else {
                    successorInfo.updatePreprocessingInfo(dimOrdering, expandedCosts);
                    heapNew.push(&successorInfo);
                }
            }
        }
        return targetCosts;
    }

private:
    const Graph &G;
    NodeInfoContainer &tracker;
};

#endif
#ifndef LEX_DIJKSTRA_H_
#define LEX_DIJKSTRA_H_

#include <cassert>
#include <set>
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
    BinaryHeapPreprocessing(size_t size, bool mainOptIndex) :
            elementsContainer(0),
            comparator(),
            lastElementIndex(0),
            mainOptIndex{mainOptIndex} {
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
    const bool mainOptIndex;

    void heapifyUp(size_t index) {
        assert(index < lastElementIndex);
        while (index > 0) {
            size_t parent = (index - 1) >> 1; //Shift right dividing by 2

            if (comparator(elementsContainer[index], elementsContainer[parent], this->mainOptIndex)) {
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
            if ((child2 < lastElementIndex) && comparator(elementsContainer[child2], elementsContainer[child1], this->mainOptIndex)) {
                swapCandidate = child2;
            }

            if (comparator(elementsContainer[swapCandidate], elementsContainer[index], this->mainOptIndex)) {
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

class LexDijkstra {
public:

    LexDijkstra() = delete;
    LexDijkstra(const Graph& G, NodeInfoContainer& forwardInfos, NodeInfoContainer& backwardInfos):
            G{G},
            fwdInfo{forwardInfos},
            bckwdInfo{backwardInfos} {};

    void runNew(bool firstCostComponent, CostType& targetUb_1, CostType& targetUb_2, bool tunePotential) {
        //Preprocessing searches always run in the opposite direction of the direction of the actual search!
        BinaryHeapPreprocessing heap{1, firstCostComponent};
        NodeInfo& startNode = this->fwdInfo.getStartNode();
        NodeInfo& targetNode = this->fwdInfo.getTargetNode();
        auto getOutgoingArcs = this->fwdInfo.forward ?
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.outgoingArcs(n);} :
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.incomingArcs(n);};
        auto getIncomingArcs = this->fwdInfo.forward ?
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.incomingArcs(n);} :
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.outgoingArcs(n);};

        startNode.minCosts[firstCostComponent] = 0;
        startNode.potential[firstCostComponent] = 0;
        startNode.dominanceBound[firstCostComponent] = 0;

        heap.push(&startNode);
        while (heap.size()) {
            NodeInfo* minElement = heap.pop();
            if (minElement->n == targetNode.n) {
                targetUb_2 = minElement->dominanceBound[firstCostComponent];
            }
            if (minElement->minCosts[firstCostComponent] > targetUb_1) {
                break;
            }
            minElement->reached[firstCostComponent] = true;
            const Neighborhood& outgoingArcs{getOutgoingArcs(this->G, minElement->n)};
            for (const Arc& a : outgoingArcs) {
                const Node successorNode = a.n;
                CostType costExtension_1 = minElement->potential[firstCostComponent] + a.c[firstCostComponent];
                CostType costExtension_2 = minElement->dominanceBound[firstCostComponent] + a.c[!firstCostComponent];
                NodeInfo& successorInfo{this->fwdInfo.getInfo(successorNode)};
                assert(successorInfo.n == successorNode);
                if (successorInfo.reached[firstCostComponent]) {
                    continue;
                }
                const Arc& reversedArc = getIncomingArcs(this->G, successorNode)[a.revArcIndex];
                if (heap.contains(&successorInfo)) {
                    if (costExtension_1 < successorInfo.potential[firstCostComponent]) {
                        successorInfo.updatePreprocessingInfo(firstCostComponent, costExtension_1, costExtension_2, &reversedArc);
                        heap.decreaseKey(&successorInfo);
                    }
                    else if (costExtension_1 == successorInfo.potential[firstCostComponent] && costExtension_2 < successorInfo.dominanceBound[firstCostComponent]) {
                        successorInfo.updatePreprocessingInfo(firstCostComponent, costExtension_1, costExtension_2, &reversedArc);
                    }
                    continue;
                }
                CostType lbToTarget = 0;
                if (tunePotential) {
                    const NodeInfo& nodeInOppositeSearch = this->bckwdInfo.getInfo(a.n);
                    if (!nodeInOppositeSearch.reached[firstCostComponent] || !successorInfo.reached[!firstCostComponent]) {
                        continue;
                    }
                    lbToTarget = nodeInOppositeSearch.potential[firstCostComponent];
                }
                successorInfo.minCosts[firstCostComponent] = costExtension_1 + lbToTarget;
                successorInfo.potential[firstCostComponent] = costExtension_1;
                successorInfo.dominanceBound[firstCostComponent] = costExtension_2;
                successorInfo.preprocessingParent[firstCostComponent] = &reversedArc;
                heap.push(&successorInfo);
            }
        }
    }

private:
    const Graph& G;
    NodeInfoContainer& fwdInfo;
    NodeInfoContainer& bckwdInfo;
};

#endif
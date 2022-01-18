//
// Created by pedro on 23.09.21.
//

#ifndef BIDIJKSTRA_BDA_H
#define BIDIJKSTRA_BDA_H

#include <algorithm>
#include <list>
#include <memory>

#include "../../datastructures/includes/HeapNew.h"
#include "../../datastructures/includes/NodeInfoContainer.h"
#include "../../datastructures/includes/Buckets_BDA.h"
#include "Solution.h"

template <typename LabelType, typename QueueType>
class BDA  {
public:
    BDA<LabelType, QueueType>(bool mainOptIndex, Graph& G, NodeInfoContainer& forwardExpander, NodeInfoContainer& backwardExpander, Preprocessor& preprocessor) :
            G{G},
            forwardExpander{forwardExpander},
            backwardExpander{backwardExpander},
            upperBounds{preprocessor.getUpperBounds()},
            mainOptIndex{mainOptIndex} {
        //To be able to prune with >= .
        upperBounds[0]++;
        upperBounds[1]++;
    }

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        assert(this->mainOptIndex != this->forwardExpander.forward);
        bool secondOptIndex = !this->mainOptIndex;
        auto getOutgoingArcs = this->forwardExpander.forward ?
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.outgoingArcs(n);} :
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.incomingArcs(n);};
        auto getIncomingArcs = this->forwardExpander.forward ?
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.incomingArcs(n);} :
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.outgoingArcs(n);};
        NodeInfo& startNodeInfo = this->forwardExpander.getStartNode();
        NodeInfo& targetNodeInfo = this->forwardExpander.getTargetNode();

        std::vector<LabelType> heapLabels(G.nodesCount);
        CostArray bestNclCosts{MAX_COST, MAX_COST};

        for (size_t i = 0; i < G.nodesCount; ++i) {
            heapLabels[i].n = i;
        }
        LabelType* startLabel = &heapLabels[startNodeInfo.n];
        startLabel->update(targetNodeInfo.potential[mainOptIndex], 0, startNodeInfo.n, MAX_DEGREE, MAX_PATH);

        QueueType Q = QueueType(this->backwardExpander.getTargetNode().potential[mainOptIndex]);
        Q.push(startLabel);
        LabelType minLabel;
        while (Q.size() != 0) {
            minLabel = *Q.pop();
            const Node n{minLabel.n};
            ++extractions;
            //printf("%lu;%u;%u;%u\n", extractions, minLabel.n, minLabel.c1, minLabel.c2);
            NodeInfo& minNodeInfo = this->forwardExpander.getInfo(n);
            NodeInfo& backwardMinNodeInfo = this->backwardExpander.getInfo(n);
            const CostType source_n_costs = this->computeCostsToNode(&minLabel, backwardMinNodeInfo);
            const CostType n_target_lb_1 = backwardMinNodeInfo.potential[mainOptIndex];
            const CostType n_target_lb_2 = backwardMinNodeInfo.potential[secondOptIndex];
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, source_n_costs, minLabel.c2, minLabel.c1);

            if (minLabel.c1 >= this->upperBounds[mainOptIndex]) {
                //printf("break!\n");
                break;
            }

            /////////////////////////////////////////////////////
            //////////////////////START NCL//////////////////////
            /////////////////////////////////////////////////////
            Neighborhood& incomingArcs{getIncomingArcs(this->G, n)};
            ArcId bestArc{INVALID_ARC};
            for (size_t i = 0; i < incomingArcs.size(); ++i) {
                Arc& a{incomingArcs[i]};
                const std::vector<MinLabel>& candidateLabels{this->backwardExpander.getInfo(a.n).permanentLabels};
                while (a.lastProcessedPred != candidateLabels.size()) {
                    const MinLabel& it{candidateLabels[a.lastProcessedPred]};
                    CostType cost_1 = it.c1 + a.c[mainOptIndex] + n_target_lb_1;
                    CostType cost_2 = it.c2 + a.c[secondOptIndex];
                    if (cost_1 >= this->upperBounds[mainOptIndex]) {
                        break;
                    }

                    if (cost_2 + n_target_lb_2 < this->upperBounds[secondOptIndex]) {
                        if (cost_1 > minLabel.c1 && cost_2 < minLabel.c2) {
                            if (cost_1 < bestNclCosts[0] ||
                                (cost_1 == bestNclCosts[0] && cost_2 < bestNclCosts[1])) {
                                bestNclCosts[0] = cost_1;
                                bestNclCosts[1] = cost_2;
                                bestArc = i;
                            }
                            break;
                        }
                    }
                    ++a.lastProcessedPred;
                }
            }
            if (bestNclCosts[0] != MAX_COST) {
                LabelType* nodeLabel = &heapLabels[n];
                assert(!nodeLabel->inQueue);
                nodeLabel->update(bestNclCosts[0], bestNclCosts[1], n, bestArc, MAX_PATH);

                Q.push(nodeLabel);
                bestNclCosts[0] = MAX_COST;
            }
            /////////////////////////////////////////////////////
            //////////////////////END NCL////////////////////////
            /////////////////////////////////////////////////////
            if (minLabel.c2 + n_target_lb_2 >= this->upperBounds[secondOptIndex]) {
                continue;
            }
            else {
                //Improve the heuristic.
                if (minNodeInfo.latestEfficientSecondComponent == MAX_COST) {
                    minNodeInfo.potential[mainOptIndex] = source_n_costs;
                }
                minNodeInfo.latestEfficientSecondComponent = minLabel.c2;
            }
            ++iterations;

            //Early solution strategy.
            const CostType n_target_ub_2 = backwardMinNodeInfo.dominanceBound[mainOptIndex];
            if (minLabel.c2 + n_target_ub_2 < this->upperBounds[secondOptIndex]) {
                this->upperBounds[secondOptIndex] = minLabel.c2 + n_target_ub_2;
                targetNodeInfo.latestEfficientSecondComponent = this->upperBounds[secondOptIndex];

                if (!solutions.empty() && solutions.back().c1 == minLabel.c1) {
                    solutions.back() = minLabel;
                    //printf("UPDATE ");
                }
                else {
                    solutions.push_back(minLabel);
                }
                //printf("Early solution with costs %u %u\n", minLabel.c1, minLabel.c2 + backwardMinNodeInfo.dominanceBound[mainOptIndex] );
                if (backwardMinNodeInfo.potential[mainOptIndex] == backwardMinNodeInfo.dominanceBound[secondOptIndex]) {
                    continue;
                }
            }
            const Neighborhood& outgoingArcs{getOutgoingArcs(this->G, n)};
            const u_int16_t predPathIndex = backwardMinNodeInfo.latestPathIndex();
            bool expanded = false;
            for (const Arc& a : outgoingArcs) {
                const Node successorNode = a.n;
                CostType costExtension_2 = minLabel.c2 + a.c[secondOptIndex];

                const NodeInfo& successorInfo{this->forwardExpander.getInfo(successorNode)};
                const NodeInfo& reverseSuccessorInfo{this->backwardExpander.getInfo(successorNode)};
                if (reverseSuccessorInfo.potential[mainOptIndex] > this->upperBounds[mainOptIndex]) {
                    continue;
                }
                //Second cost component of efficient paths should decrease.
                if (costExtension_2 >= successorInfo.latestEfficientSecondComponent) {
                    continue;
                }
                if (costExtension_2 + reverseSuccessorInfo.potential[secondOptIndex] >= this->upperBounds[secondOptIndex]) {
                    continue;
                }
                CostType costExtension_1 = source_n_costs + a.c[mainOptIndex];
                CostType costToTarget_1 = costExtension_1 + reverseSuccessorInfo.potential[mainOptIndex];
                if (costToTarget_1 > this->upperBounds[mainOptIndex]) {
                    continue;
                }

                expanded = true;
                LabelType* successorLabel{&heapLabels[successorNode]};
                if (successorLabel->inQueue) {
                    CostType c1 = successorLabel->c1;
                    CostType c2 = successorLabel->c2;
                    if (costToTarget_1 < c1 || (costToTarget_1 == c1 && costExtension_2 < c2)) {
                        successorLabel->update(costToTarget_1, costExtension_2, successorNode, a.revArcIndex, predPathIndex);
                        Q.decrease_key(c1, successorLabel);
                    }
                }
                //No label for successorNode in queue, so just push the updated version of successorLabel!
                else {
                    //Label with key (c+pi)_1, c_2 to resemble sorting in Saman's paper.
                    successorLabel->update(costToTarget_1, costExtension_2, successorNode, a.revArcIndex, predPathIndex);
                    //printf("\t\tBDApush %u %u %u\n", successorNode, costToTarget_1, costExtension_2);
                    Q.push(successorLabel);
                }
            }
            if (expanded) {
                auto& perm = backwardMinNodeInfo.permanentLabels;
                if (!perm.empty() && perm.back().c1 == source_n_costs && perm.back().c2 > minLabel.c2) {
                    MinLabel& lastPermanentLabel{perm.back()};
                    lastPermanentLabel.pathId = minLabel.pathId;
                    lastPermanentLabel.predArcId = minLabel.predArcId;
                    lastPermanentLabel.c2 = minLabel.c2;
                }
                else {
                    perm.push_back({source_n_costs, minLabel.c2, minLabel.predArcId, minLabel.pathId});
                }
            }
        }
        //printSolutions();
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.size();
    }

    /**
     * Our labels only store the reduced costs of a path.
     * For the expansion of a path along the outgoing arcs of the path's final node corresponding node n, we need
     * to recompute the path's costs out of its reduced costs.
     */
    inline CostType computeCostsToNode(const LabelType* l, const NodeInfo& revExpanderInfo) const {
        return l->c1 - revExpanderInfo.potential[this->mainOptIndex];
    }

    void printSolutions() const {
        auto getIncomingArcs = this->backwardExpander.forward ?
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.outgoingArcs(n);} :
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.incomingArcs(n);};
        for (const LabelBuckets& solution : this->solutions) {
            std::vector<std::pair<Node, CostArray>> path;
            //From this node until the target, the path follows the preprocessing path.
            const NodeInfo& lastSearchNode = this->backwardExpander.getInfo(solution.n);
            const NodeInfo* iterator = &lastSearchNode;
            uint16_t pathId = solution.pathId;
            ArcId lastArcId = solution.predArcId;
            CostType c1 = solution.c1 - lastSearchNode.potential[mainOptIndex];
            CostType c2 = solution.c2;
            //minLabel->c2 += oppositeMinNodeInfo.dominanceBound[mainOptIndex];
            //printf("Node: %u with costs %u %u\n", solution->n, c1, c2);

            while (iterator) {
                path.push_back(std::make_pair(iterator->n,CostArray{c1, c2}));
                if (iterator->n == this->backwardExpander.targetNode) {
                    break;
                }
                if (lastArcId != INVALID_ARC) {
                    const Neighborhood& incomingArcs{getIncomingArcs(this->G, iterator->n)};
                    const Arc& a{incomingArcs[lastArcId]};
                    c1 = c1-a.c[mainOptIndex];
                    c2 = c2-a.c[!mainOptIndex];
                    //printf("Node: %u with costs %u %u\n", a.n, c1, c2);
                    iterator = &this->backwardExpander.getInfo(a.n);
                    lastArcId = iterator->permanentLabels[pathId].predArcId;
                    pathId = iterator->permanentLabels[pathId].pathId;
                }
            }
            std::reverse(path.begin(), path.end());
            //Now, we backtrack the path in the dijkstra tree that we got from the preprocessing phase.
            c1 = solution.c1 - lastSearchNode.potential[mainOptIndex];
            c2 = solution.c2;
            const Arc* predArc{lastSearchNode.preprocessingParent[mainOptIndex]};
            while (predArc != nullptr) {
                c1 = c1+predArc->c[mainOptIndex];
                c2 = c2+predArc->c[!mainOptIndex];
                path.push_back(std::make_pair(predArc->n, CostArray{c1,c2}));
                //printf("Node: %u with costs %u %u\n", predArc->n, c1, c2);
                const NodeInfo& parentNodeInfo = backwardExpander.getInfo(predArc->n);
                predArc = parentNodeInfo.preprocessingParent[mainOptIndex];
            }
            for (const auto& n : path) {
                printf("%u (%u,%u)\n", n.first, n.second[0], n.second[1]);
            }
            printf("\n\n");
        }
    }

private:
    Graph& G;
    NodeInfoContainer& forwardExpander;
    NodeInfoContainer& backwardExpander;

    CostArray& upperBounds;

    std::list<LabelType> solutions;

    const bool mainOptIndex;
};

#endif //BIDIJKSTRA_BDA_H

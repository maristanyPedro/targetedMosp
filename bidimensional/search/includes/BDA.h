//
// Created by pedro on 23.09.21.
//

#ifndef BIDIJKSTRA_BDA_H
#define BIDIJKSTRA_BDA_H

#include <algorithm>
#include <list>
#include <memory>

#include "../../datastructures/includes/BinaryHeap.h"
#include "../../datastructures/includes/NodeInfoContainer.h"
#include "../../datastructures/includes/BucketsQueue.h"
#include "Solution.h"

template <typename LabelType, typename QueueType>
class BDA  {
public:
    BDA<LabelType, QueueType>(bool mainOptIndex, Graph& G, NodeInfoContainer& fNic, NodeInfoContainer& bNic, Preprocessor& preprocessor) :
            G{G},
            forwardNodeInfos{fNic},
            backwardNodeInfos{bNic},
            dominanceBound{preprocessor.getUpperBounds()},
            mainOptIndex{mainOptIndex} {
        //To be able to prune with >= .
        dominanceBound[0]++;
        dominanceBound[1]++;
    }

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        assert(this->mainOptIndex != this->forwardNodeInfos.forward);
        bool secondOptIndex = !this->mainOptIndex;
        auto getOutgoingArcs = this->forwardNodeInfos.forward ?
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.outgoingArcs(n);} :
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.incomingArcs(n);};
        auto getIncomingArcs = this->forwardNodeInfos.forward ?
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.incomingArcs(n);} :
                               [](Graph& graph, const Node& n) -> Neighborhood & {return graph.outgoingArcs(n);};
        NodeInfo& startNodeInfo = this->forwardNodeInfos.getStartNode();
        NodeInfo& targetNodeInfo = this->forwardNodeInfos.getTargetNode();

        std::vector<LabelType> heapLabels(G.nodesCount);
        CostArray bestNclCosts{MAX_COST, MAX_COST};

        for (size_t i = 0; i < G.nodesCount; ++i) {
            heapLabels[i].n = i;
        }
        LabelType* startLabel = &heapLabels[startNodeInfo.n];
        startLabel->update(targetNodeInfo.potential[mainOptIndex], 0, startNodeInfo.n, MAX_DEGREE, MAX_PATH);

        QueueType Q = QueueType(this->backwardNodeInfos.getTargetNode().potential[mainOptIndex]);
        Q.push(startLabel);
        LabelType minLabel;
        while (Q.size() != 0) {
            minLabel = *Q.pop();
            const Node n{minLabel.n};
            ++extractions;
            //printf("%lu;%u;%u;%u\n", extractions, minLabel.n, minLabel.c1, minLabel.c2);
            NodeInfo& minNodeInfo = this->forwardNodeInfos.getInfo(n);
            NodeInfo& backwardMinNodeInfo = this->backwardNodeInfos.getInfo(n);
            const CostType source_n_costs = this->computeCostsToNode(&minLabel, backwardMinNodeInfo);
            const CostType n_target_lb_1 = backwardMinNodeInfo.potential[mainOptIndex];
            const CostType n_target_lb_2 = backwardMinNodeInfo.potential[secondOptIndex];
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, source_n_costs, minLabel.c2, minLabel.c1);

            if (minLabel.c1 >= this->dominanceBound[mainOptIndex]) {
                //printf("break!\n");
                break;
            }

            /////////////////////////////////////////////////////
            //////////////////////START NCL//////////////////////
            /////////////////////////////////////////////////////
            Neighborhood& incomingArcs{getIncomingArcs(this->G, n)};
            ArcId bestArc{INVALID_ARC};
            uint16_t pathId{MAX_PATH};
            for (size_t i = 0; i < incomingArcs.size(); ++i) {
                Arc& a{incomingArcs[i]};
                const std::vector<MinLabel>& candidateLabels{this->backwardNodeInfos.getInfo(a.n).permanentLabels};
                while (a.lastProcessedPred != candidateLabels.size()) {
                    const MinLabel& it{candidateLabels[a.lastProcessedPred]};
                    CostType cost_1 = it.c1 + a.c[mainOptIndex] + n_target_lb_1;
                    CostType cost_2 = it.c2 + a.c[secondOptIndex];
                    if (cost_1 >= this->dominanceBound[mainOptIndex]) {
                        break;
                    }

                    if (cost_2 + n_target_lb_2 < this->dominanceBound[secondOptIndex]) {
                        if (cost_1 > minLabel.c1 && cost_2 < minLabel.c2) {
                            if (cost_1 < bestNclCosts[0] ||
                                (cost_1 == bestNclCosts[0] && cost_2 < bestNclCosts[1])) {
                                bestNclCosts[0] = cost_1;
                                bestNclCosts[1] = cost_2;
                                bestArc = i;
                                pathId = a.lastProcessedPred;
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
                nodeLabel->update(bestNclCosts[0], bestNclCosts[1], n, bestArc, pathId);

                Q.push(nodeLabel);
                bestNclCosts[0] = MAX_COST;
            }
            /////////////////////////////////////////////////////
            //////////////////////END NCL////////////////////////
            /////////////////////////////////////////////////////
            if (minLabel.c2 + n_target_lb_2 >= this->dominanceBound[secondOptIndex]) {
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
            if (minLabel.c2 + n_target_ub_2 < this->dominanceBound[secondOptIndex]) {
                this->dominanceBound[secondOptIndex] = minLabel.c2 + n_target_ub_2;
                targetNodeInfo.latestEfficientSecondComponent = this->dominanceBound[secondOptIndex];

                if (!solutions.empty() && solutions.back().c1 == minLabel.c1) {
                    solutions.back() = minLabel;
                    //printf("UPDATE ");
                }
                else {
                    solutions.push_back(minLabel);
                }
                if (backwardMinNodeInfo.potential[mainOptIndex] == backwardMinNodeInfo.dominanceBound[secondOptIndex]) {
                    continue;
                }
            }
            u_int16_t predPathIndex = backwardMinNodeInfo.latestPathIndex();
            auto& permanentLabels = backwardMinNodeInfo.permanentLabels;
            bool improvesLastPermanentLabel =
                    lastLabelReplacementNeeded(permanentLabels, source_n_costs, minLabel.c2);
            if (improvesLastPermanentLabel) {
                predPathIndex--;
            }
            bool expanded = false;
            const Neighborhood& outgoingArcs{getOutgoingArcs(this->G, n)};
            for (const Arc& a : outgoingArcs) {
                const Node successorNode = a.n;
                CostType costExtension_2 = minLabel.c2 + a.c[secondOptIndex];

                const NodeInfo& successorInfo{this->forwardNodeInfos.getInfo(successorNode)};
                const NodeInfo& reverseSuccessorInfo{this->backwardNodeInfos.getInfo(successorNode)};
                if (reverseSuccessorInfo.potential[mainOptIndex] > this->dominanceBound[mainOptIndex]) {
                    continue;
                }
                //Second cost component of efficient paths should decrease.
                if (costExtension_2 >= successorInfo.latestEfficientSecondComponent) {
                    continue;
                }
                if (costExtension_2 + reverseSuccessorInfo.potential[secondOptIndex] >= this->dominanceBound[secondOptIndex]) {
                    continue;
                }
                CostType costExtension_1 = source_n_costs + a.c[mainOptIndex];
                CostType costToTarget_1 = costExtension_1 + reverseSuccessorInfo.potential[mainOptIndex];
                if (costToTarget_1 > this->dominanceBound[mainOptIndex]) {
                    continue;
                }

                expanded = true;
                LabelType* successorLabel{&heapLabels[successorNode]};
                if (successorLabel->inQueue) {
                    CostType c1 = successorLabel->c1;
                    CostType c2 = successorLabel->c2;
                    if (costToTarget_1 < c1 || (costToTarget_1 == c1 && costExtension_2 < c2)) {
                        successorLabel->update(costToTarget_1, costExtension_2, successorNode, a.revArcIndex, predPathIndex);
                        Q.decreaseKey(c1, successorLabel);
                    }
                }
                //No label for successorNode in queue, so just push the updated version of successorLabel!
                else {
                    //Label with key (c+pi)_1, c_2 to resemble sorting in Ahmadi et al. paper
                    successorLabel->update(costToTarget_1, costExtension_2, successorNode, a.revArcIndex, predPathIndex);
                    //printf("\t\tBDApush %u %u %u\n", successorNode, costToTarget_1, costExtension_2);
                    Q.push(successorLabel);
                }
            }
            if (expanded) {
                if (improvesLastPermanentLabel) {
                    MinLabel& lastPermanentLabel{permanentLabels.back()};
                    lastPermanentLabel.pathId = minLabel.pathId;
                    lastPermanentLabel.predArcId = minLabel.predArcId;
                    lastPermanentLabel.c2 = minLabel.c2;
                }
                else {
                    permanentLabels.push_back({source_n_costs, minLabel.c2, minLabel.predArcId, minLabel.pathId});
                }
            }
        }
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.size();
    }

    /**
     * Our labels only store the reduced costs of a path.
     * For the expansion of a path along the outgoing arcs of the path's final node corresponding node n, we need
     * to recompute the path's costs out of its reduced costs.
     */
    inline CostType computeCostsToNode(const LabelType* l, const NodeInfo& reverseSearchNodeInfo) const {
        return l->c1 - reverseSearchNodeInfo.potential[this->mainOptIndex];
    }

    /**
     * Sometimes the last label of the label of a list of permanent labels is dominated by a newly extracted label.
     * In this case, the first cost component of the two labels coincide and the new label improves the last label in the list
     * w.r.t. the second cost component. If that's the case, the last label in the list is dominated and has to be replaced
     * by the new one. This function checks if a replacement has to be triggered.
     * @param labelList the list of permanent labels whose last label is being analyzed.
     * @param c1 the first cost component of the newly extracted label
     * @param c2 the second cost component of the newly extractd label
     * @return TRUE if and only if the last label in @param labelList is dominated by (c1, c2)
     */
    inline bool lastLabelReplacementNeeded(std::vector<MinLabel>& labelList, CostType c1, CostType c2) {
        if (!labelList.empty() && labelList.back().c1 == c1 && labelList.back().c2 > c2) {
            return true;
        }
        return false;
    }

    void printSolutions() const {
        auto getIncomingArcs = this->backwardNodeInfos.forward ?
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.outgoingArcs(n);} :
                               [](const Graph& graph, const Node& n) -> const Neighborhood & {return graph.incomingArcs(n);};
        for (const LabelType& solution : this->solutions) {
            std::vector<std::pair<Node, CostArray>> path;
            //From this node until the target, the path follows the preprocessing path.
            const NodeInfo& lastSearchNode = this->backwardNodeInfos.getInfo(solution.n);
            const NodeInfo* iterator = &lastSearchNode;
            uint16_t pathId = solution.pathId;
            ArcId lastArcId = solution.predArcId;
            CostType c1 = solution.c1 - lastSearchNode.potential[mainOptIndex];
            CostType c2 = solution.c2;
            printf("Printing path with costs %u, %u at node %u\n", c1, c2, iterator->n);
            //minLabel->c2 += oppositeMinNodeInfo.dominanceBound[mainOptIndex];
            //printf("Node: %u with costs %u %u\n", solution->n, c1, c2);

            while (iterator) {
                path.emplace_back(iterator->n,CostArray{c1, c2});
                if (iterator->n == this->backwardNodeInfos.targetNode) {
                    break;
                }
                if (lastArcId != INVALID_ARC) {
                    const Neighborhood& incomingArcs{getIncomingArcs(this->G, iterator->n)};
                    const Arc& a{incomingArcs[lastArcId]};
                    c1 = c1-a.c[mainOptIndex];
                    c2 = c2-a.c[!mainOptIndex];
                    //printf("Node: %u with costs %u %u\n", a.n, c1, c2);
                    iterator = &this->backwardNodeInfos.getInfo(a.n);
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
                path.emplace_back(predArc->n, CostArray{c1,c2});
                //printf("Node: %u with costs %u %u\n", predArc->n, c1, c2);
                const NodeInfo& parentNodeInfo = backwardNodeInfos.getInfo(predArc->n);
                predArc = parentNodeInfo.preprocessingParent[mainOptIndex];
            }
            for (const auto& n : path) {
                printf("%u (%u,%u)\n", n.first, n.second[0], n.second[1]);
            }
            printf("\n\n");
            //assert(path.begin()->second[0] == 0 && path.begin()->second[1] == 0);
        }
    }

private:
    Graph& G;
    NodeInfoContainer& forwardNodeInfos;
    NodeInfoContainer& backwardNodeInfos;

    CostArray& dominanceBound;

    std::list<LabelType> solutions;

    const bool mainOptIndex;
};

#endif //BIDIJKSTRA_BDA_H

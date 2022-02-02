#ifndef MDA_H_
#define MDA_H_

#include "../../datastructures/includes/BinaryHeapMosp.h"
#include "../../datastructures/includes/NodeInfoContainer.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/costs.h"

#include "SolutionsList.h"
#include "Solution.h"

/**
 * See https://arxiv.org/abs/2110.10978 for details about this algorithm.
 */
class MDA  {
    typedef std::list<std::pair<CostArray, size_t>> CandidateLabels;
public:
    MDA(const Graph& G, NodeInfoContainer& nic, Preprocessor& preprocessor) :
            G{G},
            nic{nic},
            upperBounds{preprocessor.getUpperBounds()} {}

    ~MDA() = default;

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};

        NodeInfo &startNodeInfo = this->nic.getInfo(this->G.source);
        NodeInfo &targetNodeInfo = this->nic.getInfo(this->G.target);

        std::vector<Label> heapLabels(G.nodesCount);
        std::vector<CandidateLabels> nextCandidateLabels(G.arcsCount);

        for (size_t i = 0; i < G.nodesCount; ++i) {
            heapLabels[i].n = i;
        }
        Label *startLabel = &heapLabels[startNodeInfo.n];
        startLabel->update(CostArray{0,0,0}, startNodeInfo.n, INVALID_ARC, MAX_PATH);

        BinaryHeapMosp heap = BinaryHeapMosp(1);
        heap.push(startLabel);
        Label minLabel;
        CostArray bestCosts{MAX_COST, MAX_COST, MAX_COST};
        CostArray lbToTarget{MAX_COST, MAX_COST, MAX_COST};
        CostArray expandedAlongArcs{MAX_COST, MAX_COST, MAX_COST};
        while (heap.size()) {
            minLabel = *heap.pop();
            const Node n{minLabel.n};
            ++extractions;
            NodeInfo &minNodeInfo = this->nic.getInfo(n);
            //CostArray source_n_costs = substract(minLabel.c, minNodeInfo.potential);
            minNodeInfo.efficientCosts.push_back(minLabel.c);

            /////////////////////////////////////////////////////
            //////////////////////START NCL//////////////////////
            /////////////////////////////////////////////////////
            const Neighborhood &incomingArcs{this->G.incomingArcs(n)};
            ArcId bestArc{INVALID_ARC};
            size_t pathId{MAX_PATH};
            for (size_t i = 0; i < incomingArcs.size(); ++i) {
                const Arc &a{incomingArcs[i]};
                CandidateLabels &candidateLabels{nextCandidateLabels[a.id / 2]};
                if (!candidateLabels.empty()) {
                    CandidateLabels::const_iterator it = candidateLabels.begin();
                    while (it != candidateLabels.end()) {
                        const CostArray& c{(*it).first};
                        lbToTarget = add(c, minNodeInfo.potential);
                        if (!weakDominates(this->upperBounds, lbToTarget)) {
                            if (lexSmaller(minLabel.c, c)) {
                                if (!dominatedAtFront(targetNodeInfo.efficientCosts, lbToTarget)) {
                                    if (!dominatedAtFront(minNodeInfo.efficientCosts, c)) {
                                        if (lexSmaller(c, bestCosts)) {
                                            bestCosts = c;
                                            bestArc = i;
                                            pathId = (*it).second;
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        it = candidateLabels.erase(it);
                    }
                }
            }
            if (bestCosts[0] != MAX_COST) {
                Label *nodeLabel = &heapLabels[n];
                assert(!heap.contains(nodeLabel));
                nodeLabel->update(bestCosts, n, bestArc, pathId);

                heap.push(nodeLabel);
                bestCosts[0] = MAX_COST;
            }
            /////////////////////////////////////////////////////
            //////////////////////END NCL////////////////////////
            /////////////////////////////////////////////////////

            //printf("%lu %u %u %u\n", extractions, minLabel.n, source_n_costs, minLabel.c2);
            ++iterations;
            if (n == targetNodeInfo.n) {
                solutions.solutions.push_back(minLabel);
                continue;
            }

            const Neighborhood &outgoingArcs{this->G.outgoingArcs(n)};
            const u_int16_t predPathIndex = minNodeInfo.latestPathIndex();
            bool expanded = false;

            for (const Arc &a : outgoingArcs) {
                expandedAlongArcs = add(minLabel.c, a.c);
                const Node successorNode = a.n;
                const NodeInfo &successorInfo{this->nic.getInfo(successorNode)};

                if (dominatedAtFront(successorInfo.efficientCosts, expandedAlongArcs)) {
                    continue;
                }
                lbToTarget = add(expandedAlongArcs, successorInfo.potential);
                if (weakDominates(this->upperBounds, lbToTarget)) {
                    continue;
                }
                if (dominatedAtFront(targetNodeInfo.efficientCosts, lbToTarget)) {
                    continue;
                }

                expanded = true;
                Label *successorLabel{&heapLabels[successorNode]};
                //Check if there is a label for the current successor node already in the heap.
                if (heap.contains(successorLabel)) {
                    const CostArray& c = successorLabel->c;
                    if (lexSmaller(expandedAlongArcs, c)) {
                        const Arc &revArc = this->G.incomingArcs(successorNode)[successorLabel->predArcId];
                        auto &tailLabelCandidates{nextCandidateLabels[revArc.id / 2]};
                        tailLabelCandidates.emplace_front(c, successorLabel->pathId);
                        successorLabel->update(expandedAlongArcs, successorNode, a.revArcIndex, predPathIndex);
//                        printf("\t\tdecKey %u %u %u\n", successorNode, costExtension_1, costExtension_2);
                        heap.decreaseKey(successorLabel);
                    } else {
                        const Arc &revArc = this->G.incomingArcs(successorNode)[a.revArcIndex];
                        auto &tailLabelCandidates{nextCandidateLabels[revArc.id / 2]};
//                            printf("\t\tfor later: %u %u %u\n", successorNode, costExtension_1, costExtension_2);
                        tailLabelCandidates.emplace_back(expandedAlongArcs, predPathIndex);
                    }
                } else {
                    successorLabel->update(expandedAlongArcs, successorNode, a.revArcIndex, predPathIndex);
//                    printf("\t\tpush %u %u %u\n", successorNode, costExtension_1, costExtension_2);
                    heap.push(successorLabel);
                }
            }
            if (expanded) {
                minNodeInfo.pathIds.push_back(minLabel.pathId);
                minNodeInfo.incomingEfficientArcs.push_back(minLabel.predArcId);
            }
        }
        //printSolutions();
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.solutions.size();
    }

    void printSolutions() const {
        printf("MDA printing!\n");
        for (const Label& solution : this->solutions.solutions) {
            std::vector<std::pair<Node, CostArray>> path;
            //From this node until the target, the path follows the preprocessing path.
            const NodeInfo& lastSearchNode = this->nic.getInfo(solution.n);
            const NodeInfo* iterator = &lastSearchNode;
            uint16_t pathId = solution.pathId;
            ArcId lastArcId = solution.predArcId;
            const CostArray& costsAndPotential = solution.c;
            CostArray costs = substract(costsAndPotential, lastSearchNode.potential);

            //printf("Node: %u with costs %u %u\n", solution->n, c1, c2);

            while (iterator) {
                path.emplace_back(iterator->n,CostArray{costs});
                if (iterator->n == this->G.source) {
                    break;
                }
                if (lastArcId != INVALID_ARC) {
                    const Neighborhood& incomingArcs{this->G.incomingArcs(iterator->n)};
                    const Arc& a{incomingArcs[lastArcId]};
                    costs = substract(costs, a.c);
                    //printf("Node: %u with costs %u %u\n", a.n, c1, c2);
                    iterator = &this->nic.getInfo(a.n);
                    lastArcId = iterator->incomingEfficientArcs[pathId];
                    pathId = iterator->pathIds[pathId];
                }
            }
            std::reverse(path.begin(), path.end());
            for (const auto& n : path) {
                printf("%u (%u,%u, %u)\n", n.first, n.second[0], n.second[1], n.second[2]);
            }
            printf("\n\n");
        }
    }


private:
    const Graph& G;
    NodeInfoContainer& nic;
    const CostArray upperBounds{0, 0, 0};
    SolutionsList solutions;
};

#endif

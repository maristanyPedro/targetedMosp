//
// Created by pedro on 23.09.21.
//

#ifndef MDA_STAR_H
#define MDA_STAR_H

#include <unordered_map>

#include "../../datastructures/includes/BinaryHeapMosp.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/MemoryPool.h"

#include "Permanents.h"
#include "SolutionsList.h"
#include "Solution.h"

/**
 * See https://arxiv.org/abs/2110.10978 for details about this algorithm.
 */
class TargetedMDA  {
public:
    TargetedMDA(Graph& G, const std::vector<CostArray>& potential) :
            G{G},
            truncatedFront(G.nodesCount),
            targetFront{truncatedFront[G.target]},
            permanentCounter(G.nodesCount, 0),
            potential(potential),
            nextCandidateLabels(G.arcsCount),
            permanentLabels() {}


    inline bool dominatedAtTarget(Label* l) {
        if (this->sols == l->knownTargetElements) {
            return false;
        }
        else {
            bool dominated = truncatedDominance(targetFront, l->c);
            l->knownTargetElements = this->sols;
            return dominated;
        }
    }

    void nextQueuePath(const Label* minLabel, const TruncatedFront& currentFront, Pool<Label>* labelsPool, BinaryHeapMosp& heap) {
        const Node n{minLabel->n};
        const Neighborhood &incomingArcs{this->G.incomingArcs(n)};
        Label* newLabel = nullptr;
        List* minCandidateLabels{nullptr};
        bool success{false};
        for (const Arc& a : incomingArcs) {
            List& candidateLabels{nextCandidateLabels[a.id]};
            if (!candidateLabels.empty()) {
                Label* l = candidateLabels.first;
                while (l != nullptr) {
                    if (newLabel != nullptr && !lexSmaller(l->c, newLabel->c)) {
                        if (dominates(newLabel->c, l->c) || dominates(minLabel->c, l->c)) {
                            candidateLabels.pop_front();
                            labelsPool->free(l);
                        }
                        break;
                    }
                    if (!dominates(minLabel->c, l->c)) {
                        if (!dominatedAtTarget(l)) {
                            if (!l->nclChecked && (this->permanentCounter[n] - l->knownPermanentElements == 1)) {
                                l->nclChecked = true;
                            }
                            if (l->nclChecked || !truncatedDominance(currentFront, l->c)) {
                                l->nclChecked = true;
                                l->knownTargetElements = this->sols;
                                success = true;
                                newLabel = l;
                                minCandidateLabels = &candidateLabels;
                                break;
                            }
                        }
                    }
                    assert(l != newLabel);
                    candidateLabels.pop_front();
                    labelsPool->free(l);
                    l = candidateLabels.first;
                }
            }
        }
        if (success) {
            heap.push(newLabel);
            assert(newLabel->n == n);
            heapLabels[n] = newLabel;
            minCandidateLabels->pop_front();
        } else {
            heapLabels.erase(n);
        }
    }

    void propagate(Label* minLabel, const CostArray& source_n_costs, BinaryHeapMosp& heap, Pool<Label>* labelsPool) {
        const Node n{minLabel->n};
        const Neighborhood &outgoingArcs{this->G.outgoingArcs(n)};
        const size_t predPathIndex = permanentLabels.getCurrentIndex();

        bool expanded = false;
        CostArray costVector = source_n_costs;
        for (const Arc &a : outgoingArcs) {
            costVector = source_n_costs;
            const Node successorNode = a.n;
            addInPlace(costVector, a.c);

            expanded = true;
            auto it = heapLabels.find(successorNode);
            //Check if there is a label for the current successor node already in the heap.
            if (it != heapLabels.end()) {
                Label *successorLabel{it->second};
                assert(successorLabel->n == successorNode);
                if (lexSmaller(costVector, successorLabel->c)) {
                    if (truncatedDominance(targetFront, costVector) ||
                        truncatedDominance(truncatedFront[successorNode], costVector)) {
                        continue;
                    }
                    expanded = true;

                    Label* l = labelsPool->newItem();
                    l->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols, this->permanentCounter[successorNode]);
                    it->second = l;
                    heap.decreaseKey(successorLabel, l);
                    if (dominatesLexSmaller(costVector, successorLabel->c)) {
                        labelsPool->free(successorLabel);
                        continue;
                    }
                    else {
                        const Arc &revArc = this->G.incomingArcs(successorNode)[successorLabel->predArcId];
                        auto &tailLabelCandidates{nextCandidateLabels[revArc.id]};
                        tailLabelCandidates.push_front(successorLabel);
                    }
                } else {
                    if (dominatesLexSmaller(successorLabel->c, costVector)) {
                        continue;
                    }
                    expanded = true;
                    const Arc &revArc = this->G.incomingArcs(successorNode)[a.revArcIndex];
                    auto &tailLabelCandidates{nextCandidateLabels[revArc.id]};
                    Label* l = labelsPool->newItem();
                    l->update(costVector, successorNode, a.revArcIndex, predPathIndex, 0, 0);
                    tailLabelCandidates.push_back(l);
                }
            } else {
                if (truncatedDominance(targetFront, costVector) || truncatedDominance(truncatedFront[successorNode], costVector)) {
                    continue;
                }
                expanded = true;
                Label* successorLabel = labelsPool->newItem();
                successorLabel->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols, this->permanentCounter[successorNode]);
                heapLabels.insert(std::make_pair(successorNode, successorLabel));
                heap.push(successorLabel);
            }
        }
        if (expanded) {
            permanentLabels.addElement(minLabel->pathId, minLabel->predArcId);
        }
        labelsPool->free(minLabel);
        if (heapLabels.size() > maxHeapLabelsSize) {
            maxHeapLabelsSize = heapLabels.size();
        }
    }



    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        auto start_bda = std::chrono::high_resolution_clock::now();
        Node target = G.target;

        Pool<Label>* labelsPool = new Pool<Label>();

        Label* startLabel = labelsPool->newItem();
        startLabel->update(potential[G.source], G.source, INVALID_ARC, MAX_PATH,0, 0);

        BinaryHeapMosp heap = BinaryHeapMosp(1);
        heap.push(startLabel);
        heapLabels[G.source] = startLabel;
        CostArray source_n_costs{generate()};
        while (heap.size()) {
            //printf("%lu\n", heap.size());
            Label *minLabel = heap.pop();
            const Node n{minLabel->n};
            ++extractions;
            //printf("Extracting %u %u %u %u\n", n, minLabel->c[0], minLabel->c[1], minLabel->c[2]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, minLabel->c[0], minLabel->c[1], minLabel->c[2]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, source_n_costs[0], source_n_costs[1], source_n_costs[2]);
//            minNodeInfo.efficientCosts.push_back(minLabel.c);
            TruncatedFront& currentFront{this->truncatedFront[n]};
            //truncatedInsertionBackward(currentFront, minLabel->c);
            truncatedInsertion(currentFront, minLabel->c);
            permanentCounter[n]++;
            nextQueuePath(minLabel, currentFront, labelsPool, heap);
            ++iterations;
            if (n == target) {
                //printf("Solution %u %u %u\n", minLabel->c[0], minLabel->c[1], minLabel->c[2]);
                solutions.solutions.push_back(minLabel);
                ++sols;
                continue;
            }
            source_n_costs = substract(minLabel->c, potential[n]);
            propagate(minLabel, source_n_costs, heap, labelsPool);
        }
        auto end_bda = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_bda = end_bda - start_bda;
        solutionData.duration = duration_bda.count();
        solutionData.permanents = permanentLabels.size();
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.solutions.size();
        solutionData.memoryConsumption = this->memory(maxHeapLabelsSize, labelsPool->size());
        solutionData.maxHeapSize = heap.maxSize();
    }

    size_t memory(size_t maxHeapSize, size_t labelsPoolSize) const {
        size_t heapTrackingSize = maxHeapSize*sizeof(Label*);
        size_t epxloredPathsSize = labelsPoolSize*sizeof(Label);
        size_t listsSize = sizeof(List)*G.arcsCount;
        //size_t frontForNclSize = permanents * DIM * sizeof(CostType);
        return heapTrackingSize + epxloredPathsSize + listsSize;
    }

private:
    Graph& G;
    std::vector<TruncatedFront> truncatedFront;
    TruncatedFront& targetFront;
    std::vector<uint32_t> permanentCounter;
    const std::vector<CostArray>& potential;
    std::vector<List> nextCandidateLabels;
    std::unordered_map<Node, Label*> heapLabels;
    Permanents permanentLabels;
    size_t sols{0};
    size_t maxHeapLabelsSize{0};
    SolutionsList solutions;
};

#endif

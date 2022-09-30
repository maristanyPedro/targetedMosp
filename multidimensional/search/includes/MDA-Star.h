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
    void merge(TruncatedFront& front, const CostArray& c) {
        if (front.empty()) {
            front.push_back({c[1], c[2]});
            return;
        }
        auto it = front.begin();
        while (it != front.end() && (*it)[0] < c[1]) {
            ++it;
        }
        it  = front.insert(it, {c[1], c[2]});
        ++it;
        while (it != front.end()) {
            if (c[2] <= (*it)[1]) {
                it = front.erase(it);
            }
            else {
//                ++it;
                break;
            }
        }
    }

    bool dominated(const TruncatedFront& front, const CostArray& c) {
        if (front.empty()) {
            return false;
        }
        auto it = front.begin();
        while (it != front.end() && (*it)[0] <= c[1]) {
            if ((*it)[1] <= c[2]) {
                return true;
            }
            ++it;
        }
        return false;
    }
public:
    TargetedMDA(Graph& G, const std::vector<CostArray>& potential) :
            G{G},
            truncatedFront(G.nodesCount),
            potential(potential),
            permanentLabels() {}

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        auto start_bda = std::chrono::high_resolution_clock::now();
        Node target = G.target;

        Pool<Label>* labelsPool = new Pool<Label>();

        std::unordered_map<Node, Label*> heapLabels;
        std::vector<List> nextCandidateLabels(G.arcsCount);
        size_t maxHeapLabelsSize = 0;
        Label* startLabel = labelsPool->newItem();
        startLabel->update(potential[G.source], G.source, INVALID_ARC, MAX_PATH,0);

        BinaryHeapMosp heap = BinaryHeapMosp(1);
        heap.push(startLabel);
        heapLabels[G.source] = startLabel;
        CostArray source_n_costs{MAX_COST, MAX_COST, MAX_COST};
        TruncatedFront& targetFront{this->truncatedFront[this->G.target]};
        while (heap.size()) {
            //printf("%lu\n", heap.size());
            Label *minLabel = heap.pop();
            const Node n{minLabel->n};
            ++extractions;
            //printf("Extracting %u %u %u %u\n", n, minLabel->c[0], minLabel->c[1], minLabel->c[2]);
            source_n_costs = substract(minLabel->c, potential[n]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, minLabel->c[0], minLabel->c[1], minLabel->c[2]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, source_n_costs[0], source_n_costs[1], source_n_costs[2]);
//            minNodeInfo.efficientCosts.push_back(minLabel.c);
            TruncatedFront& currentFront{this->truncatedFront[n]};
            merge(currentFront, minLabel->c);
            /////////////////////////////////////////////////////
            //////////////////////START NCL//////////////////////
            /////////////////////////////////////////////////////
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
                            if (dominates(newLabel->c, l->c)) {
                                candidateLabels.pop_front();
                                labelsPool->free(l);
                            }
                            break;
                        }
                        if (!dominates(minLabel->c, l->c)) {
                            if (this->sols == l->knownTargetElements || !dominated(targetFront, l->c)) {
                                if (l->nclChecked || !dominated(currentFront, l->c)) {
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
            /////////////////////////////////////////////////////
            //////////////////////END NCL////////////////////////
            /////////////////////////////////////////////////////
            ++iterations;
            if (n == target) {
                //printf("Solution %u %u %u\n", minLabel->c[0], minLabel->c[1], minLabel->c[2]);
                solutions.solutions.push_back(minLabel);
                ++sols;
                continue;
            }
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
                        if (dominated(targetFront, costVector) ||
                            dominated(truncatedFront[successorNode], costVector)) {
                            continue;
                        }
                        expanded = true;
                        const Arc &revArc = this->G.incomingArcs(successorNode)[successorLabel->predArcId];
                        auto &tailLabelCandidates{nextCandidateLabels[revArc.id]};
                        Label* l = labelsPool->newItem();
                        l->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols);
                        it->second = l;
                        heap.decreaseKey(successorLabel, l);
                        if (costVector[1] <= successorLabel->c[1] && costVector[2] <= successorLabel->c[2]) {
                            labelsPool->free(successorLabel);
                            continue;
                        }
                        else {
                            tailLabelCandidates.push_front(successorLabel);
                        }
                    } else {
                        if (successorLabel->c[1] <= costVector[1] && successorLabel->c[2] <= costVector[2]) {
                            continue;
                        }
                        expanded = true;
                        const Arc &revArc = this->G.incomingArcs(successorNode)[a.revArcIndex];
                        auto &tailLabelCandidates{nextCandidateLabels[revArc.id]};
                        Label* l = labelsPool->newItem();
                        l->update(costVector, successorNode, a.revArcIndex, predPathIndex, 0);
                        tailLabelCandidates.push_back(l);
                    }
                } else {
                    if (dominated(targetFront, costVector) || dominated(truncatedFront[successorNode], costVector)) {
                        continue;
                    }
                    expanded = true;
                    Label* successorLabel = labelsPool->newItem();
                    successorLabel->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols);
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
    const std::vector<CostArray>& potential;
    Permanents permanentLabels;
    size_t sols{0};
    SolutionsList solutions;
};

#endif

//
// Created by pedro on 23.09.21.
//

#ifndef NAMOA_LAZY_H
#define NAMOA_LAZY_H

#include "../../datastructures/includes/BinaryHeapMosp.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/MemoryPool.h"

#include "Permanents.h"
#include "SolutionsList.h"
#include "Solution.h"

/**
 * See https://arxiv.org/abs/2110.10978 for details about this algorithm.
 */
class NAMOA_LAZY  {
    static bool truncatedInsertionLazy(TruncatedFront& front, const CostArray& c) {
        TruncatedCosts tc = truncate(c);
        if (front.empty()) {
            front.push_back(tc);
            return true;
        }
        auto it = front.begin();
        assert(c[1] == tc[0]);
        //while (it != front.end() && (*it)[0] <= c[1]) {
        while (it != front.end()) {
            if (tc_dominates(*it, tc)) {
                return false;
            }
            else if (lexSmaller(*it, tc)) {
                ++it;
            }
            else {
                break;
            }
        }
        it  = front.insert(it, tc);
        ++it;
        while (it != front.end()) {
            if (tc_dominates(tc, *it)) {
                it = front.erase(it);
            }
            else {
                if(DIM > 3) {
                    ++it;
                }
                else {
                    break;
                }
            }
        }
        return true;
    }

public:
    NAMOA_LAZY(const Graph& G, const std::vector<CostArray>& potential) :
            G{G},
            truncatedFront(G.nodesCount),
            potential(potential),
            permanentLabels() {}

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        Node target = G.target;
        auto start_time = std::chrono::high_resolution_clock::now();

        Pool<Label>* labelsPool = new Pool<Label>();

        Label* startLabel = labelsPool->newItem();
        startLabel->update(potential[G.source], G.source, INVALID_ARC, MAX_PATH, 0, 0);

        BinaryHeapMosp heap = BinaryHeapMosp(1);
        heap.push(startLabel);
        CostArray source_n_costs{generate()};
        TruncatedFront& targetFront{this->truncatedFront[this->G.target]};
        while (heap.size()) {
            //printf("%lu\n", heap.size());
            Label* minLabel = heap.pop();
            const Node n{minLabel->n};
            ++extractions;
//            printf("Extracting %u %u %u %u\n", n, minLabel.c[0], minLabel.c[1], minLabel.c[2]);
            TruncatedFront& currentFront{this->truncatedFront[n]};
            bool inserted = truncatedInsertionLazy(currentFront, minLabel->c);

            //assert(dominatedAtFront(targetNodeInfo.closed, minLabel.c) == targetNodeInfo.truncatedDominance(minLabel.c));
            //if (!inserted || (n != target && this->sols != minLabel->knownTargetElements && truncatedDominance(targetFront, minLabel->c))) {
            if (!inserted) {//|| (n != target && this->sols != minLabel->knownTargetElements && truncatedDominance(targetFront, minLabel->c))) {
//                printf("\t\t\tDominated! %u %u %u %u\n", n, minLabel.c[0], minLabel.c[1], minLabel.c[2]);
//                inserted ? printf("\t\t\t\tIt was dominated at target!\n") : printf("\t\t\t\tIt was dominated at node!\n");
                labelsPool->free(minLabel);
                continue;
            }
            source_n_costs = substract(minLabel->c, potential[n]);
//            printf("Extract %u: %u %u %u\n", n, source_n_costs[0], source_n_costs[1], source_n_costs[2]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, minLabel.c[0], minLabel.c[1], minLabel.c[2]);

            //printf("%lu %u %u %u\n", extractions, minLabel.n, source_n_costs, minLabel.c2);
//            printf("%lu %u %u %u %u | %u %u %u\n", iterations, minLabel.n, minLabel.c[0], minLabel.c[1], minLabel.c[2], source_n_costs[0], source_n_costs[1], source_n_costs[2]);
            ++iterations;
            if (n == target) {
//                printf("Solution %u %u %u %u with soc %u\n", minLabel->c[0], minLabel->c[1], minLabel->c[2], minLabel->c[3], sumOfCosts);
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

                if (truncatedDominance(targetFront, costVector)) {
                    continue;
                }
                if (truncatedDominance(truncatedFront[successorNode], costVector)) {
                    continue;
                }

                expanded = true;
                //Label *successorLabel{&heapLabels[successorNode]};
                Label* newLabel = labelsPool->newItem();
                newLabel->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols, 0);
                heap.push(newLabel);
            }
            if (expanded) {
                permanentLabels.addElement(minLabel->pathId, minLabel->predArcId);
            }
            labelsPool->free(minLabel);
        }
        //printSolutions();
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_bda = end_time - start_time;
        solutionData.permanents = this->permanentLabels.size();
        solutionData.duration = duration_bda.count();
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.solutions.size();
        solutionData.memoryConsumption = labelsPool->size()*sizeof(Label);
        solutionData.maxHeapSize = heap.maxSize();
        delete labelsPool;
    }

private:
    const Graph& G;
    std::vector<TruncatedFront> truncatedFront;
    const std::vector<CostArray>& potential;
    Permanents permanentLabels;
    size_t sols{0};
    SolutionsList solutions;
};

#endif

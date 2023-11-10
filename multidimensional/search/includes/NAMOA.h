//
// Created by pedro on 23.09.21.
//

#ifndef NAMOA_H
#define NAMOA_H

#include "../../datastructures/includes/BinaryHeapMosp.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/MemoryPool.h"

#include "Permanents.h"
#include "SolutionsList.h"
#include "Solution.h"

/**
 * See https://arxiv.org/abs/2110.10978 for details about this algorithm.
 */
class NAMOA  {
    typedef std::list<std::pair<CostArray, size_t>> CandidateLabels;
    typedef std::list<Label*> OpenCosts;
public:
    NAMOA(const Graph& G, const std::vector<CostArray>& potential) :
            G{G},
            exploredPaths(G.nodesCount),
            truncatedFront(G.nodesCount),
            potential(potential),
            permanentLabels() {}

    void run(Solution& solutionData) {
        size_t extractions{0};
        size_t iterations{0};
        Node target = G.target;
        auto start_time = std::chrono::high_resolution_clock::now();

        Pool<Label>* labelsPool = new Pool<Label>();

        std::unordered_map<Node, Label*> heapLabels;
        Label* startLabel = labelsPool->newItem();
        startLabel->update(potential[G.source], G.source, INVALID_ARC, MAX_PATH,0);

        BinaryHeapMosp heap = BinaryHeapMosp(1);
        heap.push(startLabel);
        heapLabels[G.source] = startLabel;
        this->exploredPaths[G.source].push_back(startLabel);
        CostArray source_n_costs{MAX_COST, MAX_COST, MAX_COST};
        TruncatedFront& targetFront{this->truncatedFront[this->G.target]};
        while (heap.size()) {
            //printf("%lu\n", heap.size());
            Label* minLabel = heap.pop();
            const Node n{minLabel->n};
            ++extractions;
            //printf("Extracting %u %u %u %u\n", n, minLabel->c[0], minLabel->c[1], minLabel->c[2]);
            TruncatedFront& currentFront{this->truncatedFront[n]};
            truncatedInsertion(currentFront, minLabel->c);

            //labelsPool->free(*minNodeInfo.openCosts.begin());
            OpenCosts& currentOpenCosts = this->exploredPaths[n];
            currentOpenCosts.pop_front();
            bool success = false;
            if (!currentOpenCosts.empty()) {
                Label* newHeapLabel{currentOpenCosts.front()};
                while (!currentOpenCosts.empty()) {
                    if (truncatedDominance(targetFront, newHeapLabel->c)) {
                        currentOpenCosts.pop_front();
                        labelsPool->free(newHeapLabel);
                        if (!currentOpenCosts.empty()) {
                            newHeapLabel = currentOpenCosts.front();
                        }
                    }
                    else {
                        success = true;
                        heapLabels[n] = newHeapLabel;
                        heap.push(newHeapLabel);
                        break;
                    }
                }
            }
            if (!success) {
                heapLabels.erase(n);
            }
            source_n_costs = substract(minLabel->c, potential[n]);
//            printf("Extract %u: %u %u %u\n", n, source_n_costs[0], source_n_costs[1], source_n_costs[2]);
            //printf("%lu;%u;%u;%u;%u\n", extractions-1, n, minLabel.c[0], minLabel.c[1], minLabel.c[2]);

            //printf("%lu %u %u %u\n", extractions, minLabel.n, source_n_costs, minLabel.c2);
//            printf("%lu %u %u %u %u | %u %u %u\n", iterations, minLabel.n, minLabel.c[0], minLabel.c[1], minLabel.c[2], source_n_costs[0], source_n_costs[1], source_n_costs[2]);
            ++iterations;
            if (n == target) {
                //solutions.addSolution(minLabel);
                //printf("Solution with costs %u %u %u. Node %u.\n", minLabel.c[0], minLabel.c[1], minLabel.c[2], n);
//                printf("Solution %u %u %u\n", minLabel.c[0], minLabel.c[1], minLabel.c[2]);
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
                if (truncatedDominance(targetFront, costVector) ||
                    truncatedDominance(truncatedFront[successorNode], costVector)) {
                    continue;
                }

                expanded = true;
                OpenCosts& successorOpenCosts = this->exploredPaths[successorNode];
                //Label* successorLabel{&heapLabels[successorNode]};
                Label* newLabel = labelsPool->newItem();
                newLabel->update(costVector, successorNode, a.revArcIndex, predPathIndex, this->sols);
                if (successorOpenCosts.empty()) {
                    successorOpenCosts.push_back(newLabel);
                    heap.push(newLabel);
                    heapLabels[successorNode] = newLabel;
                    assert(newLabel != labelsPool->firstFreeSpace);
                }
                else {
                    bool newLexMin = merge(labelsPool, successorOpenCosts, newLabel);
                    if (newLexMin) {
                        Label* oldHeapLabel = heapLabels[successorNode];
                        heapLabels[successorNode] = newLabel;
                        heap.decreaseKey(oldHeapLabel, newLabel);
                    }
                }
            }
            if (expanded) {
                permanentLabels.addElement(minLabel->pathId, minLabel->predArcId);
            }
            labelsPool->free(minLabel);
        }
        //printSolutions();
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_time = end_time - start_time;
        solutionData.duration = duration_time.count();
        solutionData.iterations = iterations;
        solutionData.extractions = extractions;
        solutionData.solutionsCt = solutions.solutions.size();
        solutionData.memoryConsumption = labelsPool->size()*sizeof(Label);
        solutionData.maxHeapSize = heap.maxSize();
        delete labelsPool;
    }

private:
    const Graph& G;
    std::vector<OpenCosts> exploredPaths;
    std::vector<TruncatedFront> truncatedFront;
    const std::vector<CostArray>& potential;
    Permanents permanentLabels;
    size_t sols{0};
    SolutionsList solutions;

    inline static bool merge(Pool<Label>* pool, OpenCosts& open, Label* newLabel) {
        auto it = open.begin();
        assert (!open.empty());
        while (it != open.end() && lexSmaller((*it)->c, newLabel->c)) {
            assert((*it)->n == newLabel->n);
            if (dominates((*it)->c, newLabel->c)) {
                pool->free(newLabel);
                return false;
            }
            ++it;
        }
        it  = open.insert(it, newLabel);
        bool insertedAtBeginning = it == open.begin();
        ++it;
        while (it != open.end()) {
            assert((*it)->n == newLabel->n);
            if (dominates(newLabel->c, (*it)->c)) {
                pool->free(*it);
                it = open.erase(it);
            }
            else {
                ++it;
            }
        }
        return insertedAtBeginning;
    }
};

#endif

#include <cassert>
#include <algorithm>
#include <cstdio>

#include "../../graph/includes/graph.h"
#include "../../datastructures/includes/Label.h"

#include "../includes/Permanents.h"
#include "../includes/SolutionsList.h"

void SolutionsList::printSolutions(const Graph& G, const Permanents& permanents) const {
    for (const Label* solution : this->solutions) {
        std::vector<std::pair<Node, CostArray>> path;
        //From this node until the target, the path follows the preprocessing path.
        uint16_t pathId = solution->pathId;
        ArcId lastArcId = solution->predArcId;
        CostArray costs = solution->c;
        Node n = solution->n;

        //printf("Node: %u with costs %u %u\n", solution->n, costsAndPotential[0], costsAndPotential[1], costsAndPotential[2]);

        while (true) {
            path.emplace_back(n, costs);
            if (pathId == MAX_PATH) break;
            const Arc& predArc = G.incomingArcs(n)[lastArcId];
            costs = substract(costs, predArc.c);
            //addInPlace(costs, potential[n]);
            n = predArc.n;
            lastArcId = permanents.getElement(pathId).second;
            pathId = permanents.getElement(pathId).first;
        }
        std::reverse(path.begin(), path.end());
        for (const auto& n : path) {
            printf("%u (%u,%u,%u)\n", n.first, n.second[0], n.second[1], n.second[2]);
        }
        printf("\n\n");
    }
}
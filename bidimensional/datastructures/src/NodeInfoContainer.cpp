//
// Created by bzfmaris on 18.01.22.
//

#include "../../graph/includes/graph.h"
#include "../includes/NodeInfoContainer.h"

NodeInfoContainer::NodeInfoContainer(const Graph& G, bool forward):
        nodeInfos(G.nodesCount),
        G{G},
        startNode{forward ? G.source : G.target},
        targetNode{forward ? G.target : G.source},
        forward{forward} {
    this->initialize();
}

void NodeInfoContainer::initialize() {
    for (Node n = 0; n < this->G.nodesCount; ++n) {
        this->nodeInfos[n].n = n;
    }
}
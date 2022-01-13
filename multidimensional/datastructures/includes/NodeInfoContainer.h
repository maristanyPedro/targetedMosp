#ifndef NODECONTAINER_H
#define NODECONTAINER_H

#include <vector>
#include <NodeInfo.h>
#include <../../graph/includes/graph.h>

/**
 * Wraps a vector of NodeInfos. For each node in the graph, the vector has one entry. The entry containing the NodeInfo
 * for node with id n is in the vector position n.
 */
struct NodeInfoContainer {

    explicit NodeInfoContainer(const Graph& G):
        nodeInfos(G.nodesCount) {
        this->initialize();
    }

    inline NodeInfo& getInfo(const Node n) {
        return this->nodeInfos[n];
    }

    std::vector<NodeInfo> nodeInfos;

private:
    void initialize() {
        for (Node n = 0; n < this->nodeInfos.size(); ++n) {
            this->nodeInfos[n].n = n;
        }
    }
};

#endif
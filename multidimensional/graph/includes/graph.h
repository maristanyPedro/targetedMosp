#ifndef GRAPH_H_
#define GRAPH_H_

#include <memory> //for unique_ptr
#include <set>
#include <list>
#include <vector>
#include <string>

#include "../../datastructures/includes/typedefs.h"

struct Arc {
    Arc() = default;
    Arc(const Node targetId, const CostArray& c, const ArcId id);

//    void operator=(const Arc& other) {
//        this->n = other.n;
//        this->c = other.c;
//
//        this->revArcIndex = other.revArcIndex;
//        this->id = other.id;
//    }

    void print() const;

    Node n{INVALID_NODE};

    CostArray c{generate()};

    NeighborhoodSize revArcIndex{MAX_DEGREE};

    ArcId id{INVALID_ARC};
};



typedef std::vector<Arc> Neighborhood;

class NodeAdjacency {
    public:
        NodeAdjacency();
        NodeAdjacency(const Node nid, std::size_t incomingArcsCt, std::size_t outgoingArcsCt);
    //private:
        Neighborhood incomingArcs;
        Neighborhood outgoingArcs;
        Node id;
};

class Graph {
    public:
        Graph(Node nodesCount, ArcId arcsCount, Node source, Node target);

        void exportGraph() const;

        inline const Neighborhood& outgoingArcs(const Node nodeId) const {
            return this->nodes[nodeId].outgoingArcs;
        }

        inline const Neighborhood& incomingArcs(const Node nodeId) const {
            return this->nodes[nodeId].incomingArcs;
        }

        inline Neighborhood& incomingArcs(const Node nodeId) {
            return this->nodes[nodeId].incomingArcs;
        }

        inline Neighborhood& outgoingArcs(const Node nodeId) {
            return this->nodes[nodeId].outgoingArcs;
        }

        const NodeAdjacency& node(const Node nodeId) const;
        NodeAdjacency& node(const Node nodeId);

        void printNodeInfo(const Node nodeId) const;
        static void printArcs(const Neighborhood & arcs) ;

        void setNodeInfo(Node n, NeighborhoodSize inDegree, NeighborhoodSize outDegree);

        inline void setName(const std::string& n) {
            this->name = n;
        }

    const Node nodesCount;
    const ArcId arcsCount;
    const Node source;
    const Node target;

    std::string name;
    bool costComponentAdded{false};

    private: //Members
        std::vector<NodeAdjacency> nodes;
};

inline const NodeAdjacency& Graph::node(const Node nodeId) const {
    return this->nodes[nodeId];
}

inline NodeAdjacency& Graph::node(const Node nodeId) {
    return this->nodes[nodeId];
}

std::unique_ptr<Graph> setupGraph(const std::string& filename, Node sourceId, Node targetId);

void split(const std::string& s, char delim, std::vector<std::string>& elems);

std::vector<std::string> split(const std::string& s, char delim);

#endif

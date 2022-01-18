#include <cassert>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>

#include "../includes/graph.h"

using namespace std;

unique_ptr<Graph> setupGraph(const string& filename, Node sourceId, Node targetId) {
    assert (sourceId != INVALID_NODE && targetId != INVALID_NODE);
    ifstream infile(filename);
    string line;
    size_t nodesCount =0, arcsCount = 0;
    //Parse file until information about number of nodes and number of arcs is reached.
    while (getline(infile, line)) {
        vector<string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "p") {
            nodesCount = stoi(splittedLine[2]);
            arcsCount = stoi(splittedLine[3]);
            break;
        }
    }
    if (nodesCount == 0 || arcsCount == 0) {
        printf("Could not determine the size of the graph %s. Abort.\n", filename.c_str());
        exit(1);
    }
    std::vector<NeighborhoodSize> inDegree(nodesCount, 0);
    std::vector<NeighborhoodSize> outDegree(nodesCount, 0);
    size_t addedArcs = 0;
    std::vector<std::pair<Node,Arc>> arcs(arcsCount);
    while (getline(infile, line)) {
        vector<string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "a") {
            assert(splittedLine.size() >= 5);
            Node tailId;
            std::stringstream(splittedLine[1]) >> tailId;
            Node headId;
            std::stringstream(splittedLine[2]) >> headId;
            ++outDegree[tailId];
            assert(outDegree[tailId] != MAX_DEGREE);
            ++inDegree[headId];
            assert(inDegree[headId] != MAX_DEGREE);
            arcs[addedArcs] = make_pair(tailId, Arc(headId, stoi(splittedLine[3]), stoi(splittedLine[4])));
            ++addedArcs;
        }
    }
    assert(addedArcs == arcsCount);
    unique_ptr<Graph> G = make_unique<Graph>(nodesCount, arcsCount, sourceId, targetId);
    for (Node i = 0; i < nodesCount; ++i) {
        G->setNodeInfo(i, inDegree[i], outDegree[i]);
    }
    vector<NeighborhoodSize> incomingArcsPerNode(nodesCount, 0);
    vector<NeighborhoodSize> outgoingArcsPerNode(nodesCount, 0);
    for (auto& arc: arcs) {
        NodeAdjacency& tail = G->node(arc.first);
        NodeAdjacency& head = G->node(arc.second.n);
        tail.id = arc.first;
        head.id = arc.second.n;
        //The arc currently represents an arc as an outgoing arc from the arc's tail.s
        arc.second.revArcIndex = incomingArcsPerNode[head.id];
        tail.outgoingArcs[outgoingArcsPerNode[tail.id]++] = arc.second;
        //Update arc-entries s.t. the updated object represents the same arc but as an incoming arc to the arc's head.
        arc.second.n = tail.id;
        arc.second.revArcIndex = outgoingArcsPerNode[tail.id] - 1;
        head.incomingArcs[incomingArcsPerNode[head.id]++] = arc.second;

    }
    for (Node i = 0; i < nodesCount; ++i) {
        assert(inDegree[i] == incomingArcsPerNode[i]);
        assert(outDegree[i] == outgoingArcsPerNode[i]);
    }
    return G;
}

void Graph::setNodeInfo(Node n, NeighborhoodSize inDegree, NeighborhoodSize outDegree) {
    NodeAdjacency& currentNode = this->nodes[n];
    currentNode.id = n;
    currentNode.incomingArcs.resize(inDegree);
    currentNode.outgoingArcs.resize(outDegree);
}

void Graph::printNodeInfo(const Node nodeId) const {
    printf("Analyzing node: %u\n", nodeId);
    printf("OUTGOING ARCS\n");
    printArcs(this->outgoingArcs(nodeId));
    printf("INCOMING ARCS\n");
    printArcs(this->incomingArcs(nodeId));
}

void Graph::printArcs(const Neighborhood & arcs) const {
    for (const Arc& arc : arcs) {
        arc.print();
    }
}

Arc::Arc(const Node targetId, const CostType c1, const CostType c2):
        n{targetId}, c{c1, c2}{}

void Arc::print() const {
    printf("Arc costs: (%d, %d)\n", c[0], c[1]);
}

Graph::Graph(Node nodesCount, ArcId arcsCount, Node source, Node target):
    nodesCount{nodesCount},
    arcsCount{arcsCount},
    source{source},
    target{target},
    nodes(nodesCount) {
    //We need that many entries in our ncl-vectors in BDA.
    //In case this assertion fails, just change the ArcId typedef in typedefs.h
    assert(INVALID_ARC >= 2*arcsCount);
}

void split(const string& s, char delim, vector<string>& elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

vector<string> split(const string& s, char delim) {
    vector<string> elems;
    if (delim == ' ') {
        istringstream iss(s);
        elems = {istream_iterator<string>{iss}, istream_iterator<string>{}};
    } else {
        split(s, delim, elems);
    }

    for (auto& elem : elems) {
        if (elem.back() == '\n') {
            elem = elem.substr(0, elem.size() - 1);
        }
    }
    return elems;
}

NodeAdjacency::NodeAdjacency(): id{INVALID_NODE} {}

NodeAdjacency::NodeAdjacency(const Node nid, size_t incomingArcsCt, size_t outgoingArcsCt):
    incomingArcs(incomingArcsCt),
    outgoingArcs(outgoingArcsCt),
    id{nid} {

    incomingArcs.shrink_to_fit();
    outgoingArcs.shrink_to_fit();
}

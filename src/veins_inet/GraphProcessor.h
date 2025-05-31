//
// Created by l4zy9uy on 22/05/2025.
//

#ifndef OMNET_GRAPHPROCESSOR_H
#define OMNET_GRAPHPROCESSOR_H
#include "XMLProcessor.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <utility>

struct Path {
    std::vector<std::string> junctions;
    std::vector<std::string> edges;
    double length = std::numeric_limits<double>::infinity();
};

class GraphProcessor {
public:
    // ctor: takes a parsed XMLProcessorBoost instance
    explicit GraphProcessor(const XMLProcessorBoost& xmlProc);

    // Returns the shortest path as an ordered list of junction IDs.
    // Empty vector if no path exists.
    std::vector<std::string>
    getShortestPathNodes(const std::string& sourceId,
                         const std::string& targetId) const;

    // Returns the total length of that path (sum of edge lengths).
    // +inf if no path exists.
    double
    getShortestPathLength(const std::string& sourceId,
                          const std::string& targetId) const;

    std::vector<Path>
    getKShortestPaths(const std::string& sourceId,
                      const std::string& targetId,
                      int K = 3);
                      
    double dijkstra(const std::string& src, const std::string& dst) const;
    std::vector<std::string> getAllJunctions() const;

private:
    const XMLProcessorBoost& xml;
    // adjacency: junction ID -> list of (neighbor junction, edge cost)
    std::unordered_map<std::string,
        std::vector<std::pair<std::string,double>>> adj;

    // Build adj from xml.getAllRoads(), getAttributes(edge)["from"/"to"], getEdgeLength()
    void buildGraph();

    Path aStar(const std::string& sourceId,
                            const std::string& targetId,
                            const decltype(adj)& adjMap) const;
};

#endif //OMNET_GRAPHPROCESSOR_H

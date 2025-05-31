//
// Created by l4zy9uy on 22/05/2025.
//

#include "GraphProcessor.h"
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>

// For priority‐queue ordering by f = g + h (min-heap):
struct PQNode {
    std::string id;
    double       f;
    bool operator<(PQNode const &o) const {
        return f > o.f;
    }
};

GraphProcessor::GraphProcessor(const XMLProcessorBoost& xmlProc)
  : xml(xmlProc)
{
    buildGraph();
}

void GraphProcessor::buildGraph() {
    // 1) collect all junction IDs from every edge’s “from”/“to”
    std::unordered_set<std::string> junctions;
    for (auto const &edgeId : xml.getAllRoads()) {
        auto attrs = xml.getAttributes(edgeId);
        auto itF = attrs.find("from");
        auto itT = attrs.find("to");
        if (itF != attrs.end() && itT != attrs.end()) {
            junctions.insert(itF->second);
            junctions.insert(itT->second);
        }
    }
    // 2) for each junction, get its outgoing edges, then build adj list
    for (auto const &j : junctions) {
        for (auto const &edgeId : xml.getEdgesFromJunction(j)) {
            auto attrs = xml.getAttributes(edgeId);
            auto itT = attrs.find("to");
            if (itT == attrs.end()) continue;
            std::string nbr = itT->second;
            double cost = xml.getEdgeLength(edgeId);
            adj[j].push_back({nbr, cost});
        }
    }
}

double hypot_dist(const std::pair<double,double>& a, const std::pair<double,double>& b) {
    return std::hypot(a.first - b.first, a.second - b.second);
}

Path GraphProcessor::aStar(const std::string& sourceId,
                            const std::string& targetId,
                            const decltype(adj)& adjMap) const
{
    Path result;
    if (sourceId == targetId) {
        result.junctions = { sourceId };
        result.length = 0.0;
        return result;
    }

    std::priority_queue<PQNode> open;
    std::unordered_map<std::string, double> gScore;
    std::unordered_map<std::string, std::string> cameFromNode;
    std::unordered_set<std::string> closed;

    auto tgtCoord = xml.getJunctionCoord(targetId);
    auto srcCoord = xml.getJunctionCoord(sourceId);
    double h0 = hypot_dist(srcCoord, tgtCoord);
    gScore[sourceId] = 0.0;
    open.push({sourceId, h0});

    while (!open.empty()) {
        auto cur = open.top(); open.pop();
        const auto u = cur.id;
        if (u == targetId) break;
        if (closed.count(u)) continue;
        closed.insert(u);

        auto itAdj = adjMap.find(u);
        if (itAdj == adjMap.end()) continue;

        for (auto const& [v, cost] : itAdj->second) {
            double tentativeG = gScore[u] + cost;
            if (!gScore.count(v) || tentativeG < gScore[v]) {
                gScore[v] = tentativeG;
                cameFromNode[v] = u;
                auto vCoord = xml.getJunctionCoord(v);
                double h = hypot_dist(vCoord, tgtCoord);
                open.push({v, tentativeG + h});
            }
        }
    }
    if (!cameFromNode.count(targetId))
        return result; // no path

    // reconstruct
    std::vector<std::string> revJ;
    for (std::string cur = targetId; ; cur = cameFromNode[cur]) {
        revJ.push_back(cur);
        if (cur == sourceId) break;
    }
    std::reverse(revJ.begin(), revJ.end());

    result.junctions = revJ;
    result.length = gScore[targetId];
    return result;
}

std::vector<std::string>
GraphProcessor::getShortestPathNodes(const std::string& sourceId,
                                     const std::string& targetId) const
{
    return aStar(sourceId, targetId, adj).junctions;
}

double
GraphProcessor::getShortestPathLength(const std::string& sourceId,
                                      const std::string& targetId) const
{
    return aStar(sourceId, targetId, adj).length;
}

std::vector<Path>
GraphProcessor::getKShortestPaths(const std::string& sourceId,
                                  const std::string& targetId,
                                  int K)
{
    std::vector<Path> shortestPaths;
    if (K < 1) return shortestPaths;

    // firstPath shortest
    Path firstPath = aStar(sourceId, targetId, adj);
    if (firstPath.junctions.empty()) return shortestPaths;
    // build edges for firstPath
    for (size_t i = 1; i < firstPath.junctions.size(); ++i) {
        auto fromJunc = firstPath.junctions[i - 1];
        auto toJunc = firstPath.junctions[i];
        auto edgeId = xml.findEdgeBetweenJunctions(fromJunc, toJunc);
        firstPath.edges.push_back(edgeId);
    }
    // correct length by summing edges
    firstPath.length = 0.0;
    for (auto const& edgeId : firstPath.edges) {
        firstPath.length += xml.getEdgeLength(edgeId);
    }

    shortestPaths.push_back(firstPath);

    auto cmp = [](Path const& a, Path const& b){ return a.length > b.length; };
    std::priority_queue<Path, std::vector<Path>, decltype(cmp)> candidates(cmp);

    for (int pathNum = 1; pathNum < K; ++pathNum) {
        const Path& prev = shortestPaths[pathNum - 1];
        for (size_t i = 0; i + 1 < prev.junctions.size(); ++i) {
            std::vector<std::string> rootJ(prev.junctions.begin(), prev.junctions.begin()+i+1);
            auto tempAdj = adj;
            for (auto const& P : shortestPaths) {
                if (P.junctions.size() > i && std::equal(rootJ.begin(), rootJ.end(), P.junctions.begin())) {
                    auto u = P.junctions[i];
                    auto v = P.junctions[i+1];
                    auto& neighbors = tempAdj[u];
                    neighbors.erase(std::remove_if(neighbors.begin(), neighbors.end(),
                                                   [&](auto const& pr){ return pr.first == v; }), neighbors.end());
                }
            }
            for (size_t j = 0; j + 1 < rootJ.size(); ++j) {
                auto rem = rootJ[j];
                tempAdj.erase(rem);
                for (auto &kv : tempAdj) {
                    auto &vec = kv.second;
                    vec.erase(std::remove_if(vec.begin(), vec.end(),
                              [&](auto const& pr){ return pr.first == rem; }), vec.end());
                }
            }
            auto spurNode = rootJ.back();
            Path spur = aStar(spurNode, targetId, tempAdj);
            if (!spur.junctions.empty()) {
                Path tot;
                tot.junctions = rootJ;
                tot.junctions.insert(tot.junctions.end(), spur.junctions.begin()+1, spur.junctions.end());
                // rebuild edges
                for (size_t m = 1; m < tot.junctions.size(); ++m) {
                    auto u = tot.junctions[m-1];
                    auto v = tot.junctions[m];
                    auto edgeId = xml.findEdgeBetweenJunctions(u, v);
                    tot.edges.push_back(edgeId);
                }
                // correct length by summing edges
                tot.length = 0.0;
                for (auto const& eid : tot.edges) {
                    tot.length += xml.getEdgeLength(eid);
                }
                candidates.push(tot);
            }
        }
        if (candidates.empty()) break;
        shortestPaths.push_back(candidates.top());
        candidates.pop();
    }
    return shortestPaths;
}

double GraphProcessor::dijkstra(const std::string& src,
                                const std::string& dst) const
{
    // distances map
    std::unordered_map<std::string, double> dist;
    for (auto const& kv : adj)
        dist[kv.first] = std::numeric_limits<double>::infinity();

    // min‐heap of (distance, junctionID)
    using Item = std::pair<double, std::string>;
    auto cmp = [](Item const &a, Item const &b){
        return a.first > b.first;
    };
    std::priority_queue<Item, std::vector<Item>, decltype(cmp)> pq(cmp);

    // start
    if (dist.find(src) == dist.end())
        return -1.0;
    dist[src] = 0.0;
    pq.emplace(0.0, src);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u])
            continue;            // stale entry

        if (u == dst)
            return d;            // early exit!

        // relax edges
        for (auto const& [v, w] : adj.at(u)) {
            double nd = d + w;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.emplace(nd, v);
            }
        }
    }

    // unreachable
    return -1.0;
}

std::vector<std::string> GraphProcessor::getAllJunctions() const {
    std::vector<std::string> out;
    out.reserve(adj.size());
    for (auto const& kv : adj) {
        out.push_back(kv.first);
    }
    return out;
}
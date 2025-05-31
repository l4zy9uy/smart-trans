// XMLProcessorBoost.cc
#include "XMLProcessor.h"
#include <stdexcept>
#include <iostream>
#include <set>
#include <boost/algorithm/string.hpp>    // for split, is_any_of
#include <cmath>
using boost::property_tree::ptree;

#include <omnetpp.h>

using namespace omnetpp;
void XMLProcessorBoost::dumpDiagnosticsOMNeT(int maxItems) const {
    int count;
    // Roads
    EV_INFO << "=== Roads ===\n";
    count = 0;
    for (auto const &r : _roads) {
        if (count++>=maxItems) break;
        EV_INFO << r << "\n";
    }
    // Edge Lengths
    EV_INFO << "=== Edge Lengths ===\n";
    count = 0;
    for (auto const &kv : _edgeLengths) {
        if (count++>=maxItems) break;
        EV_INFO << kv.first << " -> " << kv.second << "\n";
    }
    // Outgoing
    EV_INFO << "=== Outgoing ===\n";
    count = 0;
    for (auto const &kv : _outgoing) {
        if (count++>=maxItems) break;
        EV_INFO << kv.first << ": ";
        int inner=0;
        for (auto const &nbr : kv.second) {
            if (inner++>=maxItems) break;
            EV_INFO << nbr << " ";
        }
        EV_INFO << "\n";
    }
    // Similarly for incoming, attrs, junctions... (repeat pattern)
}
void XMLProcessorBoost::dumpDiagnostics(int maxItems) const {
    // 1) Roads
    {
        std::cout << "=== Roads (up to " << maxItems << ") ===\n";
        int count = 0;
        for (auto const &r : _roads) {
            if (count++ >= maxItems) break;
            std::cout << "  " << r << "\n";
        }
        std::cout << std::endl;
    }

    // 2) Edge lengths
    {
        std::cout << "=== Edge Lengths (up to " << maxItems << ") ===\n";
        int count = 0;
        for (auto const &kv : _edgeLengths) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << " -> " << kv.second << "\n";
        }
        std::cout << std::endl;
    }

    // 3) Edge-based adjacency
    {
        std::cout << "=== Outgoing (edge -> neighbors) ===\n";
        int count = 0;
        for (auto const &kv : _outgoing) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << ": ";
            int inner = 0;
            for (auto const &nbr : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << nbr << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    {
        std::cout << "=== Incoming (edge <- neighbors) ===\n";
        int count = 0;
        for (auto const &kv : _incoming) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << ": ";
            int inner = 0;
            for (auto const &nbr : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << nbr << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    // 4) Edge attributes
    {
        std::cout << "=== Edge Attributes (up to " << maxItems << ") ===\n";
        int count = 0;
        for (auto const &kv : _attrs) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << ":\n";
            int inner = 0;
            for (auto const &attr : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << "    " << attr.first << " = " << attr.second << "\n";
            }
        }
        std::cout << std::endl;
    }

    // 5) Junction → edges
    {
        std::cout << "=== Junctions Outgoing (junction -> edges) ===\n";
        int count = 0;
        for (auto const &kv : _junctionOut) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << ": ";
            int inner = 0;
            for (auto const &e : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << e << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }
    {
        std::cout << "=== Junctions Incoming (junction <- edges) ===\n";
        int count = 0;
        for (auto const &kv : _junctionIn) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first << ": ";
            int inner = 0;
            for (auto const &e : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << e << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    // 6) Junction coordinates
    {
        std::cout << "=== Junction Coordinates (up to " << maxItems << ") ===\n";
        int count = 0;
        for (auto const &kv : _junctionCoords) {
            if (count++ >= maxItems) break;
            std::cout << "  " << kv.first
                      << ": (" << kv.second.first
                      << ", " << kv.second.second << ")\n";
        }
        std::cout << std::endl;
    }

    // 7) Junction distances
    {
        std::cout << "=== Junction Distances (up to " << maxItems << ") ===\n";
        int count = 0;
        for (auto const &kv : _junctionDist) {
            if (count++ >= maxItems) break;
            std::cout << "  from " << kv.first << ":\n";
            int inner = 0;
            for (auto const &innerKv : kv.second) {
                if (inner++ >= maxItems) break;
                std::cout << "    to " << innerKv.first
                          << " = " << innerKv.second << "\n";
            }
        }
        std::cout << std::endl;
    }
}

XMLProcessorBoost::XMLProcessorBoost(const std::string& xmlFile,
                                     const std::string& rootNode)
  : _xmlFile(xmlFile), _rootNode(rootNode)
{
    read_xml(_xmlFile, _pt, boost::property_tree::xml_parser::trim_whitespace);
    parse();
}

void XMLProcessorBoost::parse() {
    parseJunctions();
    parseEdges();
    parseConnections();
}

void XMLProcessorBoost::parseJunctions() {
    for (auto& child : _pt.get_child(_rootNode)) {
        if (child.first == "junction") {
            auto attrs = child.second.get_child("<xmlattr>");
            std::string id = attrs.get("id", "");
            if (id.empty() || id.front() == ':') continue;
            double x = std::stod(attrs.get("x", "0"));
            double y = std::stod(attrs.get("y", "0"));
            _junctionCoords[id] = {x,y};
        }
    }
}

void XMLProcessorBoost::parseEdges() {
    for (auto& child : _pt.get_child(_rootNode)) {
        if (child.first != "edge") continue;
        auto& edgeNode = child.second;
        auto attrs = edgeNode.get_child("<xmlattr>");
        std::string id  = attrs.get("id", "");
        std::string src = attrs.get("from", "");
        std::string dst = attrs.get("to",   "");
        if (id.empty() || id.front()==':' || src.empty() || dst.empty()) continue;

        // record edge
        _roads.push_back(id);
        _junctionOut[src].push_back(id);
        _junctionIn[dst].push_back(id);

        // compute and store length
        double length = computeLengthFromFirstLane(edgeNode);
        if (length<=0.0) {
            if (auto shapeOpt = attrs.get_optional<std::string>("shape"))
                length = computeLengthFromShape(*shapeOpt);
        }
        _edgeLengths[id] = length;
        _junctionDist[src][dst] = length;

        // store attributes
        AttrMap am;
        for (auto& a : attrs)
            am[a.first] = a.second.get_value<std::string>();
        _attrs[id] = std::move(am);

        // init adjacency
        _outgoing[id] = {};
        _incoming[id] = {};
    }
}

void XMLProcessorBoost::parseConnections() {
    for (auto& child : _pt.get_child(_rootNode)) {
        if (child.first != "connection") continue;
        auto attrs = child.second.get_child("<xmlattr>");
        std::string fromE = attrs.get("from", "");
        std::string toE   = attrs.get("to",   "");
        if (fromE.empty()||toE.empty()||fromE.front()==':'||toE.front()==':') continue;
        _outgoing[fromE].push_back(toE);
        _incoming[toE].push_back(fromE);
    }
}

// Helpers
double XMLProcessorBoost::computeLengthFromFirstLane(const ptree& edgeNode) const {
    for (auto& sub : edgeNode) {
        if (sub.first == "lane") {
            auto la = sub.second.get_child("<xmlattr>");
            if (auto lenOpt = la.get_optional<std::string>("length"))
                return std::stod(*lenOpt);
            break;
        }
    }
    return 0.0;
}

double XMLProcessorBoost::computeLengthFromShape(const std::string& shape) const {
    std::istringstream iss(shape);
    std::vector<std::pair<double,double>> pts;
    std::string tok;
    while (iss >> tok) {
        auto c = tok.find(',');
        if (c==std::string::npos) continue;
        pts.emplace_back(
            std::stod(tok.substr(0,c)),
            std::stod(tok.substr(c+1))
        );
    }
    double sum=0; for (size_t i=1;i<pts.size();++i) {
        double dx=pts[i].first-pts[i-1].first;
        double dy=pts[i].second-pts[i-1].second;
        sum+=std::sqrt(dx*dx+dy*dy);
    }
    return sum;
}

XMLProcessorBoost::RoadList XMLProcessorBoost::getAllRoads() const { return _roads; }
XMLProcessorBoost::RoadList XMLProcessorBoost::getOutgoing(const std::string& id) const {
    auto it=_outgoing.find(id); return it==_outgoing.end()?RoadList():it->second;
}
XMLProcessorBoost::RoadList XMLProcessorBoost::getIncoming(const std::string& id) const {
    auto it=_incoming.find(id); return it==_incoming.end()?RoadList():it->second;
}
XMLProcessorBoost::AttrMap XMLProcessorBoost::getAttributes(const std::string& id) const {
    auto it=_attrs.find(id); return it==_attrs.end()?AttrMap():it->second;
}
double XMLProcessorBoost::getEdgeLength(const std::string& id) const {
    auto it=_edgeLengths.find(id);
    return it==_edgeLengths.end()?0.0:it->second;
}
XMLProcessorBoost::RoadList XMLProcessorBoost::getEdgesFromJunction(const std::string& j) const {
    auto it=_junctionOut.find(j); return it==_junctionOut.end()?RoadList():it->second;
}
XMLProcessorBoost::RoadList XMLProcessorBoost::getEdgesToJunction(const std::string& j) const {
    auto it=_junctionIn.find(j); return it==_junctionIn.end()?RoadList():it->second;
}
std::pair<double,double> XMLProcessorBoost::getJunctionCoord(const std::string& j) const {
    auto it=_junctionCoords.find(j);
    return it==_junctionCoords.end()?std::pair<double,double>{0,0}:it->second;
}
double XMLProcessorBoost::getJunctionDistance(const std::string& j1,const std::string& j2) const {
    auto it=_junctionDist.find(j1);
    if(it!=_junctionDist.end()){
        auto it2=it->second.find(j2);
        if(it2!=it->second.end()) return it2->second;
    }
    auto c1=getJunctionCoord(j1), c2=getJunctionCoord(j2);
    double dx=c1.first-c2.first, dy=c1.second-c2.second;
    return std::sqrt(dx*dx+dy*dy);
}

std::string XMLProcessorBoost::findEdgeBetweenJunctions(const std::string& fromJunction, const std::string& toJunction) const {
    // Get all edges originating from the source junction
    auto outgoingEdges = getEdgesFromJunction(fromJunction);
    
    // Check each outgoing edge to see if it connects to the target junction
    for (const auto& edgeId : outgoingEdges) {
        auto attrs = getAttributes(edgeId);
        auto it_to = attrs.find("to");
        if (it_to != attrs.end() && it_to->second == toJunction) {
            return edgeId;
        }
    }
    
    // No direct edge found
    return "";
}

//void XMLProcessorBoost::dumpDiagnosticsOMNeT(int maxItems) const {
//    int count;
//    // Roads
//    EV_INFO << "=== Roads ===\n";
//    count = 0;
//    for (auto const &r : _roads) {
//        if (count++>=maxItems) break;
//        EV_INFO << r << "\n";
//    }
//    // Edge Lengths
//    EV_INFO << "=== Edge Lengths ===\n";
//    count = 0;
//    for (auto const &kv : _edgeLengths) {
//        if (count++>=maxItems) break;
//        EV_INFO << kv.first << " -> " << kv.second << "\n";
//    }
//    // Outgoing
//    EV_INFO << "=== Outgoing ===\n";
//    count = 0;
//    for (auto const &kv : _outgoing) {
//        if (count++>=maxItems) break;
//        EV_INFO << kv.first << ": ";
//        int inner=0;
//        for (auto const &nbr : kv.second) {
//            if (inner++>=maxItems) break;
//            EV_INFO << nbr << " ";
//        }
//        EV_INFO << "\n";
//    }
//    // Similarly for incoming, attrs, junctions... (repeat pattern)
//}

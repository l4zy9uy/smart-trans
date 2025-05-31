// XMLProcessorBoost.h
#ifndef XMLPROCESSORBOOST_H_
#define XMLPROCESSORBOOST_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <vector>
#include <map>

class XMLProcessorBoost {
public:
    using RoadList = std::vector<std::string>;
    using AttrMap  = std::map<std::string,std::string>;

    XMLProcessorBoost(const std::string& xmlFile,
                      const std::string& rootNode = "net");
    ~XMLProcessorBoost() = default;

    // Edge-based getters
    RoadList getAllRoads() const;
    RoadList getOutgoing(const std::string& roadId) const;
    RoadList getIncoming(const std::string& roadId) const;
    AttrMap  getAttributes(const std::string& roadId) const;
    double   getEdgeLength(const std::string& roadId) const;

    // Junction-based getters
    RoadList getEdgesFromJunction(const std::string& junctionId) const;
    RoadList getEdgesToJunction(const std::string& junctionId) const;
    std::pair<double,double> getJunctionCoord(const std::string& junctionId) const;
    double   getJunctionDistance(const std::string& j1, const std::string& j2) const;
    
    // Find edge connecting two junctions
    std::string findEdgeBetweenJunctions(const std::string& fromJunction, const std::string& toJunction) const;

    void dumpDiagnostics(int maxItems = 10) const;
    void dumpDiagnosticsOMNeT(int maxItems = 10) const;

private:
    void parse();
    void parseJunctions();
    void parseEdges();
    void parseConnections();
    double computeLengthFromFirstLane(const boost::property_tree::ptree& edgeNode) const;
    double computeLengthFromShape(const std::string& shape) const;

    std::string                        _xmlFile;
    std::string                        _rootNode;
    boost::property_tree::ptree        _pt;

    RoadList                           _roads;
    std::map<std::string,RoadList>     _outgoing;
    std::map<std::string,RoadList>     _incoming;
    std::map<std::string,AttrMap>      _attrs;

    std::map<std::string,RoadList>     _junctionOut;
    std::map<std::string,RoadList>     _junctionIn;

    std::map<std::string,std::pair<double,double>> _junctionCoords;
    std::map<std::string,std::map<std::string,double>> _junctionDist;
    std::map<std::string, double> _edgeLengths;

};

#endif // XMLPROCESSORBOOST_H_

// VehicleSpawner.cc
#include "VehicleSpawner.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include <map>
#include <algorithm>
#include <list>

Define_Module(VehicleSpawner);

void VehicleSpawner::initialize() {
    //BaseModule::initialize(stage);
    // Schedule an event at time 0 to insert vehicles
        xml = std::make_unique<XMLProcessorBoost>("../../simulations/veins5Final/square.net.xml", "net");
        graph_   = std::make_unique<GraphProcessor>(*xml);
        taskGen_ = std::make_unique<TaskGenerator>(*graph_);


        // ---------- TraCI is up; schedule spawn after 0.1 s ----------
        spawnTimer = new cMessage("spawnVehicles");
        scheduleAt(simTime() + 0.1, spawnTimer);
}

void VehicleSpawner::handleMessage(cMessage* msg) {
    if (!msg->isSelfMessage()) {return; }
    // Generate 5 random (src, dst) tasks
    manager_ = TraCIScenarioManagerAccess().get();
    traci_   = manager_->getCommandInterface();

    const int N = 5;
    auto sources = taskGen_->pickSourceDest(N).first;
    auto dests = taskGen_->pickSourceDest(N).second;

    for (int i = 0; i < N; ++i) {
        const std::string& src = sources[i];
        const std::string& dst = dests[i];

        // 4a) Compute shortest path
        auto paths = graph_->getKShortestPaths(src, dst, 1);
        if (paths.empty()) {
            EV_WARN << "No path from " << src << " to " << dst << '\n';
            continue;
        }
        const auto& path   = paths[0];
        const auto& edges  = path.edges;

        // 4b) Build unique IDs
        std::ostringstream oss;
        oss << "rsuVeh_" << simTime() << "_" << i;
        std::string vehId   = oss.str();
        std::string routeId = vehId + "_route";

        std::list<std::string> edgeList(edges.begin(), edges.end());
        traci_->addRoute(routeId, edgeList);

        // ------- 4d) Spawn the vehicle on that route ----------------------------
        std::string vehType = "DEFAULT_VEHTYPE";                    // or any valid SUMO type
        traci_->addVehicle(vehId, vehType, routeId, simTime().dbl());

        // 4e) Set desired speed
        double speed = 15.5;
        traci_->vehicle(vehId).setSpeed(speed);

        // 4f) Time-window as generic parameters
        double tEarliest = 75;
        double tLatest   = 90;
        if (tLatest < tEarliest) std::swap(tEarliest, tLatest);

        traci_->vehicle(vehId).setParameter("timeWindowEarliest", std::to_string(tEarliest));
       traci_->vehicle(vehId).setParameter("timeWindowLatest",   std::to_string(tLatest));
       traci_->vehicle(vehId).setParameter("srcJunction", src);
       traci_->vehicle(vehId).setParameter("dstJunction", dst);

       // --------------------------------------------------------------
       // store remaining distance to destination (in metres)
       // path.length is provided by GraphProcessor
       // --------------------------------------------------------------
       double remainingDist = path.length;
       traci_->vehicle(vehId).setParameter("remainingDist", std::to_string(remainingDist));

        EV_INFO << "Spawned " << vehId
                << " src=" << src << " dst=" << dst
                << " speed=" << speed
                << " tw=[" << tEarliest << ',' << tLatest << "]\n";
    }
    // Clean up the spawn timer
    delete msg;
    spawnTimer = nullptr;
}


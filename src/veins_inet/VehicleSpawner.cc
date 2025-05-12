// VehicleSpawner.cc
#include "VehicleSpawner.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include <map>
#include <algorithm>
#include <list>

Define_Module(VehicleSpawner);

void VehicleSpawner::initialize() {
    // Schedule an event at time 0 to insert vehicles
    std::random_device rd;
    rng.seed(rd());

    cMessage* msg = new cMessage("spawnVehicles");
    scheduleAt(simTime() + 0.1, msg);
}

void VehicleSpawner::handleMessage(cMessage* msg) {
    // Only handle the self-scheduled spawn trigger message
    if (!msg->isSelfMessage()) {
        // (Optional: call base class or handle other messages if any)
        return;
    }

    // Obtain the TraCI command interface (assumes TraCIScenarioManager is accessible)
    TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
    ASSERT(manager);
    TraCICommandInterface* traci = manager->getCommandInterface();

    // Get all road edge IDs from TraCI and filter out invalid spawn edges (e.g., internal edges)
    std::list<std::string> allEdges = traci->getRoadIds();
    std::vector<std::string> spawnEdges;
    spawnEdges.reserve(allEdges.size());
    for (const std::string& edgeId : allEdges) {
        if (!edgeId.empty() && edgeId[0] == ':') {
            continue;  // skip internal junction edges (not valid for spawning vehicles)
        }
        spawnEdges.push_back(edgeId);
    }

    // Check that we have some valid edges to spawn on
    if (spawnEdges.empty()) {
        EV_WARN << "No valid road edges available for vehicle spawning." << std::endl;
    } else {
        int vehiclesAdded = 0;
        int attempts = 0;
        const int maxVehicles = 7;
        const int maxAttempts = maxVehicles * 5;
        std::string vehicleTypeId = "DEFAULT_VEHTYPE";

        // Loop until 7 vehicles are added or 20 attempts made
        while (vehiclesAdded < maxVehicles && attempts < maxAttempts) {
            attempts++;

            // Pick a random edge from the valid list as the spawn location
            std::uniform_int_distribution<int> pickEdge(0, spawnEdges.size() - 1);
            std::string startEdge = spawnEdges[pickEdge(rng)];

            // Generate unique IDs for the new route and vehicle
            static int uniqueIdCounter = 0;
            uniqueIdCounter++;
            std::string routeId = "spawnedRoute_" + std::to_string(uniqueIdCounter);
            std::string vehicleId = "spawnedVeh_" + std::to_string(uniqueIdCounter);

            // Attempt to add a new route on the selected edge(s)
            try {
                // Here we create a route consisting of the single chosen edge.
                // (You could extend this to multiple edges if needed, ensuring they form a connected path.)
                std::list<std::string> routeEdges = { startEdge };
                traci->addRoute(routeId, routeEdges);  // may throw if route creation fails
            }
            catch (cRuntimeError& e) {
                EV_WARN << "Failed to add route " << routeId
                        << " for edge " << startEdge
                        << " (attempt " << attempts << "): " << e.what() << std::endl;
                continue;  // skip to the next attempt without trying to add vehicle
            }

            // If addRoute did not throw, we assume the route was created successfully.
            // Now attempt to add the vehicle on that route.
            try {
                bool ok = traci->addVehicle(vehicleId, vehicleTypeId, routeId);
                if (!ok) {
                    // addVehicle returned false, meaning the vehicle was not added (no exception thrown)
                    EV_WARN << "Vehicle " << vehicleId
                            << " could not be added on edge " << startEdge
                            << " (attempt " << attempts << ") – addVehicle returned false." << std::endl;
                    continue;  // try another edge
                }
            }
            catch (cRuntimeError& e) {
                EV_WARN << "Error adding vehicle " << vehicleId
                        << " on edge " << startEdge
                        << " (attempt " << attempts << "): " << e.what() << std::endl;
                // (Optionally, one could remove the previously added route here if desired)
                continue;
            }

            // If we reach here, the vehicle was added successfully
            vehiclesAdded++;
            EV_INFO << "Successfully spawned vehicle " << vehicleId
                    << " on edge " << startEdge << " (attempt " << attempts << ")." << std::endl;
        }  // end while

        if (vehiclesAdded < maxVehicles) {
            EV_WARN << "Spawner stopped after " << attempts
                    << " attempts with " << vehiclesAdded
                    << " vehicles added (target was " << maxVehicles << ")." << std::endl;
        }
    }

    // Clean up the self-message after handling to avoid memory leak
    delete msg;
}

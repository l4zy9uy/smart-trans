// VehicleSpawner.h
#ifndef VEHICLESPAWNER_H_
#define VEHICLESPAWNER_H_
#include <omnetpp.h>
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include <vector>
#include <string>
#include <random>

using namespace omnetpp;
using namespace veins;

class VehicleSpawner : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
private:
    std::vector<std::string> validEdges;
        // single RNG for the module
    std::mt19937 rng;
};

#endif

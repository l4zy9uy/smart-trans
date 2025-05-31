// VehicleSpawner.h
#ifndef VEHICLESPAWNER_H_
#define VEHICLESPAWNER_H_
#include <omnetpp.h>
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include <vector>
#include <string>
#include <random>
#include "XMLProcessor.h"
#include <memory>
#include "GraphProcessor.h"
#include "TaskGenerator.h"

using namespace omnetpp;
using namespace veins;

class VehicleSpawner : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    //virtual int  numInitStages() const override { return 2; }

private:
    std::unique_ptr<XMLProcessorBoost> xml;
    TraCIScenarioManager*        manager_ = nullptr;
    TraCICommandInterface*  traci_   = nullptr;

    std::unique_ptr<GraphProcessor>    graph_;
    std::unique_ptr<TaskGenerator>     taskGen_;
    cMessage *spawnTimer = nullptr;

};

#endif

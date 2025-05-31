#pragma once
#include "veins/veins.h"
#include "veins_inet/veins_inet.h"
#include "veins/modules/application/traci/TraCIDemoRSU11p.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
//#include "veins/modules/application/traci/TraCIScenarioManagerAccess.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "XMLProcessor.h"
#include <memory>
#include "GraphProcessor.h"
#include "TaskGenerator.h"
using namespace omnetpp;

namespace veins {

class RSUControlApp : public TraCIDemoRSU11p {
public:
    void initialize(int stage) override;
    void finish() override;
    char* mergeContent(long vehAddr);

protected:
    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    virtual int numInitStages() const override { return 1; }
private:
    bool hasStopped = false;
    int subscribedServiceId = 0;
    cMessage* sendBeacon;
    cMessage* spawnMsg_ = nullptr;
    int messageCount;
    std::unique_ptr<XMLProcessorBoost> xml;
    TraCIScenarioManager*        manager_ = nullptr;
    TraCICommandInterface*  traci_   = nullptr;

    std::unique_ptr<GraphProcessor>    graph_;
    std::unique_ptr<TaskGenerator>     taskGen_;

};
}

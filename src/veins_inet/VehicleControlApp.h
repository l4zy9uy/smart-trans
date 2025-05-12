#pragma once
#include "veins/veins.h"
#include "veins/modules/application/traci/TraCIDemo11p.h"
using namespace omnetpp;

namespace veins {

class  VehicleControlApp : public TraCIDemo11p {
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
    void handleLowerMsg(cMessage* msg) override;
private:
    bool hasStopped = false;
    int subscribedServiceId = 0;
    cMessage* sendBeacon;
};
}

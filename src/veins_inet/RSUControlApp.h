#pragma once
#include "veins/veins.h"
#include "veins_inet/veins_inet.h"
#include "veins/modules/application/traci/TraCIDemoRSU11p.h"

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
private:
    bool hasStopped = false;
    int subscribedServiceId = 0;
    cMessage* sendBeacon;
    int messageCount;
};
}

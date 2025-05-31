#include "RSUControlApp.h"
#include "Constant.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include <cstring>
#include <boost/json.hpp>
#include <list>

namespace json = boost::json;

using namespace veins;

Register_Class(RSUControlApp);

char* RSUControlApp::mergeContent(long vehAddr) {
    std::ostringstream buf;
    buf << '|' << vehAddr;
    return strdup(buf.str().c_str());
}

void RSUControlApp::initialize(int stage) {
    if (stage == 0) {
        manager_ = TraCIScenarioManagerAccess().get();
        // we don’t need TraCI right now, but keep a pointer for future use
        // (it will be valid from stage 1 on)
    }
    else if (stage == 1) {
        // TraCI connection is up → get command interface
        traci_ = manager_->getCommandInterface();
    }
}

void RSUControlApp::finish() {
    //Duoc goi khi RSU ket thuc cong viec
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void RSUControlApp::onBSM(DemoSafetyMessage *bsm) {
    //for my own simulation circle
}

void RSUControlApp::onWSM(BaseFrame1609_4* wsm) {
    auto* enc = wsm->getEncapsulatedPacket();
    if (auto* bc = dynamic_cast<TraCIDemo11pMessage*>(enc)) {
//        messageCount++;
//        EV_INFO << "RSU recv #" << messageCount << " at " << simTime() << endl;
//
//        if (messageCount % 5 == 0) {
//            // pick a target edge
//            std::string newEdge = "E30";
//
//            // --- build JSON with Boost.JSON ---
//            json::object jobj;
//            jobj["action"]     = "redirect";
//            jobj["targetEdge"] = newEdge;
//
//            std::string jsonText = json::serialize(jobj);
//            EV_INFO << "RSU JSON: " << jsonText << endl;
//
//            // send redirect WSM
//            auto* reply = new TraCIDemo11pMessage();
//            reply->setDemoData(strdup(jsonText.c_str()));
//            reply->setSenderAddress(myId);
//
//            auto* frame = new BaseFrame1609_4();
//            frame->encapsulate(reply);
//            populateWSM(frame);
//            send(frame, lowerLayerOut);
//
//            EV_INFO << "RSU sent REDIRECT to " << newEdge << endl;
//        }
    }
}


void RSUControlApp::onWSA(DemoServiceAdvertisment *wsa) {
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void RSUControlApp::handleSelfMsg(cMessage *msg) {
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void RSUControlApp::handlePositionUpdate(cObject *obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

}



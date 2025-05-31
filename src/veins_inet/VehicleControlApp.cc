#include "VehicleControlApp.h"
#include "Constant.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#define BOOST_JSON_STANDALONE
#include <boost/json/src.hpp>

namespace json = boost::json;

using namespace veins;

Register_Class(VehicleControlApp);

char* VehicleControlApp::mergeContent(long vehAddr) {
    std::ostringstream buf;
    buf << '|' << vehAddr;
    return strdup(buf.str().c_str());
}

void VehicleControlApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Set up self-message timer for periodic beacon
        mobility = TraCIMobilityAccess().get(getParentModule());
        traciVehicle = mobility->getVehicleCommandInterface();
        sendBeacon = new cMessage("sendBeacon");
    }
    else if (stage == 1) {
        // Schedule first beacon at 5 seconds
        double jitter = uniform(0, 0.5);        // ≤ 0.5 s random offset
        mobility   = TraCIMobilityAccess().get(getParentModule());
        traci      = mobility->getCommandInterface();
        traciVehicle   = mobility->getVehicleCommandInterface();
        traciVehicle->getParameter("srcJunction", srcJunc_);
        traciVehicle->getParameter("dstJunction", dstJunc_);
        EV_INFO << "Vehicle knows src=" << srcJunc_
                << " dst=" << dstJunc_ << endl;

        scheduleAt(simTime() + 1.0 + jitter, sendBeacon);
        //scheduleAt(simTime() + 1, sendBeacon);
        EV << "Scheduled periodic beacon every 1s." << endl;
    }
}

void VehicleControlApp::finish()
{
    DemoBaseApplLayer::finish();
}

void VehicleControlApp::onBSM(DemoSafetyMessage* bsm)
{
    //for my own simulation circle

}

void VehicleControlApp::onWSM(BaseFrame1609_4* wsm)
{
    EV_INFO << "*** Vehicle " << mobility->getExternalId()
                << " onWSM() called at t=" << simTime() << endl;
        // extract the TraCI message
    auto* bc = dynamic_cast<TraCIDemo11pMessage*>(wsm->getEncapsulatedPacket());
    if (!bc) return;

    // parse the JSON payload
    std::string raw = bc->getDemoData();
    json::error_code ec;
    json::value jv = json::parse(raw, ec);
    if (ec) {
        EV_WARN << "JSON parse error: " << ec.message() << endl;
        return;
    }

    // expect an object { "action": "...", "targetEdge": "E30" }
    auto& obj = jv.as_object();
    auto it = obj.find("action");
    if (it == obj.end()                         // missing key
     || it->value().as_string() != "redirect")  // wrong value
        return;

    // extract targetEdge
    std::string targetEdge = obj.at("targetEdge").as_string().c_str();

    auto traci  = mobility->getCommandInterface();
    std::string currLane = traciVehicle->getLaneId();
    bool canTurn = false;

    // get all allowed outgoing links from this lane
    for (auto &link : traci->lane(currLane).getLinks()) {
        // each 'link.approachedLane' is the ID of the lane we would go into
        std::string toLane = link.approachedLane;
        // ask SUMO which road that lane belongs to
        if (traci->lane(toLane).getRoadId() == targetEdge) {
            canTurn = true;
            break;
        }
    }

    if (!canTurn) {
        EV_INFO << "Cannot turn to “" << targetEdge
                << "” from current lane “" << currLane << "” – keep following original route\n";
        return;
    }

    // reroute
    try {
        traciVehicle->newRoute(targetEdge);
        EV_INFO << "Vehicle " << mobility->getExternalId()
                << " → newRoute(\"" << targetEdge << "\") at " << simTime() << endl;
    }
    catch (cRuntimeError& ex) {
        EV_WARN << "Redirect to \"" << targetEdge << "\" failed: "
                << ex.what() << endl;
    }
}

void VehicleControlApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void VehicleControlApp::handleSelfMsg(cMessage* msg)
{
    /* First handle *our own* timer ------------------------------- */
    if (msg == sendBeacon) {
        /* Build JSON --------------------------------------------- */
        std::string vid   = mobility->getExternalId();          // vehicle ID
        double      speed = mobility->getSpeed();
        Coord       pos   = curPosition;     // works in Veins‑INET
        std::string road  = mobility->getRoadId();

        std::ostringstream ss;
        ss << "{"
           << "\"vehicleId\":\"" << vid << "\","
           << "\"speed\":" << speed << ","
           << "\"position\":{\"x\":" << pos.x << ",\"y\":" << pos.y << "},"
           << "\"roadId\":\"" << road << "\""
           << "}";

        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        wsm->setDemoData(strdup(ss.str().c_str()));
        wsm->setSenderAddress(myId);

        BaseFrame1609_4* frame = new BaseFrame1609_4();
        frame->encapsulate(wsm);
        populateWSM(frame);
        send(frame, lowerLayerOut);

        EV_INFO << "Sent to RSU: " << ss.str() << endl;

        /* RESCHEDULE for the next second ------------------------- */
        scheduleAt(simTime() + 1, sendBeacon);
        return;                         // do *not* fall through to base class
    }

    /* For all other self‑messages keep parent behaviour ---------- */
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void VehicleControlApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

}

void VehicleControlApp::handleLowerMsg(cMessage* msg) {
    DemoBaseApplLayer::handleLowerMsg(msg);
}

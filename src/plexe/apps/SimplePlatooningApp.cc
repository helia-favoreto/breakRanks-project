#include "plexe/apps/SimplePlatooningApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "plexe/scenarios/BaseScenario.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"

#include "veins/base/utils/FindModule.h"

#include <iostream>
#include <random>

using namespace veins;

namespace plexe {

Define_Module(SimplePlatooningApp);

std::random_device rd;
std::mt19937 gen(rd());

void SimplePlatooningApp::initialize(int stage) {
    BaseApp::initialize(stage); //in this way we extend the code from the parent class
    if (stage == 1) {
        if (positionHelper->isLeader()) {
            //only the leader reads this parameter at init time,
            // followers get the value broadcast in the message...
            breakRanksSafetyDistance =
                    par("breakRanksSafetyDistance").doubleValueInUnit("m");

            rfgCounter = 0;
            LOG << "velocity for leader " << " is "
                       << plexeTraciVehicle->getCruiseControlDesiredSpeed()
                       << "\n";
        }
        if (!positionHelper->isLeader()) {
            LOG << "velocity after dismantling for " << positionHelper->getId()
                       << " is " << speedAfterBreaking << "\n";
        }
        // connect application to lower layer
        protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"),
                gate("lowerLayerOut"), gate("lowerControlIn"),
                gate("lowerControlOut"));

        // register to the signal indicating failed unicast transmissions
        findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);
        scenario = FindModule<BaseScenario*>::findSubModule(getParentModule());
    }
}

BreakRanks* SimplePlatooningApp::createBreakRanksMessage() {
    BreakRanks *brmsg = new BreakRanks();
    brmsg->setVehicleId(positionHelper->getId());
    brmsg->setPlatoonId(positionHelper->getPlatoonId());
    brmsg->setSafetyDistance(breakRanksSafetyDistance);
    brmsg->setDestinationId(-1);
    brmsg->setExternalId(positionHelper->getExternalId().c_str());
    brmsg->setKind(MANEUVER_TYPE);
    return brmsg;
}

void SimplePlatooningApp::broadcastBreakRanksMessage() {
    if (leaderState != IDLE || !positionHelper->isLeader()) {
        throw cRuntimeError("Stop here wrong leader...");
    } else {
        getSimulation()->getActiveEnvir()->alert(
                "Leader starting application by sending a BreakRanks message");
        BreakRanks *brmsg = createBreakRanksMessage();
        sendBroadcast(brmsg);
        traciVehicle->setColor(TraCIColor(100, 100, 100, 255));
        leaderState = WAITREADYFORGOODBYE; //the leader enters in the state waiting for ReadyForGoodbye
    }
}

Goodbye* SimplePlatooningApp::createGoodbyeMessage() {
    Goodbye *gbmsg = new Goodbye();
    gbmsg->setVehicleId(positionHelper->getId());
    gbmsg->setPlatoonId(positionHelper->getPlatoonId());
    gbmsg->setExternalId(positionHelper->getExternalId().c_str());
    gbmsg->setKind(MANEUVER_TYPE);
    return gbmsg;
}

void SimplePlatooningApp::sendGoodbyeMessage() {
    if (leaderState != WAITREADYFORGOODBYE) {
        throw cRuntimeError(
                "Stop here! Leader is not in the WAITREADYFORGOODBYE state: wrong state for the leader...");
    }
    Goodbye *gbmsg = createGoodbyeMessage();
    std::vector<int> formation = positionHelper->getPlatoonFormation();
    int dest;
    // multicast transmission to each follower
    for (int i = 1; i < formation.size(); i++) {
        dest = formation[i];
        Goodbye *dup = gbmsg->dup();
        dup->setDestinationId(dest);
        sendUnicast(dup, dest);
        std::stringstream sumoextid;
        sumoextid << "vtypeauto." << dest;
        plexeTraciVehicle->removePlatoonMember(sumoextid.str());
    }
    leaderState = BREAKPLATOON;
}

ReadyForGoodbye* SimplePlatooningApp::createReadyForGoodbyeMessage() {
    ReadyForGoodbye *rfgmsg = new ReadyForGoodbye();
    rfgmsg->setVehicleId(positionHelper->getId());
    rfgmsg->setPlatoonId(positionHelper->getPlatoonId());
    rfgmsg->setDestinationId(positionHelper->getLeaderId());
    rfgmsg->setExternalId(positionHelper->getExternalId().c_str());
    rfgmsg->setKind(MANEUVER_TYPE);
    return rfgmsg;
}

void SimplePlatooningApp::sendReadyForGoodbyeMessage() {
    if (followerState != OPENGAP) {
        throw cRuntimeError(
                "Stop here! Follower is not in the OPENGAP state: wrong state for the follower...");
    }
    ReadyForGoodbye *rfgmsg = createReadyForGoodbyeMessage();
    sendUnicast(rfgmsg, rfgmsg->getDestinationId());

    LOG << "Follower " << rfgmsg->getVehicleId()
               << " sent ReadyForGoodbye message to leader "
               << rfgmsg->getDestinationId() << "." << endl;
    traciVehicle->setColor(TraCIColor(0, 255, 0, 255));
    getSimulation()->getActiveEnvir()->alert(
            "ReadyForGoodbye message sent by Follower");
    followerState = READYFORLEAVING;
}

void SimplePlatooningApp::sendUnicast(cPacket *msg, int destination) {
    Enter_Method_Silent();
    take(msg);
    BaseFrame1609_4 *frame = new BaseFrame1609_4("BaseFrame1609_4",
            msg->getKind());
    frame->setRecipientAddress(destination);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->encapsulate(msg);
    // send unicast frames using 11p only
    PlexeInterfaceControlInfo *ctrl = new PlexeInterfaceControlInfo();
    ctrl->setInterfaces(PlexeRadioInterfaces::VEINS_11P);
    frame->setControlInfo(ctrl);
    sendDown(frame);
}

void SimplePlatooningApp::sendBroadcast(cPacket *msg) {
    Enter_Method_Silent();
    take(msg);
    BaseFrame1609_4 *frame = new BaseFrame1609_4("BaseFrame1609_4",
            msg->getKind());
    frame->setRecipientAddress(-1);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->encapsulate(msg);
    // send broadcast frames using 11p only
    PlexeInterfaceControlInfo *ctrl = new PlexeInterfaceControlInfo();
    ctrl->setInterfaces(PlexeRadioInterfaces::VEINS_11P);
    frame->setControlInfo(ctrl);
    sendDown(frame);
}

void SimplePlatooningApp::handleLowerMsg(cMessage *msg) {
    BaseFrame1609_4 *frame = check_and_cast<BaseFrame1609_4*>(msg);

    cPacket *enc = frame->getEncapsulatedPacket();
    ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

    if (enc->getKind() == MANEUVER_TYPE) {

        ManeuverMessage *mm = check_and_cast<ManeuverMessage*>(
                frame->decapsulate());

        if (BreakRanks *msg = dynamic_cast<BreakRanks*>(mm)) {
            handleBreakRanks(msg);
            delete msg;
        }

        else if (ReadyForGoodbye *msg = dynamic_cast<ReadyForGoodbye*>(mm)) {
            handleReadyForGoodbye(msg);
            delete msg;
        }

        else if (Goodbye *msg = dynamic_cast<Goodbye*>(mm)) {
            handleGoodbye(msg);
            delete msg;
        }
        delete frame;
    } else
        BaseApp::handleLowerMsg(msg);
}

void SimplePlatooningApp::handleBreakRanks(const BreakRanks *msg) {
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()
            || msg->getDestinationId() == positionHelper->getLeaderId()) {
        throw cRuntimeError("Stop here!");
    }

    if (followerState != IDLE) {
        throw cRuntimeError(
                "Stop here! Follower is not in theIDLE state: wrong state for the follower!");
    } else {
        std::stringstream ss;
        ss << positionHelper->getId() << " is receiving a BreakRanks message\n";
        getSimulation()->getActiveEnvir()->alert(ss.str().c_str());
        followerState = OPENGAP;
        sd = msg->getSafetyDistance();
        plexeTraciVehicle->setCACCConstantSpacing(sd);
        checkDistance = new cMessage("checkDistance");
        scheduleAt(simTime(), checkDistance);
    }
}

void SimplePlatooningApp::handleReadyForGoodbye(const ReadyForGoodbye *msg) {
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()
            || msg->getDestinationId() != positionHelper->getLeaderId()) {
        throw cRuntimeError(
                "Stop here! Wrong leader and wrong platoon member.");
    }
    if (leaderState != WAITREADYFORGOODBYE) {
        throw cRuntimeError(
                "Stop here! Leader is not in the WAITREADYFORGOODBYE state: wrong state for the leader");
    }
    rfgCounter++;
    LOG << "counter= " << rfgCounter << endl;
    if (rfgCounter >= positionHelper->getPlatoonSize() - 1) {
        LOG << "Vector is full." << endl;
        getSimulation()->getActiveEnvir()->alert(
                "Vector is full, sending the Goodbye");
        sendGoodbyeMessage();
    }
}

void SimplePlatooningApp::handleGoodbye(const Goodbye *msg) {
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()
            || msg->getDestinationId() == positionHelper->getLeaderId()) {
        throw cRuntimeError("Stop here! Wrong field in the Goodbye message");
    }
    if (followerState != READYFORLEAVING) {
        throw cRuntimeError(
                "Stop here! Follower is not in the READYFORLEAVING state: wrong state for the follower");
    }
    std::stringstream ss;
    ss << positionHelper->getId()
            << " got a GoodbyeMessage, detaching and switching to ACC!\n";
    getSimulation()->getActiveEnvir()->alert(ss.str().c_str());

    plexeTraciVehicle->setACCHeadwayTime(1.2);
    plexeTraciVehicle->setActiveController(ACC);

    minSpeed = par("minSpeed").doubleValueInUnit("mps");
    maxSpeed = par("maxSpeed").doubleValueInUnit("mps");
    LOG << "min speed: " << minSpeed << endl;
    std::uniform_int_distribution<> distrSpeed(minSpeed, maxSpeed);
    plexeTraciVehicle->setCruiseControlDesiredSpeed(distrSpeed(gen));

    plexeTraciVehicle->enableAutoLaneChanging(true);
    //https://sumo.dlr.de/docs/TraCI/Change_Vehicle_State.html#lane_change_mode_0xb6
    plexeTraciVehicle->setLaneChangeMode(1621); // DEFAULT_NOTRACI_LC
    followerState = BREAKPLATOON;
}

void plexe::SimplePlatooningApp::handleSelfMsg(cMessage *msg) {
    if (msg == checkDistance) {
        if (followerState != OPENGAP) {
            throw cRuntimeError(
                    "Stop here! Follower is not in OPENGAP state: wrong state for the follower");
        } else {
            double distance = nan("nan");
            double relSpeed = nan("nan");
            plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
            LOG << positionHelper->getId() << " now at: " << distance
                       << " meters" << endl;
            if (distance >= (sd - 0.1)) {
                traciVehicle->setColor(TraCIColor(100, 100, 100, 255));
                std::stringstream ss;
                ss << positionHelper->getId() << " reached the target"
                        " distance, sending ReadyForGoodbye\n";
                getSimulation()->getActiveEnvir()->alert(ss.str().c_str());
                sendReadyForGoodbyeMessage();
            } else {
                scheduleAt(simTime() + 0.1, checkDistance);
            }
        }
    } else {
        BaseApp::handleSelfMsg(msg);
    }
}

SimplePlatooningApp::~SimplePlatooningApp() {
    cancelAndDelete(checkDistance);
}

} // namespace plexe

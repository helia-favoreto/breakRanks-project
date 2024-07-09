#include "plexe/scenarios/BreakRanksScenario.h"
#include "plexe/apps/SimplePlatooningApp.h"
#include "veins/base/utils/FindModule.h"

using namespace veins;

namespace plexe {

Define_Module(BreakRanksScenario);

void BreakRanksScenario::initialize(int stage) {

    SimpleScenario::initialize(stage);

    if (stage == 0) {
        appl = FindModule<SimplePlatooningApp*>::findSubModule(
                getParentModule());
        leaderSpeed = par("leaderSpeed").doubleValueInUnit("mps");
    }

    if (stage == 2) {
        // average speed
        breakRanksTime = par("breakRanksTime").doubleValueInUnit("s");

        if (positionHelper->isLeader()) {
            getSimulation()->getActiveEnvir()->alert("leader starting from the scenario");
            // set base cruising speed
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed);
            breakRanksMsg = new cMessage("breakRanksMsg");
            scheduleAt(breakRanksTime, breakRanksMsg);
        } else {
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + 30);
        }
    }
}

void BreakRanksScenario::handleMessage(cMessage *msg) {
    if (msg == breakRanksMsg) {
        //
        appl->broadcastBreakRanksMessage();
    }
}

BreakRanksScenario::~BreakRanksScenario(){
    cancelAndDelete(breakRanksMsg);
}
/*void handleSelfMessage(cMessage *msg) {

}*/

} // namespace plexe

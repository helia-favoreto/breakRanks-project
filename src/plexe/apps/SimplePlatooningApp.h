//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SIMPLEPLATOONINGAPP_H_
#define SIMPLEPLATOONINGAPP_H_

#include "plexe/scenarios/BaseScenario.h"
#include "plexe/apps/BaseApp.h"
#include "plexe/messages/ReadyForGoodbye_m.h"
#include "plexe/messages/BreakRanks_m.h"
#include "plexe/messages/Goodbye_m.h"


namespace plexe {

class SimplePlatooningApp: public BaseApp {

public:
    SimplePlatooningApp() {
    }

    virtual void initialize(int stage) override;
    void broadcastBreakRanksMessage();
    void sendReadyForGoodbyeMessage();
    void sendGoodbyeMessage();
    virtual void sendUnicast(cPacket *msg, int destination);
    virtual void sendBroadcast(cPacket *msg);
    virtual void handleSelfMsg(cMessage *msg) override;

    double speedAfterBreaking;
    double minSpeed;
    double maxSpeed;

    virtual ~SimplePlatooningApp();

protected:
    double breakRanksSafetyDistance;
    double sd;
    BaseScenario *scenario;
    virtual void handleLowerMsg(cMessage *msg) override;
    void handleBreakRanks(const BreakRanks *msg);
    void handleReadyForGoodbye(const ReadyForGoodbye *msg);
    void handleGoodbye(const Goodbye *msg);

private:

    ReadyForGoodbye* createReadyForGoodbyeMessage();
    BreakRanks* createBreakRanksMessage();
    Goodbye* createGoodbyeMessage();
    cMessage *checkDistance;
    enum vehicleState{ //define the states for leader and follower
        IDLE, // leader and follower state
        WAITREADYFORGOODBYE, //leader state
        OPENGAP, //follower state
        READYFORLEAVING, //follower state
        BREAKPLATOON, //leader and follower state

    };
    vehicleState leaderState = IDLE;
    vehicleState followerState = IDLE;

    int rfgCounter; //counter ReadyForGoodbye messages

};

} // namespace plexe

#endif // SIMPLEPLATOONINGAPP_H_ //

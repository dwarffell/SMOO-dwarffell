#pragma once

#include "server/Client.hpp"

struct FreezeTagScore {
    // Score info
    int mScore = 0;

    /* Functions */

    void addScore(float add)
    {
        mScore += add;
        Client::sendFreezeInfPacket();
    };

    // Events
    void eventScoreDebug() { addScore(1); };
};
#pragma once

#include <stdint.h>
#include "server/Client.hpp"

struct FreezeTagScore {
    // Score info
    uint16_t mScore = 0;

    /* Functions */

    void addScore(float add)
    {
        mScore += add;
        Client::sendFreezeInfPacket();
    };

    // Events
    void eventScoreDebug() { addScore(1); };
};
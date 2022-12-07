#pragma once

#include "server/Client.hpp"

struct FreezeTagScore {
    // Score info
    float mScore = 0.f;
    float mRoundScore = 0.f;
    // Round info
    bool mIsInRound = false;
    int mRound = -1;

    /* Functions */

    void addScore(float add)
    {
        mScore += add;
        mRoundScore += add;
        Client::sendFreezeInfPacket();
    };

    // Events
    void eventScoreDebug() { addScore(1); };
};
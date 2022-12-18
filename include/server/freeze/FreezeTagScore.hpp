#pragma once

#include "layouts/FreezeTagIcon.h"
#include "server/Client.hpp"
#include <stdint.h>

class FreezeTagScore {
public:
    uint16_t mScore = 0;
    uint16_t mPrevScore = 0;

    void setTargetLayout(FreezeTagIcon* icon) { mIcon = icon; };

    void addScore(int add, const char* description);

    // Events
    void eventScoreDebug() { addScore(1, "Debugging!"); };
    void eventScoreUnfreeze() { addScore(3, "Rescued"); };
    void eventScoreFreeze() { addScore(1, "Caught"); };
    void eventScoreLastSurvivor() { addScore(10, "Last Survivor"); };

private:
    FreezeTagIcon* mIcon;
};
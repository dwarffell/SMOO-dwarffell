#pragma once

#include "sead/math/seadVector.h"

class PlayerRecoverySafetyPoint {
public:
    void startRecovery(float);
    void startBubbleWait();
    void noticeDangerousPoint(sead::Vector3f const&, bool);
};
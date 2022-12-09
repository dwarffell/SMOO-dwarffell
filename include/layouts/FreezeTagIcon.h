#pragma once

#include "al/layout/LayoutActor.h"
#include "al/layout/LayoutInitInfo.h"
#include "al/util/NerveUtil.h"

#include "logger.hpp"

// TODO: kill layout if going through loading zone or paused

class FreezeTagIcon : public al::LayoutActor {
    public:
        FreezeTagIcon(const char* name, const al::LayoutInitInfo& initInfo);

        void appear() override;

        bool tryStart();
        bool tryEnd();
        
        void calcTeamSizes();
        void updatePlayerNames();
        void updateRunnerFreezeIcon();
        
        void exeAppear();
        void exeWait();
        void exeEnd();
        
    private:
        struct FreezeTagInfo *mInfo;

        const int mMaxRunners = 2;
        const int mMaxChasers = 1;
        int mRunnerPlayerCount = -1;
        int mChaserPlayerCount = -1;

        bool mIsRunner = true;

        float mRunnerFreezeIconAngle = 0.f;
};

namespace {
    NERVE_HEADER(FreezeTagIcon, Appear)
    NERVE_HEADER(FreezeTagIcon, Wait)
    NERVE_HEADER(FreezeTagIcon, End)
}
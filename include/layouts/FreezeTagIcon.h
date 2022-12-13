#pragma once

#include "al/layout/LayoutActor.h"
#include "al/layout/LayoutInitInfo.h"
#include "al/util/NerveUtil.h"

#include "layouts/FreezeTagRunnerSlot.h"

#include "container/seadPtrArray.h"
#include "logger.hpp"

// TODO: kill layout if going through loading zone or paused

class FreezeTagIcon : public al::LayoutActor {
public:
    FreezeTagIcon(const char* name, const al::LayoutInitInfo& initInfo);

    void appear() override;

    bool tryStart();
    bool tryEnd();

    void exeAppear();
    void exeWait();
    void exeEnd();

private:
    struct FreezeTagInfo* mInfo;

    sead::PtrArray<FreezeTagRunnerSlot> mRunnerSlots;

    const int mMaxRunners = 9;
    const int mMaxChasers = 1;

    bool mIsRunner = true;
    bool mIsOverlayShowing = false;

    float mRunnerFreezeIconAngle = 0.f;
    float mFreezeOverlayHeight = 415.f;
};

namespace {
NERVE_HEADER(FreezeTagIcon, Appear)
NERVE_HEADER(FreezeTagIcon, Wait)
NERVE_HEADER(FreezeTagIcon, End)
}
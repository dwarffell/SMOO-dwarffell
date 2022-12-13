#include "layouts/FreezeTagIcon.h"
#include "al/string/StringTmp.h"
#include "al/util.hpp"
#include "al/util/MathUtil.h"
#include "layouts/FreezeTagRunnerSlot.h"
#include "logger.hpp"
#include "main.hpp"
#include "math/seadVector.h"
#include "prim/seadSafeString.h"
#include "puppets/PuppetInfo.h"
#include "rs/util.hpp"
#include "server/Client.hpp"
#include "server/freeze/FreezeTagMode.hpp"
#include <cstdio>
#include <cstring>

FreezeTagIcon::FreezeTagIcon(const char* name, const al::LayoutInitInfo& initInfo)
    : al::LayoutActor(name)
{
    al::initLayoutActor(this, initInfo, "FreezeTagIcon", 0);

    mInfo = GameModeManager::instance()->getInfo<FreezeTagInfo>();
    mIsRunner = mInfo->mIsPlayerRunner;

    mRunnerSlots.tryAllocBuffer(mMaxRunners, al::getSceneHeap());
    for (int i = 0; i < mMaxRunners; i++) {
        FreezeTagRunnerSlot* newSlot = new (al::getSceneHeap()) FreezeTagRunnerSlot("RunnerSlot", initInfo);
        newSlot->init(i);
        mRunnerSlots.pushBack(newSlot);
    }

    mChaserSlots.tryAllocBuffer(mMaxChasers, al::getSceneHeap());
    for (int i = 0; i < mMaxChasers; i++) {
        FreezeTagChaserSlot* newSlot = new (al::getSceneHeap()) FreezeTagChaserSlot("ChaserSlot", initInfo);
        newSlot->init(i);
        mChaserSlots.pushBack(newSlot);
    }

    initNerve(&nrvFreezeTagIconEnd, 0);
    kill();
}

void FreezeTagIcon::appear()
{
    al::startAction(this, "Appear", 0);
    al::setNerve(this, &nrvFreezeTagIconAppear);

    for (int i = 0; i < mMaxRunners; i++)
        mRunnerSlots.at(i)->tryStart();

    for (int i = 0; i < mMaxChasers; i++)
        mChaserSlots.at(i)->tryStart();

    al::LayoutActor::appear();
}

bool FreezeTagIcon::tryEnd()
{
    if (!al::isNerve(this, &nrvFreezeTagIconEnd)) {
        al::setNerve(this, &nrvFreezeTagIconEnd);
        for (int i = 0; i < mMaxRunners; i++)
            mRunnerSlots.at(i)->tryEnd();

        for (int i = 0; i < mMaxChasers; i++)
            mChaserSlots.at(i)->tryEnd();

        return true;
    }
    return false;
}

bool FreezeTagIcon::tryStart()
{
    if (!al::isNerve(this, &nrvFreezeTagIconWait) && !al::isNerve(this, &nrvFreezeTagIconAppear)) {
        appear();
        return true;
    }

    return false;
}

void FreezeTagIcon::exeAppear()
{
    if (al::isActionEnd(this, 0)) {
        al::setNerve(this, &nrvFreezeTagIconWait);
    }
}

void FreezeTagIcon::exeWait()
{
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait", 0);
    }

    // Show or hide the frozen UI overlay
    float targetHeight = mInfo->mIsPlayerFreeze ? 360.f : 415.f;
    mFreezeOverlayHeight = al::lerpValue(mFreezeOverlayHeight, targetHeight, 0.08f);
    al::setPaneLocalTrans(this, "PicFreezeOverlayTop", { 0.f, mFreezeOverlayHeight, 0.f });
    al::setPaneLocalTrans(this, "PicFreezeOverlayBot", { 0.f, -mFreezeOverlayHeight, 0.f });
}

void FreezeTagIcon::exeEnd()
{

    if (al::isFirstStep(this)) {
        al::startAction(this, "End", 0);
    }

    if (al::isActionEnd(this, 0)) {
        kill();
    }
}

namespace {
NERVE_IMPL(FreezeTagIcon, Appear)
NERVE_IMPL(FreezeTagIcon, Wait)
NERVE_IMPL(FreezeTagIcon, End)
}
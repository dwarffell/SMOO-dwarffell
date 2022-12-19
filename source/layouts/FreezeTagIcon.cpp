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
    al::hidePane(this, "Endgame");

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

    // Set all overlay positons
    setFreezeOverlayHeight();
    setSpectateOverlayHeight();

    // Update score event info
    if (mScoreEventIsQueued) {
        mScoreEventIsQueued = false;
        al::setPaneStringFormat(this, "TxtScoreNum", "%i", mScoreEventValue);
        al::setPaneStringFormat(this, "TxtScoreDesc", "%s", mScoreEventDesc);
    }

    if (mScoreEventTime >= 0.f) {
        mScoreEventTime += Time::deltaTime;

        // Calculate score event pane's position
        sead::Vector3f targetPos = mScoreEventTime < 3.f ? sead::Vector3f(0.f, 280.f, 0.f) : sead::Vector3f(-650.f, 420.f, 0.f);
        if (!mInfo->mIsPlayerRunner)
            targetPos.x *= -1.f;

        al::lerpVec(&mScoreEventPos, mScoreEventPos, targetPos, 0.05f);
        al::setPaneLocalTrans(this, "ScoreEvent", mScoreEventPos);

        // Calculate score event pane's scale
        float targetScale = mScoreEventTime < 3.f ? 1.0f : 0.f;
        mScoreEventScale = al::lerpValue(mScoreEventScale, targetScale, 0.05f);
        al::setPaneLocalScale(this, "ScoreEvent", { mScoreEventScale, mScoreEventScale });
    }

    // Spectate UI
    if (mInfo->mIsPlayerFreeze && mSpectateName)
        al::setPaneStringFormat(this, "TxtSpectateTarget", "%s", mSpectateName);

    // Endgame UI
    if (mEndgameIsDisplay) {
        if (al::isHidePane(this, "Endgame"))
            al::showPane(this, "Endgame");

        mEndgameTextSize = al::lerpValue(mEndgameTextSize, 1.1f, 0.02f);
        mEndgameTextAngle = al::lerpValue(mEndgameTextAngle, 5.f, 0.01f);

        al::setPaneLocalScale(this, "PicEndgameText", { mEndgameTextSize, mEndgameTextSize });
        al::setPaneLocalRotate(this, "PicEndgameText", { 0.f, 0.f, mEndgameTextAngle });
    }

    if (!mEndgameIsDisplay && !al::isHidePane(this, "Endgame"))
        al::hidePane(this, "Endgame");
}

void FreezeTagIcon::queueScoreEvent(int eventValue, const char* eventDesc)
{
    mScoreEventTime = 0.f;
    mScoreEventIsQueued = true;
    mScoreEventValue = eventValue;
    mScoreEventDesc = eventDesc;
    mScoreEventPos = sead::Vector3f(0.f, 280.f, 0.f);
    mScoreEventScale = 1.35f;
}

void FreezeTagIcon::setFreezeOverlayHeight()
{
    // Show or hide the frozen UI overlay
    float targetHeight = mInfo->mIsPlayerFreeze ? 360.f : 415.f;
    mFreezeOverlayHeight = al::lerpValue(mFreezeOverlayHeight, targetHeight, 0.08f);
    al::setPaneLocalTrans(this, "PicFreezeOverlayTop", { 0.f, mFreezeOverlayHeight, 0.f });
    al::setPaneLocalTrans(this, "PicFreezeOverlayBot", { 0.f, -mFreezeOverlayHeight, 0.f });
}

void FreezeTagIcon::setSpectateOverlayHeight()
{
    // Show or hide the spectator UI
    float targetHeight = mInfo->mIsPlayerFreeze && mInfo->mRunnerPlayers.size() > 0 && !mEndgameIsDisplay ? -250.f : -400.f;
    mSpectateOverlayHeight = al::lerpValue(mSpectateOverlayHeight, targetHeight, 0.04f);
    al::setPaneLocalTrans(this, "Spectate", { 0.f, mSpectateOverlayHeight, 0.f });
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
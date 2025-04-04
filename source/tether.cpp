#include "al/LiveActor/LiveActor.h"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/MathUtil.h"

#include "game/GameData/GameDataFunction.h"

#include "sead/math/seadMathCalcCommon.h"

#include "helpers.hpp"

#include "puppets/PuppetInfo.h"

#include "rs/util.hpp"

#include "tether.h"

PlayerTether& getTether()
{
    static PlayerTether i;
    return i;
}

void PlayerTether::tick(StageScene* scene, PlayerActorHakoniwa* p1)
{
    mSceneFrames++;

    if (!mIsSceneAlive)
        return;

    float closePuppetDistance = -1.f;
    int newPCount = Client::getConnectCount();

    if (newPCount != mPlayerCount) {
        mPlayerCount = newPCount;
        mSceneFrames = -180;
    }

    // Puppet finder
    if (!mDuoPartner.isEmpty()) {
        mClosePup = Client::getPuppetInfo(mDuoPartner.cstr());
        mIsTargetPupAlive = !al::isEqualSubString(PlayerAnims::FindStr(mClosePup->curAnim), "Dead");

        closePuppetDistance = al::calcDistance(p1, mClosePup->playerPos);
    } else {
        mClosePup = nullptr;
        mIsTargetPupAlive = false;
    }

    // Player pulling
    if (closePuppetDistance >= mPullDistanceMin && mClosePup && mClosePup->isConnected && mSceneFrames > 90 && mIsTargetPupAlive && !mIsRingPull) {
        if (closePuppetDistance < mPullDistanceMax) {
            al::LiveActor* hack = p1->getPlayerHackKeeper()->currentHackActor;
            sead::Vector3f target = mClosePup->playerPos;

            if (!hack) {
                sead::Vector3f* playerPos = al::getTransPtr(p1);
                sead::Vector3f direction = target - *playerPos;
                al::normalize(&direction);

                playerPos->add((direction * mPullPower) * ((al::calcDistance(p1, target) - mPullDistanceMin) / mPullPowerRate));
            } else {
                sead::Vector3f* hackPos = al::getTransPtr(hack);
                sead::Vector3f direction = target - *hackPos;
                al::normalize(&direction);

                hackPos->add((direction * mPullPower) * ((al::calcDistance(hack, target) - mPullDistanceMin) / mPullPowerRate));
            }
        }
    }

    // Player warping
    // if (mClosePup && !mClosePup->isInSameStage && mSceneFrames > 150 && mIsTargetPupAlive) {
    //     mIsSceneChangeFromPuppet = true;
    //     GameDataHolderAccessor accessor = scene->mHolder;
    //     ChangeStageInfo info(accessor.mData, "start", mClosePup->stageName, false, mClosePup->scenarioNo, ChangeStageInfo::SubScenarioType::UNK);
    //     GameDataFunction::tryChangeNextStage(accessor, &info);
    // }

    // Player Flicking
    // if (closePuppetDistance < mPullDistanceMax && closePuppetIndex != -1 && mSceneFrames > 50) {
    //     if (al::isPadTriggerPressLeftStick(-1) && rs::isPlayerOnGround(p1) && !p1->getPlayerHackKeeper()->currentHackActor) {
    //         p1->startDemoPuppetable();
    //         mIsRingPull = true;
    //         mRingPullAnimFrame = 0.f;
    //     } else if ((!al::isPadHoldPressLeftStick(-1) && mIsRingPull)
    //         || (al::calcDistance(p1, Client::getPuppetInfo(closePuppetIndex)->playerPos) > mMaxPullFlingDist && mIsRingPull)
    //         || (!mIsTargetPupAlive && mIsRingPull)) {

    //         p1->endDemoPuppetable();
    //         mIsRingPull = false;

    //         PuppetInfo* pup = Client::getPuppetInfo(closePuppetIndex);
    //         if(!pup)
    //             return;

    //         sead::Vector3f target = pup->playerPos;
    //         sead::Vector3f& playerPos = al::getTrans(p1);
    //         sead::Vector3f& playerVel = al::getVelocity(p1);

    //         float yDistance = playerPos.y - target.y;

    //         if (al::calcDistance(p1, target) > mMinPullFlingDist && al::calcDistance(p1, target) < mMaxPullFlingDist && mIsTargetPupAlive) {
    //             target.y += yDistance + 1350.f;
    //             sead::Vector3f direction = target - playerPos;

    //             al::normalize(&direction);

    //             playerPos.y += 50.f;
    //             playerVel.add((direction * mPullPower) * ((al::calcDistance(p1, target) - mPullDistanceMin) / (mPullPowerRate * 2.f)) * mRingVelocityMulti);
    //         }
    //     }

    //     if (mIsRingPull) {
    //         p1->mPlayerAnimator->startAnim("LandStiffen");
    //         p1->mPlayerAnimator->setAnimFrame(fmod(mRingPullAnimFrame, 40.f) + 20.f);
    //         mRingPullAnimFrame++;
    //     }
    // }
}

void PlayerTether::setSceneAlive(const char* stageName, sead::Vector3f* playerPos, PlayerActorBase* player)
{
    mIsSceneAlive = true;
    mIsRingPull = false;
    mSceneName = stageName;
    mPlayerPos = playerPos;
    mSceneFrames = 0;

    if (mIsSceneChangeFromPuppet) {
        // Set the translation without a collision check somehow please and thank you
        player->startDemoPuppetable();
        *mPlayerPos = mClosePup->playerPos;
        player->endDemoPuppetable();
        mIsSceneChangeFromPuppet = false;
    }

    return;
}

void PlayerTether::setSceneKilled()
{
    mIsSceneAlive = false;
    mIsRingPull = false;
    mPlayerPos = nullptr;
    mSceneFrames = 0;
}

sead::Vector3f* PlayerTether::getPlayerPos()
{
    return mPlayerPos;
}

sead::Vector3f* PlayerTether::getPuppetPos()
{
    PuppetInfo* pup = mClosePup;
    if (pup)
        return &pup->playerPos;
    else
        return nullptr;
}

float PlayerTether::getSceneFrames()
{
    return mSceneFrames;
}

float PlayerTether::getPullDistanceMin()
{
    return mPullDistanceMin;
}

bool PlayerTether::isSceneAlive()
{
    return mIsSceneAlive;
}

void PlayerTether::setDuoPartner(const char* name)
{
    mDuoPartner.clear();
    mDuoPartner.append(name);
}
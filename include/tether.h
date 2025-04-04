#pragma once

#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/MathUtil.h"
#include "al/util/VectorUtil.h"

#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/StageScene/StageScene.h"

#include "algorithms/PlayerAnims.h"

#include "puppets/PuppetHolder.hpp"
#include "puppets/PuppetInfo.h"

#include "server/Client.hpp"

#include "math/seadMathCalcCommon.hpp"
#include "math/seadVector.h"
#include "math/seadVector.hpp"
#include "math/seadVectorCalcCommon.hpp"

class PlayerTether {
private:
    // Player info
    bool mIsSceneAlive = false;
    const char* mSceneName = nullptr;
    sead::Vector3f* mPlayerPos = nullptr;
    PuppetInfo* mClosePup = nullptr;

    sead::FixedSafeString<0x10> mDuoPartner;

    // Scene info
    int mSceneFrames = 0;
    int mPlayerCount = -1;
    bool mIsSceneChangeFromPuppet = false;

    // Ring pull
    bool mIsRingPull = false;
    float mRingPullAnimFrame = 0.f;
    float mRingVelocityMulti = 9.f;
    const float mMinPullFlingDist = 1000.f;
    const float mMaxPullFlingDist = 3200.f;

    // Configuration
    float mPullPower = 50.f;
    float mPullPowerRate = 50.f;
    float mPullDistanceMin = 600.f;
    float mPullDistanceMax = 3000.f;

public:
    void tick(StageScene* scene, PlayerActorHakoniwa* p1);

    void setSceneKilled();
    void setSceneAlive(const char* stageName, sead::Vector3f* playerPos, PlayerActorBase* player);

    sead::Vector3f* getPlayerPos();
    sead::Vector3f* getPuppetPos();
    inline PuppetInfo* getClosePuppet() { return mClosePup; }
    float getSceneFrames();
    float getPullDistanceMin();

    bool isSceneAlive();

    int getDifficultyDistance() { return mPullDistanceMin; }

    void setDifficultyVeryEasy() { mPullDistanceMin = 2000.f; }
    void setDifficultyEasy() { mPullDistanceMin = 1000.f; }
    void setDifficultyNormal() { mPullDistanceMin = 800.f; }
    void setDifficultyHard() { mPullDistanceMin = 600.f; }
    void setDifficultyVeryHard() { mPullDistanceMin = 350.f; }

    void setDuoPartner(const char*);
    void clearDuoPartner() { mDuoPartner.clear(); }

    // Visual data
    bool mBounceInc = true;
    float mBounceDotProg = 0.f;
    bool mIsTargetPupAlive = true;
};

PlayerTether& getTether();
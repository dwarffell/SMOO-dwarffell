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

    // Scene info
    int mSceneFrames = 0;
    int mPlayerCount = -1;
    bool mIsSceneChangeFromPuppet = false;

    // Ring pull
    bool mIsRingPull = false;
    float mRingPullAnimFrame = 0.f;
    float mRingVelocityMulti = 9.f;

    // Consts
    const float mPullPower = 1.5f;
    const float mPullPowerRate = 75.f;
    const float mPullDistanceMin = 600.f;

public:
    void tick(StageScene* scene, PlayerActorHakoniwa* p1);

    void setSceneKilled();
    void setSceneAlive(const char* stageName, sead::Vector3f* playerPos, PlayerActorBase* player);

    sead::Vector3f* getPlayerPos();
    sead::Vector3f* getPuppetPos();
    float getSceneFrames();
    float getPullDistanceMin();

    bool isSceneAlive();

    // Visual data
    bool mBounceInc = true;
    float mBounceDotProg = 0.f;
    bool mIsTargetPupAlive = true;
};

PlayerTether& getTether();
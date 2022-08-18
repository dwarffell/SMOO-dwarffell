#include "server/snh/SardineMode.hpp"
#include "al/async/FunctorV0M.hpp"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "game/GameData/GameDataHolderAccessor.h"
#include "game/Layouts/CoinCounter.h"
#include "game/Layouts/MapMini.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/PlayerFunction.h"
#include "heap/seadHeapMgr.h"
#include "layouts/HideAndSeekIcon.h"
#include "logger.hpp"
#include "rs/util.hpp"
#include "server/Client.hpp"
#include "server/gamemode/GameModeBase.hpp"
#include "server/gamemode/GameModeFactory.hpp"
#include "server/gamemode/GameModeManager.hpp"
#include "server/gamemode/GameModeTimer.hpp"
#include <cmath>
#include <heap/seadHeap.h>

#include "basis/seadNew.h"
#include "server/snh/SardineConfigMenu.hpp"

SardineMode::SardineMode(const char* name)
    : GameModeBase(name)
{
}

void SardineMode::init(const GameModeInitInfo& info)
{
    mSceneObjHolder = info.mSceneObjHolder;
    mMode = info.mMode;
    mCurScene = (StageScene*)info.mScene;
    mPuppetHolder = info.mPuppetHolder;

    GameModeInfoBase* curGameInfo = GameModeManager::instance()->getInfo<HideAndSeekInfo>();

    sead::ScopedCurrentHeapSetter heapSetter(GameModeManager::instance()->getHeap());

    if (curGameInfo)
        Logger::log("Gamemode info found: %s %s\n", GameModeFactory::getModeString(curGameInfo->mMode), GameModeFactory::getModeString(info.mMode));
    else
        Logger::log("No gamemode info found\n");
    if (curGameInfo && curGameInfo->mMode == mMode) {
        mInfo = (SardineInfo*)curGameInfo;
        mModeTimer = new GameModeTimer(mInfo->mHidingTime);
        Logger::log("Reinitialized timer with time %d:%.2d\n", mInfo->mHidingTime.mMinutes, mInfo->mHidingTime.mSeconds);
    } else {
        if (curGameInfo)
            delete curGameInfo; // attempt to destory previous info before creating new one

        mInfo = GameModeManager::instance()->createModeInfo<SardineInfo>();

        mModeTimer = new GameModeTimer();
    }

    mModeLayout = new SardineIcon("SardineIcon", *info.mLayoutInitInfo);

    mModeLayout->showSolo();

    // mModeTimer->disableTimer();
}

void SardineMode::begin()
{
    mModeLayout->appear();

    mIsFirstFrame = true;

    if (mInfo->mIsIt) {
        mModeTimer->enableTimer();
        mModeLayout->showPack();
    } else {
        mModeTimer->disableTimer();
        mModeLayout->showSolo();
    }

    CoinCounter* coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    if (coinCounter->mIsAlive)
        coinCounter->tryEnd();
    if (coinCollect->mIsAlive)
        coinCollect->tryEnd();
    if (compass->mIsAlive)
        compass->end();
    if (playGuideLyt->mIsAlive)
        playGuideLyt->end();

    GameModeBase::begin();
}

void SardineMode::end()
{

    mModeLayout->tryEnd();

    mModeTimer->disableTimer();

    CoinCounter* coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    if (!coinCounter->mIsAlive)
        coinCounter->tryStart();
    if (!coinCollect->mIsAlive)
        coinCollect->tryStart();
    if (!compass->mIsAlive)
        compass->appearSlideIn();
    if (!playGuideLyt->mIsAlive)
        playGuideLyt->appear();

    GameModeBase::end();
}

void SardineMode::update()
{

    PlayerActorBase* playerBase = rs::getPlayerActor(mCurScene);

    bool isYukimaru = !playerBase->getPlayerInfo(); // if PlayerInfo is a nullptr, that means we're dealing with the bound bowl racer

    if (mIsFirstFrame) {
        if (mInfo->mIsUseGravityCam && mTicket)
            al::startCamera(mCurScene, mTicket, -1);
        mIsFirstFrame = false;
    }

    if (!mInfo->mIsIt && playerBase) {
        for (size_t i = 0; i < mPuppetHolder->getSize(); i++) {
            PuppetInfo* curInfo = Client::getPuppetInfo(i);

            if (!curInfo) {
                Logger::log("Checking %d, hit bounds %d-%d\n", i, mPuppetHolder->getSize(), Client::getMaxPlayerCount());
                break;
            }

            float pupDist = al::calcDistance(playerBase, curInfo->playerPos);
            if (curInfo->isConnected && curInfo->isInSameStage && curInfo->isIt && !mInfo->mIsIt && !isYukimaru && pupDist < 300.f) {
                if (((PlayerActorHakoniwa*)playerBase)->mDimKeeper->is2DModel == curInfo->is2D && !PlayerFunction::isPlayerDeadStatus(playerBase)) {
                    mInfo->mIsIt = true;
                    mModeTimer->enableTimer();
                    mModeLayout->showPack();

                    Client::sendTagInfPacket();
                }
            }
        }
    } else
        mModeTimer->updateTimer();

    if (mInfo->mIsUseGravity && !isYukimaru) {
        sead::Vector3f gravity;
        if (rs::calcOnGroundNormalOrGravityDir(&gravity, playerBase, playerBase->getPlayerCollision())) {
            gravity = -gravity;
            al::normalize(&gravity);
            al::setGravity(playerBase, gravity);
            al::setGravity(((PlayerActorHakoniwa*)playerBase)->mHackCap, gravity);
        }

        if (al::isPadHoldL(-1)) {
            if (al::isPadTriggerRight(-1)) {
                if (al::isActiveCamera(mTicket)) {
                    al::endCamera(mCurScene, mTicket, -1, false);
                    mInfo->mIsUseGravityCam = false;
                } else {
                    al::startCamera(mCurScene, mTicket, -1);
                    mInfo->mIsUseGravityCam = true;
                }
            }
        } else if (al::isPadTriggerZL(-1)) {
            if (al::isPadTriggerLeft(-1)) {
                killMainPlayer(((PlayerActorHakoniwa*)playerBase));
            }
        }
    }

    if (al::isPadTriggerUp(-1) && !al::isPadHoldZL(-1)) {
        mInfo->mIsIt = !mInfo->mIsIt;

        mModeTimer->toggleTimer();

        if (mInfo->mIsIt)
            mModeLayout->showPack();
        else
            mModeLayout->showSolo();

        Client::sendTagInfPacket();
    }

    mInfo->mHidingTime = mModeTimer->getTime();
}
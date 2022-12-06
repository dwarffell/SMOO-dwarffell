#include "server/freeze/FreezeTagMode.hpp"
#include <cmath>
#include "al/async/FunctorV0M.hpp"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "game/GameData/GameDataHolderAccessor.h"
#include "game/Layouts/CoinCounter.h"
#include "game/Layouts/MapMini.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "heap/seadHeapMgr.h"
#include "layouts/HideAndSeekIcon.h"
#include "logger.hpp"
#include "rs/util.hpp"
#include "server/gamemode/GameModeBase.hpp"
#include "server/Client.hpp"
#include "server/gamemode/GameModeTimer.hpp"
#include <heap/seadHeap.h>
#include "server/gamemode/GameModeManager.hpp"
#include "server/gamemode/GameModeFactory.hpp"

#include "basis/seadNew.h"
#include "server/freeze/FreezeTagConfigMenu.hpp"

FreezeTagMode::FreezeTagMode(const char* name) : GameModeBase(name) {}

void FreezeTagMode::init(const GameModeInitInfo& info) {
    mSceneObjHolder = info.mSceneObjHolder;
    mMode = info.mMode;
    mCurScene = (StageScene*)info.mScene;
    mPuppetHolder = info.mPuppetHolder;

    GameModeInfoBase* curGameInfo = GameModeManager::instance()->getInfo<HideAndSeekInfo>();

    if (curGameInfo) Logger::log("Gamemode info found: %s %s\n", GameModeFactory::getModeString(curGameInfo->mMode), GameModeFactory::getModeString(info.mMode));
    else Logger::log("No gamemode info found\n");
    if (curGameInfo && curGameInfo->mMode == mMode) {
        mInfo = (FreezeTagInfo*)curGameInfo;
    } else {
        if (curGameInfo) delete curGameInfo;  // attempt to destory previous info before creating new one
        mInfo = GameModeManager::instance()->createModeInfo<FreezeTagInfo>();
    }

    mModeLayout = new HideAndSeekIcon("HideAndSeekIcon", *info.mLayoutInitInfo);
    mModeLayout->showSeeking();

}

void FreezeTagMode::begin() {
    mModeLayout->appear();

    mIsFirstFrame = true;

    if (!mInfo->mIsPlayerRunner) {
        mModeLayout->showHiding();
    } else {
        mModeLayout->showSeeking();
    }

    CoinCounter *coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    mInvulnTime = 0;

    if(coinCounter->mIsAlive)
        coinCounter->tryEnd();
    if(coinCollect->mIsAlive)
        coinCollect->tryEnd();
    if (compass->mIsAlive)
        compass->end();
    if (playGuideLyt->mIsAlive)
        playGuideLyt->end();

    GameModeBase::begin();
}

void FreezeTagMode::end() {

    mModeLayout->tryEnd();

    CoinCounter *coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    mInvulnTime = 0.0f;

    if(!coinCounter->mIsAlive)
        coinCounter->tryStart();
    if(!coinCollect->mIsAlive)
        coinCollect->tryStart();
    if (!compass->mIsAlive)
        compass->appearSlideIn();
    if (!playGuideLyt->mIsAlive)
        playGuideLyt->appear();

    GameModeBase::end();
}

void FreezeTagMode::update() {

    PlayerActorBase* playerBase = rs::getPlayerActor(mCurScene);

    bool isYukimaru = !playerBase->getPlayerInfo(); // if PlayerInfo is a nullptr, that means we're dealing with the bound bowl racer

    if (mIsFirstFrame) {
        mIsFirstFrame = false;
    }

    if (!mInfo->mIsPlayerRunner) {
        if (mInvulnTime >= 5) {  

            // if (playerBase) {
            //     for (size_t i = 0; i < mPuppetHolder->getSize(); i++)
            //     {
            //         PuppetInfo *curInfo = Client::getPuppetInfo(i);

            //         if (!curInfo) {
            //             Logger::log("Checking %d, hit bounds %d-%d\n", i, mPuppetHolder->getSize(), Client::getMaxPlayerCount());
            //             break;
            //         }

            //         if(curInfo->isConnected && curInfo->isInSameStage && curInfo->isIt) { 

            //             float pupDist = al::calcDistance(playerBase, curInfo->playerPos); // TODO: remove distance calculations and use hit sensors to determine this

            //             if (!isYukimaru) {
            //                 if(pupDist < 200.f && ((PlayerActorHakoniwa*)playerBase)->mDimKeeper->is2DModel == curInfo->is2D) {
            //                     if(!PlayerFunction::isPlayerDeadStatus(playerBase)) {
                                    
            //                         GameDataFunction::killPlayer(GameDataHolderAccessor(this));
            //                         playerBase->startDemoPuppetable();
            //                         al::setVelocityZero(playerBase);
            //                         rs::faceToCamera(playerBase);
            //                         ((PlayerActorHakoniwa*)playerBase)->mPlayerAnimator->endSubAnim();
            //                         ((PlayerActorHakoniwa*)playerBase)->mPlayerAnimator->startAnimDead();

            //                         mInfo->mIsPlayerIt = true;
            //                         mModeTimer->disableTimer();
            //                         mModeLayout->showSeeking();
                                    
            //                         Client::sendTagInfPacket();
            //                     }
            //                 } else if (PlayerFunction::isPlayerDeadStatus(playerBase)) {

            //                     mInfo->mIsPlayerIt = true;
            //                     mModeTimer->disableTimer();
            //                     mModeLayout->showSeeking();

            //                     Client::sendTagInfPacket();
                                
            //                 }
            //             }
            //         }
            //     }
            // }
            
        }else {
            mInvulnTime += Time::deltaTime;
        }
    }

    if (al::isPadTriggerUp(-1) && !al::isPadHoldZL(-1))
    {
        mInfo->mIsPlayerRunner = !mInfo->mIsPlayerRunner;

        if(!mInfo->mIsPlayerRunner) {
            mInvulnTime = 0;
            mModeLayout->showHiding();
        } else {
            mModeLayout->showSeeking();
        }

        Client::sendTagInfPacket();
    }
}
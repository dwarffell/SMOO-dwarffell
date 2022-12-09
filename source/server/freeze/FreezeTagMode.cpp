#include "server/freeze/FreezeTagMode.hpp"
#include <cmath>
#include "actors/PuppetActor.h"
#include "al/async/FunctorV0M.hpp"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/NerveUtil.h"
#include "game/GameData/GameDataHolderAccessor.h"
#include "game/Layouts/CoinCounter.h"
#include "game/Layouts/MapMini.h"
#include "game/Player/HackCap.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "heap/seadHeapMgr.h"
#include "layouts/FreezeTagIcon.h"
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

    mModeLayout = new FreezeTagIcon("FreezeTagIcon", *info.mLayoutInitInfo);

    //Create main player's ice block
    mMainPlayerIceBlock = new FreezePlayerBlock("MainPlayerBlock");
    mMainPlayerIceBlock->init(*info.mActorInitInfo);
}

void FreezeTagMode::begin() {
    mModeLayout->appear();

    mIsFirstFrame = true;

    if (!mInfo->mIsPlayerRunner) {
        // mModeLayout->showHiding();
    } else {
        // mModeLayout->showSeeking();
    }

    CoinCounter *coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    mInvulnTime = 0.f;

    if(coinCounter->mIsAlive)
        coinCounter->tryEnd();
    if(coinCollect->mIsAlive)
        coinCollect->tryEnd();
    if (compass->mIsAlive)
        compass->end();
    if (playGuideLyt->mIsAlive)
        playGuideLyt->end();

    //Update other players on your freeze tag state when starting
    Client::sendFreezeInfPacket();

    GameModeBase::begin();
}

void FreezeTagMode::end() {

    mModeLayout->tryEnd();

    CoinCounter *coinCollect = mCurScene->mSceneLayout->mCoinCollectLyt;
    CoinCounter* coinCounter = mCurScene->mSceneLayout->mCoinCountLyt;
    MapMini* compass = mCurScene->mSceneLayout->mMapMiniLyt;
    al::SimpleLayoutAppearWaitEnd* playGuideLyt = mCurScene->mSceneLayout->mPlayGuideMenuLyt;

    mInvulnTime = 0.f;

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

    // First frame stuff, remove this?
    if (mIsFirstFrame) {
        mIsFirstFrame = false;
    }

    //Main player's ice block state controller
    if(mInfo->mIsPlayerFreeze) {
        if(!al::isAlive(mMainPlayerIceBlock))
            mMainPlayerIceBlock->appear();
        
        //Lock block onto player
        al::setTrans(mMainPlayerIceBlock, al::getTrans(playerBase));

    } else {
        if(al::isAlive(mMainPlayerIceBlock) && !al::isNerve(mMainPlayerIceBlock, &nrvFreezePlayerBlockDisappear))
            mMainPlayerIceBlock->end();
    }

    // Runner team frame checks
    if (mInfo->mIsPlayerRunner) {
        if (mInvulnTime >= 3 && !isYukimaru) {
            bool isPDead = PlayerFunction::isPlayerDeadStatus(playerBase);
            bool isP2D = ((PlayerActorHakoniwa*)playerBase)->mDimKeeper->is2D;

            for (size_t i = 0; i < mPuppetHolder->getSize(); i++) {
                PuppetInfo *curInfo = Client::getPuppetInfo(i);
                float pupDist = al::calcDistance(playerBase, curInfo->playerPos);

                if(!curInfo->isConnected)
                    continue;

                //Check for freeze
                if (!mInfo->mIsPlayerFreeze && pupDist < 200.f && isP2D == curInfo->is2D && !isPDead && !curInfo->isFreezeTagRunner)
                    trySetPlayerRunnerState(FreezeState::FREEZE);

                //Check for unfreeze
                if (mInfo->mIsPlayerFreeze && pupDist < 150.f && isP2D == curInfo->is2D && !isPDead && curInfo->isFreezeTagRunner && !curInfo->isFreezeTagFreeze)
                    trySetPlayerRunnerState(FreezeState::ALIVE);
            }
        } else {
            mInvulnTime += Time::deltaTime;
        }
    }

    if (al::isPadTriggerRight(-1) && !al::isPadHoldZL(-1) && !al::isPadHoldX(-1) && !al::isPadHoldY(-1)) {
        mInfo->mIsPlayerRunner = !mInfo->mIsPlayerRunner;

        if(!mInfo->mIsPlayerRunner)
            mInvulnTime = 0;

        Client::sendFreezeInfPacket();
    }

    if (al::isPadTriggerUp(-1) && al::isPadHoldX(-1))
        trySetPlayerRunnerState(FreezeState::ALIVE);
    if (al::isPadTriggerUp(-1) && al::isPadHoldY(-1))
        trySetPlayerRunnerState(FreezeState::FREEZE);
    if (al::isPadTriggerUp(-1) && al::isPadHoldA(-1)) {
        mInfo->mPlayerTagScore.eventScoreDebug();
    }
}

bool FreezeTagMode::trySetPlayerRunnerState(FreezeState newState)
{
    PlayerActorBase* playerBase = rs::getPlayerActor(mCurScene);
    bool isYukimaru = !playerBase->getPlayerInfo();

    if(mInfo->mIsPlayerFreeze == newState || isYukimaru)
        return false;
    
    PlayerActorHakoniwa* player = (PlayerActorHakoniwa*)playerBase;
    HackCap* hackCap = player->mHackCap;
    
    if(newState == FreezeState::ALIVE) {
        mInfo->mIsPlayerFreeze = FreezeState::ALIVE;
        playerBase->endDemoPuppetable();
        mInvulnTime = 0.f;

    } else {
        mInfo->mIsPlayerFreeze = FreezeState::FREEZE;
        playerBase->startDemoPuppetable();
        
        player->mPlayerAnimator->endSubAnim();
        player->mPlayerAnimator->startAnim("DeadIce");
        
        hackCap->forcePutOn();
    }

    Client::sendFreezeInfPacket();

    return true;
}
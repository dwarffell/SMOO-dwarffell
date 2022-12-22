#include "al/util/LiveActorUtil.h"
#include "al/util/RandomUtil.h"
#include "math/seadVector.h"
#include "puppets/PuppetInfo.h"
#include "server/freeze/FreezeTagMode.hpp"
#include "al/alCollisionUtil.h"

/*
    ROUND START AND END FUNCTIONS
*/

void FreezeTagMode::startRound(int roundMinutes) {
    mInfo->mIsRound = true;

    mModeTimer->enableTimer();
    mModeTimer->disableControl();
    mModeTimer->setTimerDirection(false);
    // Starts at the round minutes - 1 (and 59 seconds to not instantly set off score event)
    mModeTimer->setTime(0.f, 59, roundMinutes-1, 0);
}

void FreezeTagMode::endRound() {
    mInfo->mIsRound = false;
    mModeTimer->disableTimer();

    if(!mIsEndgameActive) {
        if(!mInfo->mIsPlayerRunner) {
            mInfo->mIsPlayerRunner = true;
            Client::sendFreezeInfPacket();
            return;
        }

        mInfo->mPlayerTagScore.eventScoreRunnerWin();

        if(mInfo->mIsPlayerFreeze)
            trySetPlayerRunnerState(FreezeState::ALIVE);
    }
}

/*
    SET THE RUNNER PLAYER'S FROZEN/ALIVE STATE
*/

bool FreezeTagMode::trySetPlayerRunnerState(FreezeState newState)
{
    PlayerActorHakoniwa* player = getPlayerActorHakoniwa();
    if(!player)
        return false;

    if(mInfo->mIsPlayerFreeze == newState || !mInfo->mIsPlayerRunner)
        return false;
    
    HackCap* hackCap = player->mHackCap;

    mInvulnTime = 0.f;
    
    if(newState == FreezeState::ALIVE) {
        mInfo->mIsPlayerFreeze = FreezeState::ALIVE;
        player->endDemoPuppetable();
    } else {
        if(!mInfo->mIsRound)
            return false;

        mInfo->mIsPlayerFreeze = FreezeState::FREEZE;
        if(player->getPlayerHackKeeper()->currentHackActor)
            player->getPlayerHackKeeper()->cancelHackArea();
            
        player->startDemoPuppetable();
        player->mPlayerAnimator->endSubAnim();
        player->mPlayerAnimator->startAnim("DeadIce");

        hackCap->forcePutOn();

        mSpectateIndex = -1;

        if(isAllRunnerFrozen(nullptr))
            tryStartEndgameEvent();
    }

    Client::sendFreezeInfPacket();

    return true;
}

/*
    UPDATE PLAYER SCORES
    FUNCTION CALLED FROM client.cpp ON RECEIVING FREEZE TAG PACKETS
*/

void FreezeTagMode::tryScoreEvent(FreezeInf* incomingPacket, PuppetInfo* sourcePuppet)
{
    if(!mCurScene || !sourcePuppet || !GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return;
    
    if(!mCurScene->mIsAlive)
        return;

    // Get the distance of the incoming player
    PlayerActorBase* playerBase = rs::getPlayerActor(mCurScene);
    if(!playerBase)
        return;
    
    float puppetDistance = al::calcDistance(playerBase, sourcePuppet->playerPos);
    bool isInRange = puppetDistance < 400.f; // Only apply this score event if player is less than this many units away

    if(isInRange) {
        //Check for unfreeze score event
        if((mInfo->mIsPlayerRunner && !mInfo->mIsPlayerFreeze) && (sourcePuppet->isFreezeTagFreeze && !incomingPacket->isFreeze)) {
            mInfo->mPlayerTagScore.eventScoreUnfreeze();
        }

        //Check for freeze score event
        if((!mInfo->mIsPlayerRunner) && (!sourcePuppet->isFreezeTagFreeze && incomingPacket->isFreeze)) {
            mInfo->mPlayerTagScore.eventScoreFreeze();
        }
    }

    // Check if the current player is the last unfrozen runner!
    if(mInfo->mIsPlayerRunner && !mInfo->mIsPlayerFreeze && !sourcePuppet->isFreezeTagFreeze
    && incomingPacket->isFreeze && isPlayerLastSurvivor(sourcePuppet)) {
        mInfo->mPlayerTagScore.eventScoreLastSurvivor();
    }

    // Checks if every runner is frozen, starts endgame sequence if so
    if(!sourcePuppet->isFreezeTagFreeze && incomingPacket->isFreeze && isAllRunnerFrozen(sourcePuppet)) {
        tryStartEndgameEvent();
    }
}

/*
    HANDLE PLAYER RECOVERY
    Player recovery is started by entering a death area or an endgame (wipeout)
*/

bool FreezeTagMode::tryStartRecoveryEvent(bool isEndgame)
{
    PlayerActorHakoniwa* player = getPlayerActorHakoniwa();
    if(!player)
        return false;

    if(mRecoveryEventFrames > 0 || !mWipeHolder)
        return false; //Something isn't applicable here, return fail
    
    mRecoveryEventFrames = (mRecoveryEventLength / 2) * (isEndgame + 1);
    mWipeHolder->startClose("FadeBlack", (mRecoveryEventLength / 4) * (isEndgame + 1));

    if(!isEndgame)
        mRecoverySafetyPoint = player->mPlayerRecoverySafetyPoint->mSafetyPointPos;
    else
        mRecoverySafetyPoint = sead::Vector3f::zero;
    
    Logger::log("Recovery event %.00fx %.00fy %.00fz\n", mRecoverySafetyPoint.x, mRecoverySafetyPoint.y, mRecoverySafetyPoint.z);

    return true;
}

bool FreezeTagMode::tryEndRecoveryEvent()
{
    if(!mWipeHolder)
        return false; //Recovery event is already started, return fail
    
    mWipeHolder->startOpen(mRecoveryEventLength / 2);
    
    PlayerActorHakoniwa* player = getPlayerActorHakoniwa();
    if(!player)
        return false;

    // Set the player to frozen if they are a runner AND they had a valid recovery point
    if(mInfo->mIsPlayerRunner && mRecoverySafetyPoint != sead::Vector3f::zero && mInfo->mIsRound) {
        trySetPlayerRunnerState(FreezeState::FREEZE);
        warpToRecoveryPoint(player);
    } else {
        trySetPlayerRunnerState(FreezeState::ALIVE);
    }

    // If player is a chaser with a valid recovery point, teleport (and disable collisions)
    if(!mInfo->mIsPlayerRunner || !mInfo->mIsRound) {
        player->startDemoPuppetable();
        if(mRecoverySafetyPoint != sead::Vector3f::zero)
            warpToRecoveryPoint(player);
    }

    // If player is being made alive, force end demo puppet state
    if(!mInfo->mIsPlayerFreeze)
        player->endDemoPuppetable();

    if(!mIsEndgameActive)
        mModeLayout->hideEndgameScreen();

    return true;
}

void FreezeTagMode::warpToRecoveryPoint(al::LiveActor *actor)
{
    if(mInfo->mChaserPlayers.size() == 0 || !mInfo->mIsPlayerRunner || !mInfo->mIsRound) {
        al::setTrans(actor, mRecoverySafetyPoint);
        return;
    }

    PuppetInfo* inf = mInfo->mChaserPlayers.at(al::getRandom(mInfo->mChaserPlayers.size()));
    if(!inf->isInSameStage || !inf->isConnected) {
        al::setTrans(actor, mRecoverySafetyPoint);
        return;
    }

    al::setTrans(actor, inf->playerPos);
}

/*
    START AN ENDGAME EVENT (wipeout)
*/

void FreezeTagMode::tryStartEndgameEvent()
{
    mIsEndgameActive = true;
    mEndgameTimer = 0.f;
    mModeLayout->showEndgameScreen();

    PlayerActorHakoniwa* player = getPlayerActorHakoniwa();
    if(!player)
        return;
    
    player->startDemoPuppetable();
    rs::faceToCamera(player);
    player->mPlayerAnimator->endSubAnim();
    if(mInfo->mIsPlayerRunner)
        player->mPlayerAnimator->startAnim("RaceResultLose");
    else
        player->mPlayerAnimator->startAnim("RaceResultWin");
    
    // Award wipeout points to chasers
    if(!mInfo->mIsPlayerRunner)
        mInfo->mPlayerTagScore.eventScoreWipeout();

    endRound();
}
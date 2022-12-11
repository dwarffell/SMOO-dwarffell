#include "layouts/FreezeTagIcon.h"
#include "al/string/StringTmp.h"
#include "al/util.hpp"
#include "al/util/MathUtil.h"
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

    initNerve(&nrvHideAndSeekIconEnd, 0);
    kill();
}

void FreezeTagIcon::appear()
{
    al::startAction(this, "Appear", 0);
    al::setNerve(this, &nrvFreezeTagIconAppear);
    al::LayoutActor::appear();
}

bool FreezeTagIcon::tryEnd()
{
    if (!al::isNerve(this, &nrvFreezeTagIconEnd)) {
        al::setNerve(this, &nrvFreezeTagIconEnd);
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

void FreezeTagIcon::calcTeamSizes()
{
    int playerCount = Client::getMaxPlayerCount();

    // Calculate the runner and chaser count for HUD display
    mRunnerPlayerCount = mInfo->mIsPlayerRunner;
    mChaserPlayerCount = !mInfo->mIsPlayerRunner;

    for (int i = 0; i < playerCount; i++) {
        PuppetInfo* curPuppet = Client::getPuppetInfo(i);
        if (!curPuppet->isConnected)
            continue;

        if (curPuppet->isFreezeTagRunner)
            mRunnerPlayerCount++;
        else
            mChaserPlayerCount++;
    }

    if (mRunnerPlayerCount > mMaxRunners) {
        Logger::log("Runner player count warning!");
        mRunnerPlayerCount = mMaxRunners;
    }

    if (mChaserPlayerCount > mMaxChasers) {
        Logger::log("Chaser player count warning!");
        mChaserPlayerCount = mMaxChasers;
    }
}

void FreezeTagIcon::updatePlayerNames()
{
    if (mInfo->mIsPlayerRunner)
        al::setPaneStringFormat(this, "TxtRunner0Name", "%s", Client::instance()->getClientName());
    else
        al::setPaneStringFormat(this, "TxtChaser0Name", "%s", Client::instance()->getClientName());

    // Update runner team
    int skippedCount = 0; // When scanning through puppet list, increment this to keep the target pane correct
    for (int i = 0; i < mRunnerPlayerCount; i++) {
        if (mInfo->mIsPlayerRunner && i == 0)
            continue;

        // Grabs puppet index, goes back one if player is slot zero
        PuppetInfo* pup = Client::instance()->getPuppetInfo(i - mInfo->mIsPlayerRunner);

        if (!pup->isFreezeTagRunner) {
            skippedCount++;
            continue;
        }

        char paneBuf[0x20] = { 0 };
        sead::BufferedSafeStringBase<char> paneName = sead::BufferedSafeStringBase<char>(paneBuf, 0x20);
        paneName.appendWithFormat("TxtRunner%iName", i - skippedCount);

        al::setPaneStringFormat(this, paneName.cstr(), pup->puppetName);
    }

    // Update chaser team
    skippedCount = 0;
    for (int i = 0; i < mChaserPlayerCount; i++) {
        if (!mInfo->mIsPlayerRunner && i == 0)
            continue;

        PuppetInfo* pup = Client::instance()->getPuppetInfo(i - !mInfo->mIsPlayerRunner);

        if (pup->isFreezeTagRunner) {
            skippedCount++;
            continue;
        }

        char paneBuf[0x20] = { 0 };
        sead::BufferedSafeStringBase<char> paneName = sead::BufferedSafeStringBase<char>(paneBuf, 0x20);
        paneName.appendWithFormat("TxtChaser%iName", i - skippedCount);
        al::setPaneStringFormat(this, paneName.cstr(), pup->puppetName);
    }
}

void FreezeTagIcon::updateRunnerFreezeIcon()
{
    mRunnerFreezeIconAngle += 1.2f;
    int skippedCount = 0; // When scanning through puppet list, increment this to keep the target pane correct

    // Spin frozen symbol
    for (int i = 0; i < mRunnerPlayerCount; i++) {
        char paneBuf[0x20] = { 0 };
        sead::BufferedSafeStringBase<char> paneName = sead::BufferedSafeStringBase<char>(paneBuf, 0x20);
        paneName.appendWithFormat("PicRunner%iFreeze", i);

        al::setPaneLocalRotate(this, paneName.cstr(), { 0.f, 0.f, mRunnerFreezeIconAngle + (i * 13.5f) });

        if (mInfo->mIsPlayerRunner && i == 0) {
            float targetSize = mInfo->mIsPlayerFreeze ? 1.f : 0.f;
            mInfo->mFreezeIconSize = al::lerpValue(mInfo->mFreezeIconSize, targetSize, 0.05f);
            al::setPaneLocalScale(this, paneName.cstr(), { mInfo->mFreezeIconSize, mInfo->mFreezeIconSize });
        } else {
            PuppetInfo* curInfo = mInfo->mRunnerPlayers.at(i - mInfo->mIsPlayerRunner);

            float targetSize = curInfo->isFreezeTagFreeze ? 1.f : 0.f;
            curInfo->freezeIconSize = al::lerpValue(curInfo->freezeIconSize, targetSize, 0.05f);
            al::setPaneLocalScale(this, paneName.cstr(), { curInfo->freezeIconSize, curInfo->freezeIconSize });
        }
    }
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

    int playerCount = Client::getMaxPlayerCount();

    calcTeamSizes();

    // Player count validity check
    if (playerCount < (mMaxRunners + mMaxChasers)) {
        Logger::log("Too many players for Freeze Tag layout! %i/%i\n", playerCount, mMaxRunners + mMaxChasers);
        return;
    }

    updatePlayerNames();

    updateRunnerFreezeIcon();

    sead::Vector2f iconSize = al::getPaneLocalScale(this, "PicRunner0Freeze");
    al::setPaneStringFormat(this, "TxtDebug", "IsRunner: %s\nRunnerCount: %i\nChaserCount: %i\nMaxRunner: %i\nMaxChaser: %i\nIconSize: %.02f/%.02f", BTOC(mInfo->mIsPlayerRunner), mRunnerPlayerCount, mChaserPlayerCount, mMaxRunners, mMaxChasers, iconSize.x, iconSize.y);

    // if (mInfo->mIsPlayerFreeze)
    //     al::showPane(this, "PicRunner0Freeze");
    // else
    //     al::hidePane(this, "PicRunner0Freeze");

    // PuppetInfo* testPuppet = Client::instance()->getPuppetInfo(0);
    // if (testPuppet->isFreezeTagFreeze)
    //     al::showPane(this, "PicRunner1Freeze");
    // else
    //     al::hidePane(this, "PicRunner1Freeze");

    if (playerCount > 0) {

        char playerNameBuf[0x100] = { 0 }; // max of 16 player names if player name size is 0x10

        sead::BufferedSafeStringBase<char> playerList = sead::BufferedSafeStringBase<char>(playerNameBuf, 0x200);

        // Add your own name to the list at the top
        playerList.appendWithFormat("%s %s\n", mInfo->mIsPlayerRunner ? "&" : "%%", Client::instance()->getClientName());

        // Add all it players to list
        for (int i = 0; i < playerCount; i++) {
            PuppetInfo* curPuppet = Client::getPuppetInfo(i);
            if (curPuppet && curPuppet->isConnected && curPuppet->isIt)
                playerList.appendWithFormat("%s %s\n", curPuppet->isIt ? "&" : "%%", curPuppet->puppetName);
        }

        // Add not it players to list
        for (int i = 0; i < playerCount; i++) {
            PuppetInfo* curPuppet = Client::getPuppetInfo(i);
            if (curPuppet && curPuppet->isConnected && !curPuppet->isIt)
                playerList.appendWithFormat("%s %s\n", curPuppet->isIt ? "&" : "%%", curPuppet->puppetName);
        }

        al::setPaneStringFormat(this, "TxtPlayerList", playerList.cstr());
    }
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
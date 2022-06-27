#include "timeWarp.h"
#include "al/scene/Scene.h"
#include "al/util.hpp"
#include "al/util/LiveActorUtil.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "gfx/seadColor.h"
#include "rs/util.hpp"
#include <cmath>

timeContainer& getTimeContainer()
{
    static timeContainer i;
    return i;
}

void updateTimeStates(PlayerActorHakoniwa* p1)
{
    timeContainer& container = getTimeContainer();

    // If position has changed enough, push a new frame
    if (al::calcDistance(p1, container.lastRecordPosition) > container.minPushDistance && !rs::isActiveDemo(p1) && !container.isRewinding)
        pushNewFrame();

    if (al::isPadHoldR(-1) && !rs::isActiveDemo(p1) && (!container.timeFrames[container.minTrailLength].isEmptyFrame || container.isRewinding)) {
        rewindFrame(p1);
    } else if (container.isRewinding) {
        endRewind(p1);
    }
}

void removeOldestFrame()
{
    timeContainer& container = getTimeContainer();
    for (int i = 0; i < container.maxFrames; i++) {
        container.timeFrames[i] = container.timeFrames[i + 1];
    }
    container.timeFrames[container.frameCount].isEmptyFrame = true;
    return;
}

void pushNewFrame()
{
    timeContainer& container = getTimeContainer();

    // Before doing anything, if the frame container is full push data down
    if (container.maxFrames <= container.frameCount)
        removeOldestFrame();

    al::PlayerHolder* pHolder = al::getScenePlayerHolder(container.stageSceneRef);
    PlayerActorHakoniwa* p1 = (PlayerActorHakoniwa*)al::tryGetPlayerActor(pHolder, 0);

    if (container.timeFrames[container.frameCount].isEmptyFrame) {
        container.timeFrames[container.frameCount].isEmptyFrame = false;
        container.lastRecordColorFrame += 0.04f;
        container.timeFrames[container.frameCount].colorFrame = container.lastRecordColorFrame;
        if (p1->mHackKeeper->currentHackActor == nullptr) {
            container.timeFrames[container.frameCount].position = al::getTrans(p1);
            container.timeFrames[container.frameCount].gravity = al::getGravity(p1);
            container.timeFrames[container.frameCount].velocity = al::getVelocity(p1);
            container.timeFrames[container.frameCount].rotation = p1->mPoseKeeper->getQuat();
            container.timeFrames[container.frameCount].animation.clear();
            container.timeFrames[container.frameCount].animation.append(p1->mPlayerAnimator->curAnim);
            container.timeFrames[container.frameCount].animationFrame = p1->mPlayerAnimator->getAnimFrame();
        } else {
            container.timeFrames[container.frameCount].position = al::getTrans(p1->mHackKeeper->currentHackActor);
            container.timeFrames[container.frameCount].velocity = al::getVelocity(p1->mHackKeeper->currentHackActor);
        }
    }

    if (container.frameCount < container.maxFrames)
        container.frameCount++;

    container.lastRecordPosition = al::getTrans(p1);
    return;
}

void rewindFrame(PlayerActorHakoniwa* p1)
{
    timeContainer& container = getTimeContainer();
    bool isNotCaptured = p1->mHackKeeper->currentHackActor == nullptr;

    if (!container.isRewinding)
        startRewind(p1);

    if (isNotCaptured) {
        al::setTrans(p1, container.timeFrames[container.frameCount].position);
        al::setGravity(p1, container.timeFrames[container.frameCount].gravity);
        al::setVelocity(p1, container.timeFrames[container.frameCount].velocity);
        al::setQuat(p1, container.timeFrames[container.frameCount].rotation);
        if (!container.timeFrames[container.frameCount].animation.isEqual(p1->mPlayerAnimator->curAnim))
            p1->mPlayerAnimator->startAnim(container.timeFrames[container.frameCount].animation);
        p1->mPlayerAnimator->setAnimFrame(container.timeFrames[container.frameCount].animationFrame);
    } else {
        al::setTrans(p1->mHackKeeper->currentHackActor, container.timeFrames[container.frameCount].position);
        al::setVelocity(p1->mHackKeeper->currentHackActor, container.timeFrames[container.frameCount].velocity);
    }
    container.timeFrames[container.frameCount].isEmptyFrame = true;
    container.frameCount--;

    if (container.timeFrames[1].isEmptyFrame)
        endRewind(p1);

    return;
}

sead::Color4f calcColorFrame(float colorFrame)
{
    sead::Color4f returnColor = { 0.f, 0.f, 0.f, 0.7f };
    returnColor.r = sin(colorFrame) * 0.9f;
    returnColor.g = sin(colorFrame + 1.f) * 0.9f;
    returnColor.b = sin(colorFrame + 2.f) * 0.9f;

    if (returnColor.r < 0)
        returnColor.r = -returnColor.r;
    if (returnColor.g < 0)
        returnColor.g = -returnColor.g;
    if (returnColor.b < 0)
        returnColor.b = -returnColor.b;

    return returnColor;
}

void startRewind(PlayerActorHakoniwa* p1)
{
    timeContainer& container = getTimeContainer();
    al::Scene* scene = container.stageSceneRef;
    int filterID = al::getPostProcessingFilterPresetId(scene);

    if (!p1->mHackKeeper->currentHackActor) {
        p1->startDemoPuppetable();
        p1->mPlayerAnimator->startAnim("Default");
    }
    container.isRewinding = true;

    while (filterID != 4) {
        if (filterID > 4) {
            al::decrementPostProcessingFilterPreset(scene);
            filterID--;
        } else {
            filterID++;
            al::incrementPostProcessingFilterPreset(scene);
        }
    }

    return;
}
void endRewind(PlayerActorHakoniwa* p1)
{
    timeContainer& container = getTimeContainer();

    al::Scene* scene = container.stageSceneRef;
    int filterID = al::getPostProcessingFilterPresetId(scene);

    if (!p1->mHackKeeper->currentHackActor)
        p1->endDemoPuppetable();
    container.isRewinding = false;

    while (filterID != 0) {
        if (filterID > 0) {
            al::decrementPostProcessingFilterPreset(scene);
            filterID--;
        } else {
            filterID++;
            al::incrementPostProcessingFilterPreset(scene);
        }
    }
    return;
}
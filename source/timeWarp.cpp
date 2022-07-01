#include "timeWarp.h"
#include "al/scene/Scene.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "gfx/seadColor.h"
#include "rs/util.hpp"
#include <cmath>

TimeContainer& getTimeContainer()
{
    static TimeContainer i;
    return i;
}

void updateTimeStates(PlayerActorHakoniwa* p1)
{
    TimeContainer& container = getTimeContainer();

    //Cancel early if the invactive time is still running
    if(container.sceneInvactiveTime >= 0){
        container.sceneInvactiveTime--;
        return;
    }

    //Clear history on a capture
    if(container.isCaptured != (p1->mHackKeeper->currentHackActor != nullptr)){
        emptyFrameInfo();
        container.isCaptured = (p1->mHackKeeper->currentHackActor != nullptr);
    }

    //Clear history on 2D
    if(container.is2D != p1->mDimKeeper->is2D){
        emptyFrameInfo();
        container.is2D = p1->mDimKeeper->is2D;
    }

    // If position has changed enough, push a new frame
    if (container.timeFrames.isEmpty()){
        if (!rs::isActiveDemo(p1) && !container.isRewinding)
            pushNewFrame();
    } else {
        if ((al::calcDistance(p1, container.timeFrames.at(container.timeFrames.size()-1)->position) > container.minPushDistance
        || p1->mHackCap->isFlying()) && !rs::isActiveDemo(p1) && !container.isRewinding)
            pushNewFrame();
    }

    if (al::isPadHoldR(-1) && !rs::isActiveDemo(p1) && (container.timeFrames.size() >= container.minTrailLength || container.isRewinding)) {
        if(container.rewindFrameDelay >= container.rewindFrameDelayTarget) rewindFrame(p1);
        else container.rewindFrameDelay++;
    } else if (container.isRewinding) {
        endRewind(p1);
    }

    // if (al::isPadTriggerUp(-1)) {
    //     p1->mHackCap->showPuppetCap();
    // }
    // if (al::isPadTriggerRight(-1)) {
    //     p1->mHackCap->hidePuppetCap();
    // }
}

void pushNewFrame()
{
    TimeContainer& container = getTimeContainer();
    container.lastRecordColorFrame += 0.04f;
    TimeFrame* newFrame = nullptr; 
    sead::Heap* seqHeap = sead::HeapMgr::instance()->findHeapByName("SceneHeap",0);
    if (seqHeap) {
        newFrame = new (seqHeap) TimeFrame();
    } else {
        newFrame = new TimeFrame();
    }
    

    // Before doing anything, if the frame container is full push data down
    if (container.maxFrames <= container.timeFrames.size())
        delete container.timeFrames.popFront();

    al::PlayerHolder* pHolder = al::getScenePlayerHolder(container.stageSceneRef);
    PlayerActorHakoniwa* p1 = (PlayerActorHakoniwa*)al::tryGetPlayerActor(pHolder, 0);
    
    newFrame->colorFrame = container.lastRecordColorFrame;
    if (!p1->mHackKeeper->currentHackActor) {
        //Mario
        newFrame->position = al::getTrans(p1);
        newFrame->gravity = al::getGravity(p1);
        newFrame->velocity = al::getVelocity(p1);
        newFrame->rotation = p1->mPoseKeeper->getQuat();
        newFrame->animation.clear();
        newFrame->animation.append(p1->mPlayerAnimator->curAnim);
        newFrame->animationFrame = p1->mPlayerAnimator->getAnimFrame();

        //Cappy
        newFrame->capFrame.isFlying = p1->mHackCap->isFlying();
        newFrame->capFrame.position = al::getTrans(p1->mHackCap);
        newFrame->capFrame.rotation = p1->mHackCap->mJointKeeper->mJointRot;
        newFrame->capFrame.action = al::getActionName(p1->mHackCap);
    } else {
        //Capture
        newFrame->position = al::getTrans(p1->mHackKeeper->currentHackActor);
        newFrame->velocity = al::getVelocity(p1->mHackKeeper->currentHackActor);
        newFrame->rotation = p1->mHackKeeper->currentHackActor->mPoseKeeper->getQuat();
    }
    
    container.timeFrames.pushBack(newFrame);
    return;
}

void emptyFrameInfo()
{
    TimeContainer& container = getTimeContainer();
    container.timeFrames.clear();
    return;
}

void rewindFrame(PlayerActorHakoniwa* p1)
{
    TimeContainer& container = getTimeContainer();
    bool isCaptured = p1->mHackKeeper->currentHackActor != nullptr;
    container.rewindFrameDelay = 0;

    if (!container.isRewinding)
        startRewind(p1);

    if (!isCaptured) {
        //Mario
        al::setTrans(p1, container.timeFrames.back()->position);
        al::setGravity(p1, container.timeFrames.back()->gravity);
        al::setVelocity(p1, container.timeFrames.back()->velocity);
        al::setQuat(p1, container.timeFrames.back()->rotation);
        if (!container.timeFrames.back()->animation.isEqual(p1->mPlayerAnimator->curAnim))
            p1->mPlayerAnimator->startAnim(container.timeFrames.back()->animation);
        p1->mPlayerAnimator->setAnimFrame(container.timeFrames.back()->animationFrame);

        //Cappy
        
        //Initalize puppet cappy's state
        if(container.timeFrames.back()->capFrame.isFlying != p1->mHackCap->isFlying()){
            if(container.timeFrames.back()->capFrame.isFlying){ 
                p1->mHackCap->setupThrowStart();
            } else {
                p1->mHackCap->startCatch("Default", true, al::getTrans(p1));
                p1->mHackCap->forcePutOn();
            } 
        }

        //Toggle the puppet cap's visiblity
        if(container.timeFrames.back()->capFrame.isFlying){
            p1->mHackCap->showPuppetCap();
            // al::hideModel(al::getSubActor(p1, "頭"));
        } else {
            p1->mHackCap->hidePuppetCap();
            // al::showModel(al::getSubActor(p1, "頭"));
        }

        //Hide and show the cappy model on Mario's head
        al::LiveActor* headModel = al::getSubActor(p1->mPlayerModelHolder->currentModel->mLiveActor, "頭");
        if (headModel) { 
            if(container.timeFrames.back()->capFrame.isFlying) al::startVisAnimForAction(headModel, "CapOff"); 
            else al::startVisAnimForAction(headModel, "CapOn");
            // p1->makeActorDead();
        }

        al::setTrans(p1->mHackCap, container.timeFrames.back()->capFrame.position);
        p1->mHackCap->mJointKeeper->mJointRot = container.timeFrames.back()->capFrame.rotation;
        al::startAction(p1->mHackCap, container.timeFrames.back()->capFrame.action.cstr());
    } else {
        al::setTrans(p1->mHackKeeper->currentHackActor, container.timeFrames.back()->position);
        al::setVelocity(p1->mHackKeeper->currentHackActor, container.timeFrames.back()->velocity);
    }

    delete container.timeFrames.popBack();
    if (container.timeFrames.isEmpty())
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
    TimeContainer& container = getTimeContainer();
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
    TimeContainer& container = getTimeContainer();

    al::Scene* scene = container.stageSceneRef;
    int filterID = al::getPostProcessingFilterPresetId(scene);

    //Cleanup cappy state
    p1->mHackCap->startCatch("Default", true, al::getTrans(p1));
    p1->mHackCap->forcePutOn();
    p1->mHackCap->hidePuppetCap();

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
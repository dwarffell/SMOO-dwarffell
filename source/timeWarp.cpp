#include "timeWarp.h"
#include "al/LiveActor/LiveActor.h"
#include "al/scene/Scene.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "gfx/seadColor.h"
#include "prim/seadSafeString.h"
#include "rs/util.hpp"
#include <cmath>

TimeContainer& getTimeContainer()
{
    static TimeContainer i;
    return i;
}

void TimeContainer::init()
{
    timeFrames.allocBuffer(maxFrames, nullptr);
    sceneInvactiveTime = 15;
    return;
}

void TimeContainer::updateTimeStates(PlayerActorHakoniwa* p1)
{   
    al::LiveActor* hack = p1->mHackKeeper->currentHackActor;
    bool isCur2D = p1->mDimKeeper->is2D;
    
     //Cancel early if the invactive time is still running
    if(sceneInvactiveTime >= 0){
        sceneInvactiveTime--;
        return;
    }

    //Clear history on a capture
    if(isCapture != (hack != nullptr)){
        isCapture = hack != nullptr;
        isCaptureInvalid = false;
        emptyFrameInfo();

        if(isRewinding) endRewind(p1);

        //Don't update anything related to the trail on invalid captures
        if(isCapture)
            if(isInvalidCapture(p1->mHackKeeper->getCurrentHackName())) isCaptureInvalid = true;
    }

    //Clear history on 2D
    if(is2D != isCur2D){
        is2D = isCur2D;
        emptyFrameInfo();

        if(isRewinding) endRewind(p1);
    }

    if(isCaptureInvalid) return;

    // If position has changed enough, push a new frame
    if (timeFrames.isEmpty()){
        if (!rs::isActiveDemo(p1) && !isRewinding)
            pushNewFrame();
    } else {
        if ((al::calcDistance(p1, timeFrames.at(timeFrames.size()-1)->position) > minPushDistance
        || p1->mHackCap->isFlying()) && !rs::isActiveDemo(p1) && !isRewinding)
            pushNewFrame();
    }

    if (al::isPadHoldR(-1) && !rs::isActiveDemo(p1) && (timeFrames.size() >= minTrailLength || isRewinding)) {
        if(rewindFrameDelay >= rewindFrameDelayTarget) rewindFrame(p1);
        else rewindFrameDelay++;
    } else if (isRewinding) {
        endRewind(p1);
    }
}

TimeFrame* TimeContainer::getTimeFrame(uint32_t index)
{
    if(timeFrames.isEmpty()) return nullptr;

    //Return the index requested, or the highest frame possible if too large
    if(index > timeFrames.size()-1) return timeFrames.at(timeFrames.size()-1);
    return timeFrames.at(index);
}

int TimeContainer::getTimeArraySize()
{
    return timeFrames.size();
}

float TimeContainer::getColorFrame()
{
    return colorFrame;
}

int TimeContainer::getRewindDelay()
{
    return rewindFrameDelayTarget;
}

bool TimeContainer::isSceneActive()
{
    return sceneInvactiveTime == -1;
}

bool TimeContainer::isRewind()
{
    return isRewinding;
}

bool TimeContainer::isInvalidCapture(const char* curName)
{
    constexpr static const char* hackList[] = {
        "ElectricWire", "TRex", "Fukankun", //Binoculars
        "Cactus", "BazookaElectric", //Sub-area rocket
        "JugemFishing", //Lakitu
        "Fastener", //Zipper
        "GotogotonLake", "GotogotonCity", //Puzzle pieces
        "Senobi", //Uproot
        "Tree", "RockForest", "FukuwaraiFacePartsKuribo", "Imomu", //Tropical wiggler
        "AnagramAlphabetCharacter", "Car", "Manhole", "Tsukkun", //Pokio
        "Statue", "StatueKoopa", "KaronWing", "Bull", //Chargin' chuck
        "Koopa", "Yoshi"
    };
    int hackListSize = *(&hackList + 1) - hackList;

    for (int i = 0; i < hackListSize; i++) {
        if (al::isEqualString(curName, hackList[i])) {
            return true;
        }
    }

    return false;
}

void TimeContainer::setRewindDelay(int index)
{
    rewindFrameDelayTarget += index;

    if(rewindFrameDelayTarget < 0) rewindFrameDelayTarget = 0;
    return;
}

void TimeContainer::setInactiveTimer(int time)
{
    sceneInvactiveTime = time;
    return;
}

void TimeContainer::setTimeFramesEmpty()
{
    emptyFrameInfo();
    return;
}

void TimeContainer::pushNewFrame()
{
    colorFrame += colorFrameRate;
    TimeFrame* newFrame = nullptr; 
    sead::Heap* seqHeap = sead::HeapMgr::instance()->findHeapByName("SceneHeap",0);
    if (seqHeap) {
        newFrame = new (seqHeap) TimeFrame();
    } else {
        newFrame = new TimeFrame();
    }

    // Before doing anything, if the frame container is full push data down
    if (maxFrames <= timeFrames.size())
        delete timeFrames.popFront();

    al::PlayerHolder* pHolder = al::getScenePlayerHolder(stageSceneRef);
    PlayerActorHakoniwa* p1 = (PlayerActorHakoniwa*)al::tryGetPlayerActor(pHolder, 0);
    al::LiveActor* hack = p1->mHackKeeper->currentHackActor;
    
    newFrame->colorFrame = colorFrame;
    if (!hack) {
        //Mario
        newFrame->position = al::getTrans(p1);
        newFrame->gravity = al::getGravity(p1);
        newFrame->velocity = al::getVelocity(p1);
        newFrame->rotation = p1->mPoseKeeper->getQuat();
        newFrame->action.append(p1->mPlayerAnimator->curAnim);
        newFrame->actionFrame = p1->mPlayerAnimator->getAnimFrame();

        //Cappy
        newFrame->capFrame.isFlying = p1->mHackCap->isFlying();
        newFrame->capFrame.position = al::getTrans(p1->mHackCap);
        newFrame->capFrame.rotation = p1->mHackCap->mJointKeeper->mJointRot;
        newFrame->capFrame.action = al::getActionName(p1->mHackCap);
    } else {
        //Capture
        newFrame->position = al::getTrans(hack);
        newFrame->velocity = al::getVelocity(hack);
        newFrame->rotation = hack->mPoseKeeper->getQuat();
        newFrame->action = al::getActionName(hack);
    }
    
    timeFrames.pushBack(newFrame);
    return;
}

void TimeContainer::emptyFrameInfo()
{
    timeFrames.clear();
    colorFrame = 0.f;
    return;
}

void TimeContainer::rewindFrame(PlayerActorHakoniwa* p1)
{
    al::LiveActor* hack = p1->mHackKeeper->currentHackActor;
    al::LiveActor* headModel = al::getSubActor(p1->mPlayerModelHolder->currentModel->mLiveActor, "頭");
    rewindFrameDelay = 0;

    if (!isRewinding) startRewind(p1);

    if (!hack) {
        //Mario
        al::setTrans(p1, timeFrames.back()->position);
        al::setGravity(p1, timeFrames.back()->gravity);
        al::setVelocity(p1, timeFrames.back()->velocity);
        al::setQuat(p1, timeFrames.back()->rotation);
        if (!timeFrames.back()->action.isEqual(p1->mPlayerAnimator->curAnim))
            p1->mPlayerAnimator->startAnim(timeFrames.back()->action);
        p1->mPlayerAnimator->setAnimFrame(timeFrames.back()->actionFrame);

        //Cappy
        updateHackCap(p1->mHackCap, headModel);
        al::setTrans(p1->mHackCap, timeFrames.back()->capFrame.position);
        p1->mHackCap->mJointKeeper->mJointRot = timeFrames.back()->capFrame.rotation;
        al::startAction(p1->mHackCap, timeFrames.back()->capFrame.action.cstr());
    } else {
        al::setTrans(hack, timeFrames.back()->position);
        al::setVelocity(hack, timeFrames.back()->velocity);
        al::setQuat(hack, timeFrames.back()->rotation);
        al::startAction(hack, timeFrames.back()->action.cstr());
    }

    delete timeFrames.popBack();
    if (timeFrames.isEmpty())
        endRewind(p1);

    return;
}

void TimeContainer::updateHackCap(HackCap* cap, al::LiveActor* headModel)
{
    //Initalize puppet cappy's state
    if(timeFrames.back()->capFrame.isFlying != cap->isFlying()){
        if(timeFrames.back()->capFrame.isFlying){ 
            cap->setupThrowStart();
        } else {
            cap->startCatch("Default", true, al::getTrans(cap));
            cap->forcePutOn();
        } 
    }

    //Toggle the puppet cap's visiblity
    if(timeFrames.back()->capFrame.isFlying){
        cap->showPuppetCap();
        al::startVisAnimForAction(headModel, "CapOff");
    } else {
        cap->hidePuppetCap();
        al::startVisAnimForAction(headModel, "CapOn");
    }
    
    return;
}

sead::Color4f TimeContainer::calcColorFrame(float colorFrame)
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

void TimeContainer::startRewind(PlayerActorHakoniwa* p1)
{
    al::Scene* scene = stageSceneRef;
    int filterID = al::getPostProcessingFilterPresetId(scene);

    if (!p1->mHackKeeper->currentHackActor) {
        p1->startDemoPuppetable();
        p1->mPlayerAnimator->startAnim("Default");
    }
    isRewinding = true;

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
void TimeContainer::endRewind(PlayerActorHakoniwa* p1)
{
    al::Scene* scene = stageSceneRef;
    int filterID = al::getPostProcessingFilterPresetId(scene);

    if (!p1->mHackKeeper->currentHackActor){
        p1->endDemoPuppetable();
        
        //Cleanup cappy state
        p1->mHackCap->startCatch("Default", true, al::getTrans(p1));
        p1->mHackCap->forcePutOn();
        p1->mHackCap->hidePuppetCap();
    }
    
    isRewinding = false;

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
#include <sys/types.h>
#include "server/Client.hpp"
#include "al/LiveActor/LiveActor.h"
#include "al/actor/ActorInitInfo.h"
#include "al/actor/Placement.h"
#include "al/byaml/ByamlIter.h"
#include "al/nerve/Nerve.h"
#include "al/nerve/NerveExecutor.h"
#include "al/nerve/NerveKeeper.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/NerveUtil.h"
#include "game/Actors/WorldEndBorderKeeper.h"
#include "game/Layouts/CoinCounter.h"
#include "game/Player/Actions/PlayerActionGroundMoveControl.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/PlayerConst.h"
#include "game/Player/States/PlayerStateRunHakoniwa.h"
#include "game/StageScene/StageSceneStateOption.h"
#include "game/StageScene/StageSceneStatePauseMenu.h"
#include "game/StageScene/StageSceneStateServerConfig.hpp"
#include "logger.hpp"
#include "main.hpp"
#include "al/byaml/writer/ByamlWriter.h"
#include "math/seadVector.h"
#include "rs/util/InputUtil.h"
#include "sead/prim/seadSafeString.h"
#include "server/hns/HideAndSeekMode.hpp"
#include "game/Player/HackCap/CapFunction.h"

bool comboBtnHook(int port) {
    if (GameModeManager::instance()->isActive()) { // only switch to combo if any gamemode is active
        return !al::isPadHoldL(port) && al::isPadTriggerDown(port);
    } else {
        return al::isPadTriggerDown(port);
    }
}

void saveWriteHook(al::ByamlWriter* saveByml) {

    const char *serverIP = Client::getCurrentIP();
    const int serverPort = Client::getCurrentPort();

    if (serverIP) {
        saveByml->addString("ServerIP", serverIP);
    } else {
        saveByml->addString("ServerIP", "127.0.0.1");
    }

    if (serverPort) {
        saveByml->addInt("ServerPort", serverPort);
    } else {
        saveByml->addInt("ServerPort", 0);
    }

    saveByml->pop();
}

bool saveReadHook(int* padRumbleInt, al::ByamlIter const& saveByml, char const* padRumbleKey) {

    const char *serverIP = "";
    int serverPort = 0;

    if (al::tryGetByamlString(&serverIP, saveByml, "ServerIP")) {
        Client::setLastUsedIP(serverIP);
    }

    if (al::tryGetByamlS32(&serverPort, saveByml, "ServerPort")) {
        Client::setLastUsedPort(serverPort);
    }
    
    return al::tryGetByamlS32(padRumbleInt, saveByml, padRumbleKey);
}

bool registerShineToList(Shine* shineActor) {

    if (shineActor->shineId >= 0) {
        Client::tryRegisterShine(shineActor);
    } 

    return al::isAlive(shineActor);
}

void overrideNerveHook(StageSceneStatePauseMenu* thisPtr, al::Nerve* nrvSet) {

    if (al::isPadHoldZL(-1)) {
        al::setNerve(thisPtr, &nrvStageSceneStatePauseMenuServerConfig);
    } else {
        al::setNerve(thisPtr, nrvSet);
    }
}

StageSceneStateServerConfig *sceneStateServerConfig = nullptr;

void initStateHook(StageSceneStatePauseMenu *thisPtr, char const *stateName, al::Scene *host, al::LayoutInitInfo const &initInfo, FooterParts *footer,
                   GameDataHolder *data, bool unkBool) {
    thisPtr->mStateOption =
        new StageSceneStateOption(stateName, host, initInfo, footer, data, unkBool);

    sceneStateServerConfig = new StageSceneStateServerConfig("ServerConfig", host, initInfo, footer, data, unkBool);
}

void initNerveStateHook(StageSceneStatePauseMenu* stateParent, StageSceneStateOption* stateOption,
                        al::Nerve const* executingNerve, char const* stateName) {

    al::initNerveState(stateParent, stateOption, executingNerve, stateName);

    al::initNerveState(stateParent, sceneStateServerConfig, &nrvStageSceneStatePauseMenuServerConfig, "CustomNerveOverride");
}

// skips starting both coin counters
void startCounterHook(CoinCounter* thisPtr) {
    if (!GameModeManager::instance()->isActive()) {
        thisPtr->tryStart();
    }
}

// Simple hook that can be used to override isModeE3 checks to enable/disable certain behaviors
bool modeE3Hook() {
    return GameModeManager::instance()->isActive();
}

// Skips ending the play guide layout if a mode is active, since the mode would have already ended it
void playGuideEndHook(al::SimpleLayoutAppearWaitEnd* thisPtr) {
    if (!GameModeManager::instance()->isActive()) {
        thisPtr->end();
    }
}

// Gravity Hooks

void initHackCapHook(al::LiveActor *cappy) {
    al::initActorPoseTQGSV(cappy);
}

al::PlayerHolder* createTicketHook(StageScene* curScene) {
    // only creates custom gravity camera ticket if hide and seek mode is active
    if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
        al::CameraDirector* director = curScene->getCameraDirector();
        if (director) {
            if (director->mFactory) {
                al::CameraTicket* gravityCamera = director->createCameraFromFactory(
                    "CameraPoserCustom", nullptr, 0, 5, sead::Matrix34f::ident);

                HideAndSeekMode* mode = GameModeManager::instance()->getMode<HideAndSeekMode>();

                mode->setCameraTicket(gravityCamera);
            }
        }
    }

    return al::getScenePlayerHolder(curScene);
}

bool borderPullBackHook(WorldEndBorderKeeper* thisPtr) {

    bool isFirstStep = al::isFirstStep(thisPtr);

    if (isFirstStep) {
        if (GameModeManager::instance()->isModeAndActive(GameMode::HIDEANDSEEK)) {

            HideAndSeekMode* mode = GameModeManager::instance()->getMode<HideAndSeekMode>();

            if (mode->isUseGravity()) {
                killMainPlayer(thisPtr->mActor);
            }
        }
    }
    
    return isFirstStep;
}

float followDistHook() {
    switch(curSize){
        case NORMAL:
            return 893.f;
            break;
        case SMALL:
            return 270.f;
            break;
        case BIG:
            return 1750.f;
            break;
            break;
        case VERYBIG:
            return 3750.f;
            break;
    }
}

const char* offsetOverideHook(al::ByamlIter const& iter, char const* key) {
    switch(curSize){
        case SMALL:
             return "Y0.5m";
            break;
        default:
             return al::tryGetByamlKeyStringOrNULL(iter, key);
            break;
            break;
    }
}

PlayerConst* createPlayerConstHook(char const* suffix) {
    switch(curSize){
        case NORMAL:
            return PlayerFunction::createMarioConst(suffix);
            break;
        case SMALL: {
            PlayerConst* cons = PlayerFunction::createMarioConst(al::StringTmp<0x20>("Small%s", suffix).cstr());
            return cons;
            break;
        }
        case BIG: {
            PlayerConst* cons = PlayerFunction::createMarioConst(al::StringTmp<0x20>("Big%s", suffix).cstr());
            return cons;
            break;
        }
        case VERYBIG: {
            PlayerConst* cons = PlayerFunction::createMarioConst(al::StringTmp<0x20>("VeryBig%s", suffix).cstr());
            return cons;
            break;
        }
    }
}

// bool loadParamHook(float *output, al::ByamlIter const &iter, char const *key) {
//     bool result = al::tryGetByamlF32(output, iter, key);
//     if(isSmallMode)
//         *output = 0.2f;

//     return result;
// }

void effectHook(al::ActionEffectCtrl* effectController, char const* effectName) {

    if (curSize == PlayerSize::SMALL) {
        if(al::isEqualString(effectName, "RollingStart") || al::isEqualString(effectName, "Rolling") || al::isEqualString(effectName, "RollingStandUp") || al::isEqualString(effectName, "Jump") || al::isEqualString(effectName, "LandDownFall")|| al::isEqualString(effectName, "SpinCapStart") || al::isEqualString(effectName, "FlyingWaitR") || al::isEqualString(effectName, "StayR")|| al::isEqualString(effectName, "SpinGroundR")|| al::isEqualString(effectName, "StartSpinJumpR")|| al::isEqualString(effectName, "SpinJumpDownFallR")|| al::isEqualString(effectName, "Move")|| al::isEqualString(effectName, "Brake")) {
            al::tryDeleteEffect(effectController->mEffectKeeper, effectName);
            return;
        }
    }

    effectController->startAction(effectName);

}

void capVelScaleHook(al::LiveActor* hackCap, sead::Vector3f const& addition) {
    al::setVelocity(hackCap, addition * (scale * 0.8f));
}

void capReturnVelHook(HackCap *hackCap, sead::Vector3f const& addition) {
    if (curSize == PlayerSize::SMALL || curSize == PlayerSize::VERYBIG) {
        PlayerActorHakoniwa *pActor = (PlayerActorHakoniwa*)al::getPlayerActor(hackCap, 0);
        CapFunction::putOnCapPlayer(hackCap, pActor->mPlayerAnimator);
    } else {
        al::setVelocity(hackCap, addition);
    }
    
}

void spinFlowerHook(al::LiveActor* actor, float velocity) {
    al::addVelocityToGravity(actor, velocity * scale);
}

void sensorHook(al::LiveActor *actor, al::ActorInitInfo const &initInfo, char const *sensorName, uint typeEnum, float radius, ushort maxCount, sead::Vector3f const& position) {
    sead::Vector3f newPos = sead::Vector3f(position);
    if(position.y > 0)
        newPos.y = position.y * scale;

    al::addHitSensor(actor, initInfo, sensorName, typeEnum, radius, maxCount, newPos);
}

float fpHook() {
    return 300.0f * scale;
}

float fpScaleHook() {
    switch(curSize){
        case NORMAL:
            return 0.94f;
            break;
        case SMALL:
            return 31.3f;
            break;
        case BIG:
            return 0.24f;
            break;
            break;
        case VERYBIG:
            break;
    }
}
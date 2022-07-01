#include "container/seadSafeArray.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/StageScene/StageScene.h"
#include "gfx/seadColor.h"
#include "prim/seadSafeString.h"
#include "sead/math/seadVector.h"

class TimeFrameCap {
public:
    bool isFlying = false;
    sead::Vector3f position = sead::Vector3f::zero;
    sead::Vector3f rotation = sead::Vector3f::zero;
    sead::FixedSafeString<0x20> action;
};

class TimeFrame {
public:
    float colorFrame = 0;
    sead::Vector3f position = sead::Vector3f::zero;
    sead::Vector3f velocity = sead::Vector3f::zero;
    sead::Vector3f gravity = -1*sead::Vector3f::ey;
    sead::Quatf rotation;
    sead::FixedSafeString<0x20> animation;
    float animationFrame;
    TimeFrameCap capFrame;
};

class TimeContainer {
public:
    StageScene* stageSceneRef;
    bool isRewinding = false;
    bool isCaptured = false;
    bool is2D = false;
    bool isFirstStep = false;
    int sceneInvactiveTime = -1;
    int debugCheckFrame = 0;

    int rewindFrameDelay = 0;
    int rewindFrameDelayTarget = 0;

    int minTrailLength = 40;
    float minPushDistance = 20.f;
    float lastRecordColorFrame = 0;
    static const int maxFrames = 300;
    sead::PtrArray<TimeFrame> timeFrames;
};

TimeContainer& getTimeContainer();

void updateTimeStates(PlayerActorHakoniwa* p1);
sead::Color4f calcColorFrame(float colorFrame);

void pushNewFrame();
void emptyFrameInfo();
void isFramesEmpty();
void rewindFrame(PlayerActorHakoniwa* p1);

void startRewind(PlayerActorHakoniwa* p1);
void endRewind(PlayerActorHakoniwa* p1);
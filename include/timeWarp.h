#include "game/Player/PlayerActorHakoniwa.h"
#include "game/StageScene/StageScene.h"
#include "gfx/seadColor.h"
#include "prim/seadSafeString.h"
#include "sead/math/seadVector.h"

class timeFrame {
public:
    bool isEmptyFrame = true;
    float colorFrame = 0;
    sead::Vector3f position = { 0.f, 0.f, 0.f };
    sead::Vector3f velocity = { 0.f, 0.f, 0.f };
    sead::Vector3f gravity = { 0.f, -1.f, 0.f };
    sead::Quatf rotation;
    sead::FixedSafeString<0x20> animation;
    float animationFrame;
};

class timeContainer {
public:
    StageScene* stageSceneRef;
    bool isRewinding = false;
    int frameCount = 0;
    int debugCheckFrame = 0;
    int minTrailLength = 40;
    float minPushDistance = 20.f;
    sead::Vector3f lastRecordPosition;
    float lastRecordColorFrame = 0;

    int maxFrames = 200;
    timeFrame timeFrames[201];
};

timeContainer& getTimeContainer();

void updateTimeStates(PlayerActorHakoniwa* p1);
sead::Color4f calcColorFrame(float colorFrame);

void removeOldestFrame();
void pushNewFrame();
void isFramesEmpty();
void rewindFrame(PlayerActorHakoniwa* p1);

void startRewind(PlayerActorHakoniwa* p1);
void endRewind(PlayerActorHakoniwa* p1);
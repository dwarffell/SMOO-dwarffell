#include <stdint.h>
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
    sead::FixedSafeString<0x20> action;
    float actionFrame;
    TimeFrameCap capFrame;
};

class TimeContainer {
private:
    //Current State
    bool isRewinding = false;
    bool isCapture = false;
    bool isCaptureInvalid = false;
    bool is2D = false;
    int sceneInvactiveTime = -1; //Disable timewarp trails during scene transition while this frame counter is higher than -1

    //Timewarp Trail Array
    static const int maxFrames = 300;
    sead::PtrArray<TimeFrame> timeFrames;

    //Timewarp stats/settings
    int rewindFrameDelay = 0;
    int rewindFrameDelayTarget = 0; //How many extra frames to stall before rewinding a frame, used for debugging
    int minTrailLength = 40; //Minimum number of dots to rewind
    float minPushDistance = 20.f; //Minimum distance to move before rewinding

    //Trail color
    float colorFrame = 0.f; //Current color state, passed into calcColorFrame
    float colorFrameRate = 0.05f; //Rate that the color value increases/decreases when drawing/rewinding
    float colorFrameOffset = 0.f;
    float colorFrameOffsetRate = 0.07f;
    int dotBounceIndex = -1;

    //Cooldown information
    bool isCooldown = false;
    float cooldownCharge = 0.f;
    float cooldownRate = 0.25f;

    //Private functions
    void pushNewFrame(); //Adds new value to array
    void rewindFrame(PlayerActorHakoniwa* p1); //Pops newest array entry off and places the player there
    void updateHackCap(HackCap* cap, al::LiveActor* headModel);
    void startRewind(PlayerActorHakoniwa* p1); //Inits a rewind
    void endRewind(PlayerActorHakoniwa* p1); //Ends a rewind
    void emptyFrameInfo(); //Wipes the whole frame array
    void resetCooldown(); //Restarts the cooldown

public:
    //References
    StageScene* stageSceneRef;

    //Enter points
    void init();
    void updateTimeStates(PlayerActorHakoniwa* p1);

    //Getters
    TimeFrame* getTimeFrame(uint32_t index);
    int getTimeArraySize();
    float getColorFrame();
    float getCooldownTimer();
    int getRewindDelay(); //Returns the target for the rewind delay, 0 = none

    //Is
    bool isSceneActive(); //Checks if the scene inactive time is -1 and draws can happen
    bool isRewind();
    bool isOnCooldown();
    bool isInvalidCapture(const char* curName);

    //Setters
    void setRewindDelay(int index); //Modifies the rewind delay based on the amount of the index
    void setInactiveTimer(int time); //Sets the scene invctivity timer
    void setTimeFramesEmpty();

    //Calcs
    sead::Color4f calcColorFrame(float colorFrame, int dotIndex); //Calculates the sead::Color4f of the colorFrame
    sead::Vector3f calcDotTrans(sead::Vector3f position, int dotIndex);
};

TimeContainer& getTimeContainer();
#pragma once

#include "al/camera/CameraTicket.h"
#include "container/seadPtrArray.h"
#include "container/seadSafeArray.h"
#include "game/Player/PlayerActorBase.h"
#include "game/StageScene/StageScene.h"
#include "layouts/FreezeTagIcon.h"
#include "math/seadVector.h"
#include "server/freeze/FreezePlayerBlock.h"
#include "server/freeze/FreezeTagScore.hpp"
#include "server/gamemode/GameModeBase.hpp"
#include "server/gamemode/GameModeConfigMenu.hpp"
#include "server/gamemode/GameModeInfoBase.hpp"
#include "server/gamemode/GameModeTimer.hpp"
#include "server/hns/HideAndSeekConfigMenu.hpp"
#include <math.h>

enum FreezeState {
    ALIVE = 0,
    FREEZE = 1
};

struct FreezeTagInfo : GameModeInfoBase {
    FreezeTagInfo() { mMode = GameMode::FREEZETAG; }
    bool mIsPlayerRunner = true;
    float mFreezeIconSize = 0.f;
    FreezeState mIsPlayerFreeze = FreezeState::ALIVE;
    FreezeTagScore mPlayerTagScore;

    sead::PtrArray<PuppetInfo> mRunnerPlayers;
    sead::PtrArray<PuppetInfo> mChaserPlayers;
};

class FreezeTagMode : public GameModeBase {
public:
    FreezeTagMode(const char* name);

    void init(GameModeInitInfo const& info) override;

    virtual void begin() override;
    virtual void update() override;
    virtual void end() override;

    bool isPlayerRunner() const { return mInfo->mIsPlayerRunner; };
    bool isPlayerFreeze() const { return mInfo->mIsPlayerFreeze; };

    void setWipeHolder(al::WipeHolder* wipe) { mWipeHolder = wipe; };
    bool tryStartRecoveryEvent(bool isMakeFrozen, bool isResetScore);
    bool tryEndRecoveryEvent();

    bool trySetPlayerRunnerState(FreezeState state);

    void updateSpectateCam(PlayerActorBase* playerBase);
    void setCameraTicket(al::CameraTicket* ticket) { mTicket = ticket; }

private:
    // Recovery event info
    int mRecoveryEventFrames = 0;
    const int mRecoveryEventLength = 60; // Length of recovery event in frames
    sead::Vector3f mRecoverySafetyPoint = sead::Vector3f::zero;

    FreezeTagIcon* mModeLayout = nullptr;
    FreezeTagInfo* mInfo = nullptr;
    FreezePlayerBlock* mMainPlayerIceBlock = nullptr;
    al::WipeHolder* mWipeHolder = nullptr; // Pointer set by setWipeHolder on first step of hakoniwaSequence hook

    float mInvulnTime = 0.0f;

    // Spectate camera ticket and target information
    al::CameraTicket* mTicket = nullptr;
    int mPrevSpectateIndex = -2;
    int mSpectateIndex = -1;
};
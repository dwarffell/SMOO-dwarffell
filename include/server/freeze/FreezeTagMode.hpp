#pragma once

#include "al/camera/CameraTicket.h"
#include "container/seadPtrArray.h"
#include "container/seadSafeArray.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/StageScene/StageScene.h"
#include "layouts/FreezeTagIcon.h"
#include "math/seadVector.h"
#include "puppets/PuppetInfo.h"
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

    bool mIsRound = false;
    FreezeTagScore mPlayerTagScore;
    GameTime mRoundTimer;

    sead::PtrArray<PuppetInfo> mRunnerPlayers;
    sead::PtrArray<PuppetInfo> mChaserPlayers;

    bool mIsDebugMode = false;
};

enum FreezeUpdateType : u8 {
    PLAYER                 = 1 << 0,
    ROUNDSTART             = 1 << 1,
    ROUNDCANCEL            = 1 << 2,
    FALLOFF                = 1 << 3
};

struct PACKED FreezeTagPacket : Packet {
    FreezeTagPacket() : Packet() { this->mType = PacketType::GAMEMODEINF; mPacketSize = sizeof(FreezeTagPacket) - sizeof(Packet);};
    FreezeUpdateType updateType;
    bool isRunner = false;
    bool isFreeze = false;
    uint16_t score = 0;
};

class FreezeTagMode : public GameModeBase {
public:
    FreezeTagMode(const char* name);

    void init(GameModeInitInfo const& info) override;

    virtual void begin() override;
    virtual void update() override;
    virtual void end() override;

    void pause() override;
    void unpause() override;

    bool isUseNormalUI() const override { return false; }

    void processPacket(Packet* packet) override;
    Packet* createPacket() override;
    void sendFreezePacket(FreezeUpdateType updateType);

    void startRound(int roundMinutes);
    void endRound(bool isAbort);

    bool isScoreEventsEnabled() const { return mIsScoreEventsValid; };
    bool isPlayerRunner() const { return mInfo->mIsPlayerRunner; };
    bool isPlayerFreeze() const { return mInfo->mIsPlayerFreeze; };
    bool isEndgameActive() {return mIsEndgameActive;}
    bool isPlayerLastSurvivor(PuppetInfo* changingPuppet);
    bool isAllRunnerFrozen(PuppetInfo* changingPuppet);

    PlayerActorHakoniwa* getPlayerActorHakoniwa();
    uint16_t getScore() { return mInfo->mPlayerTagScore.mScore; }

    void setWipeHolder(al::WipeHolder* wipe) { mWipeHolder = wipe; };
    bool tryStartRecoveryEvent(bool isEndgame);
    bool tryEndRecoveryEvent();
    void warpToRecoveryPoint(al::LiveActor* actor);

    void tryStartEndgameEvent();

    bool trySetPlayerRunnerState(FreezeState state);

    void tryScoreEvent(FreezeTagPacket* incomingPacket, PuppetInfo* sourcePuppet);

    void updateSpectateCam(PlayerActorBase* playerBase);
    void setCameraTicket(al::CameraTicket* ticket) { mTicket = ticket; }

private:
    const int mRoundLength = 10; // Length of rounds in minutes

    FreezeUpdateType mNextUpdateType = FreezeUpdateType::PLAYER;
    GameModeTimer* mModeTimer = nullptr;
    FreezeTagIcon* mModeLayout = nullptr;
    FreezeTagInfo* mInfo = nullptr;
    FreezePlayerBlock* mMainPlayerIceBlock = nullptr;
    al::WipeHolder* mWipeHolder = nullptr; // Pointer set by setWipeHolder on first step of hakoniwaSequence hook

    // Recovery event info
    int mRecoveryEventFrames = 0;
    const int mRecoveryEventLength = 60; // Length of recovery event in frames
    sead::Vector3f mRecoverySafetyPoint = sead::Vector3f::zero;

    // Endgame info
    bool mIsEndgameActive = false;
    float mEndgameTimer = -1.f;

    float mInvulnTime = 0.0f;
    bool mIsScoreEventsValid = false;

    // Spectate camera ticket and target information
    al::CameraTicket* mTicket = nullptr;
    int mPrevSpectateIndex = -2;
    int mSpectateIndex = -1;
};
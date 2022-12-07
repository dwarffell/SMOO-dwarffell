#pragma once

#include "al/camera/CameraTicket.h"
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
    bool mIsPlayerRunner = false;
    FreezeState mIsPlayerFreeze = FreezeState::ALIVE;
    FreezeTagScore mPlayerTagScore;
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

    bool trySetPlayerRunnerState(FreezeState state);

    void setCameraTicket(al::CameraTicket* ticket) { mTicket = ticket; }

private:
    float mInvulnTime = 0.0f;
    HideAndSeekIcon* mModeLayout = nullptr;
    FreezeTagInfo* mInfo = nullptr;
    al::CameraTicket* mTicket = nullptr;

    FreezePlayerBlock* mMainPlayerIceBlock;
};
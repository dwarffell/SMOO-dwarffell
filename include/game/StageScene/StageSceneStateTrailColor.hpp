#pragma once

#include "al/layout/LayoutInitInfo.h"
#include "al/message/IUseMessageSystem.h"
#include "al/message/MessageSystem.h"
#include "al/nerve/HostStateBase.h"
#include "al/scene/Scene.h"
#include "al/util/NerveUtil.h"
#include "game/Input/InputSeparator.h"
#include "game/Layouts/CommonVerticalList.h"
#include "game/Layouts/SimpleLayoutMenu.h"
#include "rs/util/InputUtil.h"

#include "al/util.hpp"

#include "game/GameData/GameDataHolder.h"

#include "logger.hpp"
#include "server/gamemode/GameModeConfigMenu.hpp"

class FooterParts;

class StageSceneStateTrailColor : public al::HostStateBase<al::Scene>, public al::IUseMessageSystem {
public:
    StageSceneStateTrailColor(const char*, al::Scene*, const al::LayoutInitInfo&, FooterParts*,
                             GameDataHolder*, bool);

    virtual al::MessageSystem* getMessageSystem(void) const override;
    virtual void init(void) override;
    virtual void appear(void) override;
    virtual void kill(void) override;

    void exeMainMenu();

private:
    al::MessageSystem* mMsgSystem = nullptr;

    FooterParts* mFooterParts = nullptr;
    GameDataHolder* mGameDataHolder = nullptr;

    InputSeparator* mInput = nullptr;
    bool decided = false;
    int holdFrames = 0;

    SimpleLayoutMenu* mCurrentMenu = nullptr;
    CommonVerticalList* mCurrentList = nullptr;
    // Root Page, contains buttons for gamemode config, server reconnecting, and server ip address
    // changing
    SimpleLayoutMenu* mMainOptions = nullptr;
    CommonVerticalList* mMainOptionsList = nullptr;

    static const char16_t* mainMenuItems[];
};

namespace {
NERVE_HEADER(StageSceneStateTrailColor, MainMenu)
}
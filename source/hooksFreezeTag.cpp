#include "al/util.hpp"
#include "al/util/NerveUtil.h"
#include "al/util/SensorUtil.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"

#include "rs/util/InputUtil.h"
#include "server/Client.hpp"
#include "server/freeze/FreezeTagMode.hpp"
#include "server/gamemode/GameModeManager.hpp"

#include "al/nerve/Nerve.h"
#include "rs/util.hpp"

bool freezeDisableActor(al::IUseNerve const* thisPtr, al::Nerve const* nrv)
{
    if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return true;

    return al::isNerve(thisPtr, nrv);
}

bool freezeDisableMsgDisregard(al::SensorMsg const* msg)
{
    if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return true;

    return al::isMsgPlayerDisregard(msg);
}

bool freezeDisableBazooka(IUsePlayerHack* param_1)
{
    if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return false;

    return rs::isHoldHackJump(param_1);
}

bool freezeDisableLoadZone(al::LiveActor const* actor)
{
    if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return true;

    return rs::isInvalidChangeStage(actor);
}

bool freezeDeathArea(al::LiveActor const* player)
{
    // If player isn't actively playing freeze tag, perform normal functionality
    if (!GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG))
        return al::isInDeathArea(player);

    // If player is in a death area but in Freeze Tag mode, start a recovery event
    if (al::isInAreaObj(player, "DeathArea")) {
        FreezeTagMode* mode = GameModeManager::instance()->getMode<FreezeTagMode>();
        if (!mode->isEndgameActive())
            mode->tryStartRecoveryEvent(false);
    }

    return false;
}
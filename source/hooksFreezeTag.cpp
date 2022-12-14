#include "al/util.hpp"
#include "al/util/NerveUtil.h"
#include "al/util/SensorUtil.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "server/Client.hpp"
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

bool freezeDisableLoadZone(al::LiveActor const* actor)
{
    if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG)) {
        PlayerActorBase* playerBase = rs::getPlayerActor(Client::instance()->getCurrentScene());
        bool isYukimaru = !playerBase->getPlayerInfo();
        bool isPlayerInArea = al::isInAreaObj(playerBase, "ChangeStageArea");

        if (!isYukimaru && actor == playerBase && isPlayerInArea) {
            Logger::log("Attempting player recovery from load zone\n");
            PlayerActorHakoniwa* player = (PlayerActorHakoniwa*)playerBase;
            // player->mPlayerRecoverySafetyPoint->noticeDangerousPoint(al::getTrans(player), false);
        }

        return true;
    }

    return rs::isInvalidChangeStage(actor);
}
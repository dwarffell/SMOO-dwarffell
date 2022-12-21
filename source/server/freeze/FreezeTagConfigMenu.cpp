#include "server/freeze/FreezeTagConfigMenu.hpp"
#include <cmath>
#include <stdint.h>
#include "logger.hpp"
#include "server/gamemode/GameModeManager.hpp"
#include "server/freeze/FreezeTagMode.hpp"
#include "server/Client.hpp"

FreezeTagConfigMenu::FreezeTagConfigMenu() : GameModeConfigMenu() {}

void FreezeTagConfigMenu::initMenu(const al::LayoutInitInfo &initInfo) {
    
}

const sead::WFixedSafeString<0x200> *FreezeTagConfigMenu::getStringData() {
    sead::SafeArray<sead::WFixedSafeString<0x200>, mItemCount>* gamemodeConfigOptions =
        new sead::SafeArray<sead::WFixedSafeString<0x200>, mItemCount>();

    gamemodeConfigOptions->mBuffer[0].copy(u"Set Score");
    gamemodeConfigOptions->mBuffer[1].copy(u"Enable Debug Mode");
    gamemodeConfigOptions->mBuffer[2].copy(u"Disable Debug Mode");

    return gamemodeConfigOptions->mBuffer;
}

bool FreezeTagConfigMenu::updateMenu(int selectIndex) {

    FreezeTagInfo *curMode = GameModeManager::instance()->getInfo<FreezeTagInfo>();

    Logger::log("Updating freeze tag menu\n");

    if (!curMode) {
        Logger::log("Unable to Load Mode info!\n");
        return true;   
    }
    
    switch (selectIndex) {
        case 0: {
            if (GameModeManager::instance()->isModeAndActive(GameMode::FREEZETAG)) {
                uint16_t newScore = Client::openKeyboardFreezeTag();
                if(newScore != uint16_t(-1))
                    curMode->mPlayerTagScore.mScore = newScore;
            }
            return true;
        }
        case 1: {
            if (GameModeManager::instance()->isMode(GameMode::FREEZETAG)) {
                curMode->mIsDebugMode = true;
            }
            return true;
        }
        case 2: {
            if (GameModeManager::instance()->isMode(GameMode::FREEZETAG)) {
                curMode->mIsDebugMode = false;
            }
            return true;
        }
        default:
            Logger::log("Failed to interpret Index!\n");
            return false;
    }
    
}
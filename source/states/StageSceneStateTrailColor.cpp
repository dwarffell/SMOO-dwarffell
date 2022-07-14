#include "game/StageScene/StageSceneStateTrailColor.hpp"
#include "game/SaveData/SaveDataAccessFunction.h"
#include "timeWarp.h"

const char16_t* StageSceneStateTrailColor::mainMenuItems[] = {
    u"Keybindings:",
    u"Rewind DualButtonRHandheldButtonRFullKeyButtonRLeftButtonRRightButtonR",
    u"Rewind DualButtonLHandheldButtonLFullKeyButtonLLeftButtonLRightButtonL",
    u"Rewind ~DualStickLHandheldStickLFullKeyStickLLeftStickLRightStickL",
    u"Rainbow Flag",
    u"Lesbian Flag",
    u"Gay Male Flag",
    u"Bisexual Flag",
    u"Pansexual Flag",
    u"Polysexual Flag",
    u"Asexual Flag",
    u"Aromantic Flag",
    u"Transgender Flag",
    u"Non-Binary Flag",
    u"Intersex Flag"
};

enum mainMenuValues
{
    NULL1,
    KEYBINDBUMPR,
    KEYBINDBUMPL,
    KEYBINDSTICKL,
    RAINBOW,
    LESBIAN,
    GAY,
    BISEXUAL,
    PANSEXUAL,
    POLYSEXUAL,
    ASEXUAL,
    AROMANTIC,
    TRANSGENDER,
    ENBY,
    INTERSEX
};

StageSceneStateTrailColor::StageSceneStateTrailColor(const char* name, al::Scene* scene,
                                                   const al::LayoutInitInfo& initInfo,
                                                   FooterParts* footerParts,
                                                   GameDataHolder* dataHolder, bool)
    : al::HostStateBase<al::Scene>(name, scene) {
    constexpr const int mainMenuItemCount =
        sizeof(StageSceneStateTrailColor::mainMenuItems) / sizeof(char16_t*);

    mFooterParts = footerParts;
    mGameDataHolder = dataHolder;

    mMsgSystem = initInfo.getMessageSystem();

    mInput = new InputSeparator(mHost, true);

    // page 0 menu
    mMainOptions = new SimpleLayoutMenu("TrialColor", "OptionSelect", initInfo, 0, false);
    mMainOptionsList = new CommonVerticalList(mMainOptions, initInfo, true);

    al::setPaneString(mMainOptions, "TxtOption", u"Time Travel Settings", 0);

    mMainOptionsList->initDataNoResetSelected(mainMenuItemCount);

    sead::SafeArray<sead::WFixedSafeString<0x200>, mainMenuItemCount>* mainMenuOptions =
        new sead::SafeArray<sead::WFixedSafeString<0x200>, mainMenuItemCount>();

    int i = 0;
    for (const char16_t* item : mainMenuItems) {
        mainMenuOptions->mBuffer[i++].copy(item);
    }

    mMainOptionsList->addStringData(mainMenuOptions->mBuffer, "TxtContent");

    mCurrentList = mMainOptionsList;
    mCurrentMenu = mMainOptions;
}

void StageSceneStateTrailColor::init() {
    initNerve(&nrvStageSceneStateTrailColorMainMenu, 0);
}

void StageSceneStateTrailColor::appear(void) {
    mCurrentMenu->startAppear("Appear");
    al::NerveStateBase::appear();
}

void StageSceneStateTrailColor::kill(void) {
    SaveDataAccessFunction::startSaveDataWrite(mGameDataHolder);
    mCurrentMenu->startEnd("End");
    al::NerveStateBase::kill();
}

al::MessageSystem* StageSceneStateTrailColor::getMessageSystem(void) const {
    return mMsgSystem;
}

void StageSceneStateTrailColor::exeMainMenu() {
    if (al::isFirstStep(this)) {
        mInput->reset();
        mCurrentList->activate();
        mCurrentList->appearCursor();
        decided = false;
    }

    mInput->update();
    mCurrentList->update();

    if(mCurrentList->mCurSelected == 0 && mInput->isTriggerUiUp()) mCurrentList->jumpBottom();
    if(mCurrentList->mCurSelected == (sizeof(mainMenuItems)/sizeof(mainMenuItems[0]))-1 && mInput->isTriggerUiDown()) mCurrentList->jumpTop();

    if (mInput->isTriggerUiUp() || (mInput->isHoldUiUp() && holdFrames > 20)) mCurrentList->up();

    if (mInput->isTriggerUiDown() || (mInput->isHoldUiDown() && holdFrames > 20)) mCurrentList->down();

    if(mInput->isHoldUiDown() || mInput->isHoldUiUp()) holdFrames++;
    else holdFrames = 0;

    if (rs::isTriggerUiCancel(mHost)) {
        kill();
    }

    if (rs::isTriggerUiDecide(mHost)) {
        al::startHitReaction(mCurrentMenu, "決定", 0);
        mCurrentList->endCursor();
        mCurrentList->decide();
        decided = true;
    }

    if (decided && mCurrentList->isDecideEnd()) {
        TimeContainer& container = getTimeContainer();

        switch(mCurrentList->mCurSelected){
            case(mainMenuValues::NULL1):
                break;
            case(mainMenuValues::KEYBINDBUMPR):
                container.setControlBinding(0);
                break;
            case(mainMenuValues::KEYBINDBUMPL):
                container.setControlBinding(1);
                break;
            case(mainMenuValues::KEYBINDSTICKL):
                container.setControlBinding(2);
                break;
            case(mainMenuValues::RAINBOW):
                container.setCurrentColorPattern(mainMenuValues::RAINBOW);
                break;
            case(mainMenuValues::LESBIAN):
                container.setCurrentColorPattern(mainMenuValues::LESBIAN);
                break;
            case(mainMenuValues::GAY):
                container.setCurrentColorPattern(mainMenuValues::GAY);
                break;
            case(mainMenuValues::BISEXUAL):
                container.setCurrentColorPattern(mainMenuValues::BISEXUAL);
                break;
            case(mainMenuValues::PANSEXUAL):
                container.setCurrentColorPattern(mainMenuValues::PANSEXUAL);
                break;
            case(mainMenuValues::POLYSEXUAL):
                container.setCurrentColorPattern(mainMenuValues::POLYSEXUAL);
                break;
            case(mainMenuValues::ASEXUAL):
                container.setCurrentColorPattern(mainMenuValues::ASEXUAL);
                break;
            case(mainMenuValues::AROMANTIC):
                container.setCurrentColorPattern(mainMenuValues::AROMANTIC);
                break;
            case(mainMenuValues::TRANSGENDER):
                container.setCurrentColorPattern(mainMenuValues::TRANSGENDER);
                break;
            case(mainMenuValues::ENBY):
                container.setCurrentColorPattern(mainMenuValues::ENBY);
                break;
            case(mainMenuValues::INTERSEX):
                container.setCurrentColorPattern(mainMenuValues::INTERSEX);
                break;
        }
        
        al::setNerve(this, &nrvStageSceneStateTrailColorMainMenu);  // reset
    }
}

namespace {
NERVE_IMPL(StageSceneStateTrailColor, MainMenu)
}
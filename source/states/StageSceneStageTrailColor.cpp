#include "game/StageScene/StageSceneStateTrailColor.hpp"
#include "timeWarp.h"

const char16_t* StageSceneStateTrailColor::mainMenuItems[] = {
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

    al::setPaneString(mMainOptions, "TxtOption", u"Trail Color", 0);

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
        container.setCurrentColorPattern(mCurrentList->mCurSelected);
        al::setNerve(this, &nrvStageSceneStateTrailColorMainMenu);  // reset
    }
}

namespace {
NERVE_IMPL(StageSceneStateTrailColor, MainMenu)
}
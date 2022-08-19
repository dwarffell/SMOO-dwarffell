#include "tether.h"
#include "al/util/ControllerUtil.h"

PlayerTether& getTether()
{
    static PlayerTether i;
    return i;
}

void PlayerTether::tick(StageScene* scene, PlayerActorHakoniwa* p1)
{
    mSceneFrames++;

    if (!mIsSceneAlive)
        return;

    float closePuppetDistance = -1.f;
    int closePuppetIndex = -1;
    int newPCount = Client::getConnectCount();

    if (newPCount != mPlayerCount) {
        mPlayerCount = newPCount;
        mSceneFrames = -180;
    }

    // Puppet finder
    for (size_t i = 0; i < Client::getPuppetHolder()->getSize(); i++) {
        PuppetInfo* curInfo = Client::getPuppetInfo(i);
        float dist = al::calcDistance(p1, curInfo->playerPos);
        bool isInSameScene = false;
        bool isAlive = true;

        if (mIsSceneAlive) {
            isInSameScene = al::isEqualString(mSceneName, curInfo->stageName);
            isAlive = !al::isEqualSubString(PlayerAnims::FindStr(curInfo->curAnim), "Dead");
        }

        if ((dist < closePuppetDistance || closePuppetDistance == -1.f) && isInSameScene && isAlive) {
            closePuppetIndex = i;
            closePuppetDistance = dist;
            mClosePup = curInfo;
        }
    }

    // Player pulling
    if (closePuppetDistance >= mPullDistanceMin && closePuppetIndex != -1 && mClosePup && mSceneFrames > 90) {
        sead::Vector3f target = Client::getPuppetInfo(closePuppetIndex)->playerPos;
        sead::Vector3f* playerPos = al::getTransPtr(p1);
        sead::Vector3f direction = target - *playerPos;

        al::normalize(&direction);

        playerPos->add((direction * mPullPower) * ((al::calcDistance(p1, target) - mPullDistanceMin) / mPullPowerRate));
    }

    // Player warping
    if (mClosePup)
        if (!mClosePup->isInSameStage && mSceneFrames > 150) {
            mIsSceneChangeFromPuppet = true;
            GameDataHolderAccessor accessor = scene->mHolder;
            ChangeStageInfo info(accessor.mData, "start", mClosePup->stageName, false, mClosePup->scenarioNo, ChangeStageInfo::SubScenarioType::UNK);
            GameDataFunction::tryChangeNextStage(accessor, &info);
        }

    // Player Flicking
    if (al::isPadTriggerPressLeftStick(-1)) {
        p1->startDemoPuppetable();
        p1->mPlayerAnimator->startAnim("HipDropLand");
    } else if (al::isPadTriggerPressRightStick(-1)) {
        p1->endDemoPuppetable();
    }
}

void PlayerTether::setSceneAlive(const char* stageName, sead::Vector3f* playerPos)
{
    mIsSceneAlive = true;
    mSceneName = stageName;
    mPlayerPos = playerPos;
    mSceneFrames = 0;

    if (mIsSceneChangeFromPuppet) {
        *mPlayerPos = mClosePup->playerPos;
        mIsSceneChangeFromPuppet = false;
    }

    return;
}

void PlayerTether::setSceneKilled()
{
    mIsSceneAlive = false;
    mSceneFrames = 0;
}
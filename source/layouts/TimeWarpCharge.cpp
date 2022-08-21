#include "layouts/TimeWarpCharge.hpp"
#include "al/layout/LayoutActor.h"
#include "al/util.hpp"
#include "al/util/LayoutUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/MathUtil.h"
#include "al/util/NerveUtil.h"
#include "game/WorldList/WorldResourceLoader.h"
#include "gfx/seadColor.h"
#include "logger.hpp"
#include "math/seadMathCalcCommon.h"
#include "prim/seadSafeString.h"
#include "server/DeltaTime.hpp"

TimeWarpCharge::TimeWarpCharge(const al::LayoutInitInfo& initInfo)
    : al::LayoutActor("TimeWarpCharge")
{
    al::initLayoutActor(this, initInfo, "TimeWarpCharge", nullptr);
    initNerve(&nrvTimeWarpChargeAppear, 0);
    // al::setPaneLocalScale(this, "All", { 5.0f, 5.0f });
    appear();
}

void TimeWarpCharge::exeAppear()
{
    if (al::isFirstStep(this)) {
        al::startAction(this, "Appear", nullptr);
    }

    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &nrvTimeWarpChargeWait);
    }
}

void TimeWarpCharge::exeWait()
{
    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &nrvTimeWarpChargeDecrease);
    }
}

void TimeWarpCharge::exeDecrease()
{
    sead::Color4u8 color = { 255, 255, 0, 255 };

    color.r = 255 - sead::MathCalcCommon<int>::ceil(mFill * 255);
    color.g = sead::MathCalcCommon<int>::ceil(mFill * 255);

    sead::WFormatFixedSafeString<0x40> string(u"Time Power %.01f", mFill * 100);

    al::setPaneString(this, "TxtPower", string.cstr(), 0);

    // al::startFreezeAction(this, "Decrease", 60.f * (1.0f - actual), "State");
    al::setPaneLocalScale(this, "PicFill", { mFill, 1.f });
    al::setPaneVtxColor(this, "PicFill", color);

    al::setPaneVtxColor(this, "PicEmpty", mEmptyColor);

    // if (mProgression > 1.0f) {
    //     al::setNerve(this, &nrvSpeedbootLoadEnd);
    // }
}

void TimeWarpCharge::exeEnd()
{
    if (al::isFirstStep(this)) {
        al::startAction(this, "End", nullptr);
    }

    if (al::isActionEnd(this, nullptr)) {
        kill();
    }
}

namespace {
NERVE_IMPL(TimeWarpCharge, Appear)
NERVE_IMPL(TimeWarpCharge, Wait)
NERVE_IMPL(TimeWarpCharge, Decrease)
NERVE_IMPL(TimeWarpCharge, End)
} // namespace
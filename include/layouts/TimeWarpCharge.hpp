#include "al/layout/LayoutActor.h"
#include "al/layout/LayoutInitInfo.h"
#include "al/util/NerveUtil.h"
#include "gfx/seadColor.h"

class TimeWarpCharge : public al::LayoutActor {
public:
    TimeWarpCharge(const al::LayoutInitInfo& initInfo);

    void exeAppear();
    void exeWait();
    void exeDecrease();
    void exeEnd();

    float mFill = 0.5f;
    sead::Color4u8 mEmptyColor;
};

namespace {
NERVE_HEADER(TimeWarpCharge, Appear)
NERVE_HEADER(TimeWarpCharge, Wait)
NERVE_HEADER(TimeWarpCharge, Decrease)
NERVE_HEADER(TimeWarpCharge, End)
}
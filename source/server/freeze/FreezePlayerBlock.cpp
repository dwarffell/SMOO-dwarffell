#include "server/freeze/FreezePlayerBlock.h"
#include "al/util.hpp"
#include "al/util/LiveActorUtil.h"
#include "math/seadVector.h"
#include "rs/util.hpp"

FreezePlayerBlock::FreezePlayerBlock(const char* name)
    : al::LiveActor(name)
{
}

void FreezePlayerBlock::init(al::ActorInitInfo const& info)
{
    al::initActorWithArchiveName(this, info, "BlockBrick", nullptr);
    al::initNerve(this, &nrvFreezePlayerBlockWait, 0);
    this->makeActorAlive();

    al::setScaleAll(this, 4.f);
    al::invalidateClipping(this);

    makeActorDead();
}

void FreezePlayerBlock::initAfterPlacement(void)
{
    return;
}

bool FreezePlayerBlock::receiveMsg(const al::SensorMsg* message, al::HitSensor* source, al::HitSensor* target)
{
    return false;
}

void FreezePlayerBlock::attackSensor(al::HitSensor* target, al::HitSensor* source)
{
    return;
}

void FreezePlayerBlock::control(void) { return; }

void FreezePlayerBlock::exeWait()
{
    return;
}

namespace {
    NERVE_IMPL(FreezePlayerBlock, Wait)
} // namespace
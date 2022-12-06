#pragma once

#include "al/LiveActor/LiveActor.h"
#include "al/util.hpp"
#include "al/sensor/SensorMsg.h"
#include "al/util/NerveUtil.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "rs/util/SensorUtil.h"
#include "logger.hpp"


class FreezePlayerBlock : public al::LiveActor
{
public:
    FreezePlayerBlock(const char *name);
    virtual void init(al::ActorInitInfo const &) override;
    virtual void initAfterPlacement(void) override;
    virtual bool receiveMsg(const al::SensorMsg *message, al::HitSensor *source, al::HitSensor *target) override;
    virtual void attackSensor(al::HitSensor *source, al::HitSensor *target) override;
    virtual void control(void) override;

    void exeWait();
};

namespace
{
    NERVE_HEADER(FreezePlayerBlock, Wait)
}

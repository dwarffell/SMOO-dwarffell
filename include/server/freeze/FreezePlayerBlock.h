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
    void init(al::ActorInitInfo const &) override;
    void initAfterPlacement(void) override;
    bool receiveMsg(const al::SensorMsg *message, al::HitSensor *source, al::HitSensor *target) override;
    void attackSensor(al::HitSensor *source, al::HitSensor *target) override;
    void control(void) override;
    void appear() override;

    void end();

    void exeAppear();
    void exeWait();
    void exeDisappear();
    void exeDead();
};

namespace
{
    NERVE_HEADER(FreezePlayerBlock, Appear)
    NERVE_HEADER(FreezePlayerBlock, Wait)
    NERVE_HEADER(FreezePlayerBlock, Disappear)
    NERVE_HEADER(FreezePlayerBlock, Dead)
}

#include "server/freeze/FreezeTagScore.hpp"

void FreezeTagScore::addScore(int add, const char* description)
{
    mScore += add;
    mIcon->queueScoreEvent(add, description);
    Client::sendFreezeInfPacket();
};
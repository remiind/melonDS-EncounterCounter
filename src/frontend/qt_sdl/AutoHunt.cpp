#include "AutoHunt.h"

AutoHunt::AutoHunt(uint32_t seed) : rng(seed) {}

void AutoHunt::restart()
{
    frame = 0;
    pressLeft = 0;
    nextPress = std::uniform_int_distribution<uint32_t>(0, kStartDelayMax)(rng);
}

AutoHunt::Action AutoHunt::toggle(uint32_t encounterSeq)
{
    if (on)
    {
        stop(StopReason::ByUser);
        return Action::Stopped;
    }
    on = true;
    reason = StopReason::None;
    seenSeq = encounterSeq;
    resetCount = 0;
    timeouts = 0;
    restart();
    return Action::Reset;
}

void AutoHunt::stop(StopReason why)
{
    on = false;
    reason = why;
    pressLeft = 0;
}

uint32_t AutoHunt::keysForFrame()
{
    if (!on)
        return 0;
    if (pressLeft == 0 && frame >= nextPress)
    {
        pressLeft = kPressFrames;
        nextPress = frame + kPressFrames + std::uniform_int_distribution<uint32_t>(kGapMin, kGapMax)(rng);
    }
    if (pressLeft == 0)
        return 0;
    pressLeft--;
    return 1u;   // A
}

AutoHunt::Action AutoHunt::afterFrame(uint32_t encounterSeq, bool lastShiny)
{
    if (!on)
        return Action::None;
    frame++;
    if (encounterSeq != seenSeq)
    {
        seenSeq = encounterSeq;
        timeouts = 0;
        if (lastShiny)
        {
            stop(StopReason::Shiny);
            return Action::Stopped;
        }
        resetCount++;
        restart();
        return Action::Reset;
    }
    if (frame >= (uint32_t)kTimeoutFrames)
    {
        if (++timeouts >= kMaxTimeouts)
        {
            stop(StopReason::NoBattle);
            return Action::Stopped;
        }
        restart();
        return Action::Reset;
    }
    return Action::None;
}

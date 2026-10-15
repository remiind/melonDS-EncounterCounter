#ifndef AUTOHUNT_H
#define AUTOHUNT_H

// EncounterCounter: automatic soft-reset hunt for static encounters
// (Pokémon Diamond / Pearl / Platinum / HeartGold / SoulSilver).
//
// The player saves the game in front of the Pokémon (legendary, gift ...).
// Per emulator window, toggled by a hotkey:
//   1. reset the console, wait a random number of frames
//   2. press A at random intervals: intro, title, "Continue", talk to the
//      Pokémon, dialogue - until the PokéCounter tracker reports the wild
//      Pokémon of the battle
//   3. shiny -> stop (the shiny auto-pause holds the window), else back to 1
// The random waits give every reset its own start delay and with it its own
// RNG seed, like a person soft-resetting by hand.
//
// Pure logic without melonDS/Qt, driven once per emulated frame by EmuThread.

#include <cstdint>
#include <random>

class AutoHunt
{
public:
    enum class Action
    {
        None,
        Reset,      // reset the console now
        Stopped,    // the hunt ended (see stopReason())
    };

    enum class StopReason
    {
        None,
        ByUser,
        Shiny,
        NoBattle,   // several resets in a row without a battle
    };

    // Frames (60 per emulated second)
    static constexpr int kStartDelayMax = 240;     // random wait after a reset
    static constexpr int kPressFrames = 3;         // A held this long
    static constexpr int kGapMin = 10;             // pause between presses
    static constexpr int kGapMax = 40;
    static constexpr int kTimeoutFrames = 120 * 60;   // no battle after 2 min -> reset
    static constexpr int kMaxTimeouts = 3;            // ... three times in a row -> stop

    explicit AutoHunt(uint32_t seed = std::random_device{}());

    bool active() const { return on; }

    // Hotkey: starts or stops the hunt. encounterSeq = current
    // PokeCounter::encounterSeq(), so an earlier encounter is not taken for
    // the first result. Starting returns Action::Reset (begin from a reset).
    Action toggle(uint32_t encounterSeq);
    void stop(StopReason why);

    // Before the frame: keys to press (DS key bits, 1 = pressed; bit 0 = A).
    uint32_t keysForFrame();

    // After the frame: the tracker's encounter counter and whether the last
    // reported encounter was shiny.
    Action afterFrame(uint32_t encounterSeq, bool lastShiny);

    uint32_t resets() const { return resetCount; }
    StopReason stopReason() const { return reason; }

private:
    void restart();

    bool on = false;
    std::mt19937 rng;
    uint32_t frame = 0;          // since the last reset
    uint32_t nextPress = 0;      // frame of the next A press
    uint32_t pressLeft = 0;      // frames A is still held
    uint32_t seenSeq = 0;
    uint32_t resetCount = 0;
    int timeouts = 0;
    StopReason reason = StopReason::None;
};

#endif // AUTOHUNT_H

#pragma once

#include <SDL2/SDL.h>

#include <cstdint>

#include "release_mask.hpp"

// Players 1 and 2 (libretro ports 0 and 1) from the first two joysticks, as DualShocks.
// Switch buttons by position: A=Circle, B=Cross, X=Triangle, Y=Square (the libretro ids
// A,B,X,Y as is); L,R=L1,R1; ZL,ZR=L2,R2; stick clicks=L3,R3; Plus=Start, Minus=Select.
// The D-pad is the directions; the sticks are the DualShock sticks.
class Input {
public:
    void init();
    void shutdown();
    void handleEvent(const SDL_Event& e);                  // hot-plug
    int16_t state(unsigned port, unsigned retroId);  // RETRO_DEVICE_ID_JOYPAD_*, hides suppressed buttons
    // RETRO_DEVICE_INDEX_ANALOG_LEFT/RIGHT, RETRO_DEVICE_ID_ANALOG_X/Y; -32768..32767, down positive.
    int16_t analog(unsigned port, unsigned index, unsigned axis) const;
    // Hides every button held right now from the game until it is released (menu close).
    void suppressHeld();
    // Buttons newly pressed since the previous poll(); call once per frame.
    struct Edges {
        bool up = false, down = false, left = false, right = false;  // D-pad or left stick
        bool confirm = false, back = false;                          // A, B
        bool menuCombo = false;  // Minus+Plus together (ZL+ZR are L2+R2 on this console)
    };
    Edges poll();
    // True once if player 1's pad was unplugged since the last call.
    bool takePadLost();

private:
    int16_t rawState(unsigned port, unsigned retroId) const;
    void rebuild();
    SDL_Joystick* pads_[2] = {nullptr, nullptr};
    SDL_Joystick* all_[8] = {};
    bool prevUp_ = false, prevDown_ = false, prevLeft_ = false, prevRight_ = false, prevA_ = false, prevB_ = false, prevCombo_ = false;
    bool padLost_ = false;
    ReleaseMask release_[2];
};

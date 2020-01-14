#pragma once

#include <switch.h>

// Each of these functions correspond to a given menu function
// And returns true if they are to occur on this frame. Maybe
// could include controller keybinds in the future.
namespace KB {
    bool openMenu() {
        return hidKeyboardDown(KBD_O);
    }

    bool closeMenu() {
        return hidKeyboardDown(KBD_C);
    }
}
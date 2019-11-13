#pragma once

#include <ctime>

extern "C" {
    #include <switch.h>
}

float millisecondsSinceEpoch() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // This should be good enough
    return (float) tv.tv_sec * 1000 + (float) tv.tv_usec / 1000;
}

// This namespace includes the time that each key started
namespace KeyTimes { 
    float keyZL = 0.0f;
    float keyZR = 0.0f;
    float keyL = 0.0f;
    float keyR = 0.0f;
    float SECONDS_BEFORE_NULL = 0.8f;
}

bool areAllTriggersPressed() {
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
    bool ZLDown = kHeld & KEY_ZL;
    bool ZRDown = kHeld & KEY_ZR;
    bool LDown = kHeld & KEY_L;
    bool RDown = kHeld & KEY_R;
    bool keysHeldNow = ZLDown && ZRDown && LDown && RDown;
    bool keysHeldBefore = KeyTimes::keyZL && KeyTimes::keyZR && KeyTimes::keyL && KeyTimes::keyR;
    if (keysHeldNow && keysHeldBefore) {
        // All the keys are held, maybe the keybind will trigger
        bool ZLJustUsed = false;
        if (millisecondsSinceEpoch() - KeyTimes::keyZL <= KeyTimes::SECONDS_BEFORE_NULL) {
            ZLJustUsed = true;
        }
        bool ZRJustUsed = false;
        if (millisecondsSinceEpoch() - KeyTimes::keyZR <= KeyTimes::SECONDS_BEFORE_NULL) {
            ZRJustUsed = true;
        }
        bool LJustUsed = false;
        if (millisecondsSinceEpoch() - KeyTimes::keyL <= KeyTimes::SECONDS_BEFORE_NULL) {
            LJustUsed = true;
        }
        bool RJustUsed = false;
        if (millisecondsSinceEpoch() - KeyTimes::keyR <= KeyTimes::SECONDS_BEFORE_NULL) {
            RJustUsed = true;
        }
        if (ZLJustUsed && ZRJustUsed && LJustUsed && RJustUsed) {
            return true;
        }
    }
    // Catch all failure
    return false;
}
#pragma once

#include <ctime>
#include <thread>
#include <chrono>

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

// Returns true only if all the triggers have been pressed down around the same time
// Binding to open the UI
bool allTriggersKeybind() {
	u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
	u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
	u64 kUp = hidKeysUp(CONTROLLER_P1_AUTO);
	bool ZLDown = kHeld & KEY_ZL;
	bool ZRDown = kHeld & KEY_ZR;
	bool LDown = kHeld & KEY_L;
	bool RDown = kHeld & KEY_R;
	bool keysHeldNow = ZLDown && ZRDown && LDown && RDown;
	// Returns true if all the keys have been held down, but not this frame
	bool keysHeldBefore = KeyTimes::keyZL && KeyTimes::keyZR && KeyTimes::keyL && KeyTimes::keyR;
	if (keysHeldNow && keysHeldBefore) {
		// All the keys are held, maybe the keybind will trigger
		bool ZLJustUsed = false;
		if (millisecondsSinceEpoch() - KeyTimes::keyZL <= KeyTimes::SECONDS_BEFORE_NULL) {
			ZLJustUsed = true;
			// Reset it again
			KeyTimes::keyZL = 0.0f;
		}
		bool ZRJustUsed = false;
		if (millisecondsSinceEpoch() - KeyTimes::keyZR <= KeyTimes::SECONDS_BEFORE_NULL) {
			ZRJustUsed = true;
			// Reset it again
			KeyTimes::keyZR = 0.0f;
		}
		bool LJustUsed = false;
		if (millisecondsSinceEpoch() - KeyTimes::keyL <= KeyTimes::SECONDS_BEFORE_NULL) {
			LJustUsed = true;
			// Reset it again
			KeyTimes::keyL = 0.0f;
		}
		bool RJustUsed = false;
		if (millisecondsSinceEpoch() - KeyTimes::keyR <= KeyTimes::SECONDS_BEFORE_NULL) {
			RJustUsed = true;
			// Reset it again
			KeyTimes::keyR = 0.0f;
		}
		if (ZLJustUsed && ZRJustUsed && LJustUsed && RJustUsed) {
			return true;
		}
	}
	// Make sure all keys are updated
	if (kDown & KEY_ZL) {
		// This key was just pressed
		KeyTimes::keyZL = millisecondsSinceEpoch();
	} else if (kUp & KEY_ZL) {
		// This key was just released
		KeyTimes::keyZL = 0.0f;
	}

	if (kDown & KEY_ZR) {
		// This key was just pressed
		KeyTimes::keyZR = millisecondsSinceEpoch();
	} else if (kUp & KEY_ZR) {
		// This key was just released
		KeyTimes::keyZR = 0.0f;
	}

	if (kDown & KEY_L) {
		// This key was just pressed
		KeyTimes::keyL = millisecondsSinceEpoch();
	} else if (kUp & KEY_L) {
		// This key was just released
		KeyTimes::keyL = 0.0f;
	}

	if (kDown & KEY_R) {
		// This key was just pressed
		KeyTimes::keyR = millisecondsSinceEpoch();
	} else if (kUp & KEY_R) {
		// This key was just released
		KeyTimes::keyR = 0.0f;
	}
	// Catch all failure
	return false;
}

// Sleep a thread for a specified number of milliseconds
void sleepMs(int milliseconds) {
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

// Call a function after a specified amount of time
void setTimeout(auto function, int delay) {
	std::thread t([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		function();
	});
	t.detach();
}
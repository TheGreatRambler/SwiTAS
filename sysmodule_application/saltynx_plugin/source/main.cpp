#include <cstring>
#include <saltysd/SaltySD_core.h>
#include <saltysd/SaltySD_dynamic.h>
#include <saltysd/SaltySD_ipc.h>
#include <string>
#include <switch_min.h>

#include "header.hpp"
#include "sdkTypes.hpp"

// clang-format off
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
// clang-format on

uint64_t ticksToNanoseconds(uint64_t tick) {
	return (tick * 625) / 12;
}

// Print log info
uint8_t dumpDebugInfo = true;
// This needs to be set to spoof, turned off for testing
uint8_t recordInputs = true;

uint8_t frameHasPassed = false;

uint16_t logStringIndex = 0;
char logString[1024];

uint8_t wasJustLeft;
uint8_t wasJustTASController;
int32_t lastControllerId;

// This is the controller that needs to record, all other controllers get their
// state From the exposed sensor states. If -1, no controller records (all
// accept input) If -2, every controller records (none accept input)
int32_t controllerToRecord = -2;

nn::hid::SixAxisSensorHandle sixAxisHandlesLeftJoycon[8]  = { 0 };
nn::hid::SixAxisSensorHandle sixAxisHandlesRightJoycon[8] = { 0 };

// Update state of left and right joycons along with the touchscreen
nn::hid::SixAxisSensorState sixAxisStateLeftJoycon[8]  = { 0 };
nn::hid::SixAxisSensorState sixAxisStateRightJoycon[8] = { 0 };
nn::hid::TouchScreenState16Touch touchscreenState      = { 0 };
nn::hid::KeyboardState keyboardState                   = { 0 };
nn::hid::MouseState mouseState                         = { 0 };

int32_t leftJoyconBacklogSize  = 0;
int32_t rightJoyconBacklogSize = 0;
nn::hid::SixAxisSensorState
	sixAxisStateLeftJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax]
	= { 0 };
nn::hid::SixAxisSensorState
	sixAxisStateRightJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax]
	= { 0 };

// Updated with the real values, to spoof touch values better
uint64_t touchScreenLastAccessTime
	= ticksToNanoseconds(_ZN2nn2os13GetSystemTickEv());

// Whether to record the touchscreen or the keyboard (and mouse)
// Zero means record both, 1 means touchscreen, 2 means keyboard and mouse
// 3 means both are set by the code
SaltyNXCommTypes::ThingToRecord recordScreenOrKeyboard
	= SaltyNXCommTypes::ThingToRecord::Both_Touch_And_Key;

int32_t touchScreenBacklogSize = 0;
nn::hid::TouchScreenState16Touch
	touchScreenStateBacklog[nn::hid::TouchScreenStateCountMax]
	= { 0 };

int32_t keyboardBacklogSize = 0;
nn::hid::KeyboardState keyboardStateBacklog[nn::hid::KeyboardStateCountMax]
	= { 0 };

int32_t mouseBacklogSize                                           = 0;
nn::hid::MouseState mouseStateBacklog[nn::hid::MouseStateCountMax] = { 0 };

SaltyNXCommTypes::PerformanceType performanceMode
	= SaltyNXCommTypes::PerformanceType::Docked;

// Number of controllers the game should know exists
int32_t numberOfTASControllers = -1;

void writeToLog(const char* str) {
	uint16_t stringLength = strlen(str);
	if(logStringIndex + stringLength < sizeof(logString)) {
		memcpy(&logString[logStringIndex], str, stringLength);
		logStringIndex += stringLength;
	}
}

uint8_t canWriteToLog() {
	// Give a generous estimate
	return logStringIndex < (sizeof(logString) - 100);
}

void moveLeftBacklog(int32_t i, nn::hid::SixAxisSensorState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&sixAxisStateLeftJoyconBacklog[i][1],
		&sixAxisStateLeftJoyconBacklog[i][0],
		sizeof(nn::hid::SixAxisSensorState)
			* (nn::hid::SixAxisSensorStateCountMax - 1));
	if(leftJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
		leftJoyconBacklogSize++;
	}

	memcpy(sixAxisStateLeftJoyconBacklog[i], state,
		sizeof(nn::hid::SixAxisSensorState));
}

void moveRightBacklog(int32_t i, nn::hid::SixAxisSensorState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&sixAxisStateRightJoyconBacklog[i][1],
		&sixAxisStateRightJoyconBacklog[i][0],
		sizeof(nn::hid::SixAxisSensorState)
			* (nn::hid::SixAxisSensorStateCountMax - 1));
	if(rightJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
		rightJoyconBacklogSize++;
	}

	memcpy(&sixAxisStateRightJoyconBacklog[i][0], state,
		sizeof(nn::hid::SixAxisSensorState));
}

void moveTouchBacklog(void* state, int32_t numOfMaxTouches) {
	// Get state as normal and move back backlog by 1
	memmove(&touchScreenStateBacklog[1], &touchScreenStateBacklog[0],
		sizeof(nn::hid::TouchScreenState16Touch)
			* (nn::hid::TouchScreenStateCountMax - 1));
	if(touchScreenBacklogSize != nn::hid::TouchScreenStateCountMax) {
		touchScreenBacklogSize++;
	}

	// Header is 16 bytes
	size_t sizeOfStruct = 16 + sizeof(nn::hid::TouchState) * numOfMaxTouches;
	memcpy(&touchScreenStateBacklog[0], state, sizeOfStruct);
	if(touchScreenStateBacklog[0].count > numOfMaxTouches) {
		touchScreenStateBacklog[0].count = numOfMaxTouches;
	}
}

void moveMouseBacklog(nn::hid::MouseState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&mouseStateBacklog[1], &mouseStateBacklog[0],
		sizeof(nn::hid::MouseState) * (nn::hid::MouseStateCountMax - 1));
	if(mouseBacklogSize != nn::hid::MouseStateCountMax) {
		mouseBacklogSize++;
	}

	memcpy(&mouseStateBacklog[0], state, sizeof(nn::hid::MouseState));
}

void moveKeyboardBacklog(nn::hid::KeyboardState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&keyboardStateBacklog[1], &keyboardStateBacklog[0],
		sizeof(nn::hid::KeyboardState) * (nn::hid::KeyboardStateCountMax - 1));
	if(keyboardBacklogSize != nn::hid::KeyboardStateCountMax) {
		keyboardBacklogSize++;
	}

	memcpy(&keyboardStateBacklog[0], state, sizeof(nn::hid::KeyboardState));
}

void fixMotionState(
	nn::hid::SixAxisSensorState* dest, nn::hid::SixAxisSensorState* orig) {
	dest->deltaTimeNanoSeconds = orig->deltaTimeNanoSeconds;
	dest->samplingNumber       = orig->samplingNumber;
	// I dunno
	dest->attributes
		|= (uint8_t)nn::hid::SixAxisSensorAttribute::IsInterpolated;
	dest->attributes |= (uint8_t)nn::hid::SixAxisSensorAttribute::IsConnected;
}

void fixTouchState(nn::hid::TouchScreenState16Touch* dest, void* orig,
	int32_t numOfMaxTouches) {
	nn::hid::TouchScreenState16Touch* orig16Touch
		= (nn::hid::TouchScreenState16Touch*)orig;
	dest->samplingNumber = orig16Touch->samplingNumber;

	uint64_t currentNanosecond
		= ticksToNanoseconds(_ZN2nn2os13GetSystemTickEv());
	uint64_t accessDuration   = currentNanosecond - touchScreenLastAccessTime;
	touchScreenLastAccessTime = currentNanosecond;

	for(int32_t i = 0; i < dest->count; i++) {
		dest->touches[i].attributes    = 0;
		dest->touches[i].rotationAngle = 0;
		dest->touches[i].diameterX     = 15;
		dest->touches[i].diameterY     = 15;

		dest->touches[i].touchIndex           = i;
		dest->touches[i].deltaTimeNanoSeconds = accessDuration;
	}
}

void fixKeyboardState(
	nn::hid::KeyboardState* dest, nn::hid::KeyboardState* orig) {
	dest->samplingNumber = orig->samplingNumber;
	// Keyboard is always listed as connected
	dest->attributes |= (int32_t)nn::hid::KeyboardAttribute::IsConnected;
}

void fixMouseState(nn::hid::MouseState* dest, nn::hid::MouseState* orig) {
	dest->samplingNumber = orig->samplingNumber;
	// Mouse is always listed as connected
	dest->attributes |= (int32_t)nn::hid::MouseAttribute::IsConnected;
}

void GetSixAxisSensorHandle1(nn::hid::ConsoleSixAxisSensorHandle* handle) {
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(
		handle);
}

void GetSixAxisSensorHandle2(
	nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) {
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(
		handle, param_2);
}

void GetSixAxisSensorHandles1(nn::hid::SixAxisSensorHandle* handle1,
	nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) {
	_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(
		handle1, handle2, param_3);
}

int32_t GetSixAxisSensorHandles2(nn::hid::SixAxisSensorHandle* handles,
	int32_t numOfHandles, const nn::hid::NpadIdType& id,
	int32_t npadStyleBitflags) {
	// Ignore numOfHandles
	// To see what kind of controller, and the bit flags with the chosen
	// NpadStyleTag if (npadStyleBitflags &
	// nn::hid::NpadStyleTag::ProController) Returned number is the number of
	// successful handles, I think
	if(recordInputs) {
		if(id != nn::hid::NpadIdType::Unknown
			&& id != nn::hid::NpadIdType::Handheld) {
			// We're dealing with a potential hid:dbg controller
			// Check if it is dual joycons
			if(numOfHandles == 2) {
				int32_t res
					= _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(
						handles, numOfHandles, id, npadStyleBitflags);

				sixAxisHandlesLeftJoycon[(int32_t)id]  = handles[0];
				sixAxisHandlesRightJoycon[(int32_t)id] = handles[1];

				wasJustTASController = true;

				return res;
			}
		}
	}

	return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(
		handles, numOfHandles, id, npadStyleBitflags);
}

void GetSixAxisSensorState(nn::hid::SixAxisSensorState* state,
	const nn::hid::SixAxisSensorHandle& handle) {
	if(recordInputs) {
		for(int32_t i = 0; i < 8; i++) {
			if(sixAxisHandlesLeftJoycon[i] == handle) {
				_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(
					state, handle);

				/*
								writeToLog((
									std::string("") + "state->acceleration.x: "
									+ std::to_string(state->acceleration.x) + "
				   "
									+ "state->acceleration.y: "
									+ std::to_string(state->acceleration.y) + "
				   "
									+ "state->acceleration.z: "
									+ std::to_string(state->acceleration.z) + "
				   "
									+ "state->angle.x: " +
				   std::to_string(state->angle.x) + " "
									+ "state->angle.y: " +
				   std::to_string(state->angle.y) + " "
									+ "state->angle.z: " +
				   std::to_string(state->angle.z) + " "
									+ "state->angularVelocity.x: "
									+ std::to_string(state->angularVelocity.x) +
				   " "
									+ "state->angularVelocity.y: "
									+ std::to_string(state->angularVelocity.y) +
				   " "
									+ "state->angularVelocity.z: "
									+ std::to_string(state->angularVelocity.z) +
				   " "
									+ "state->direction.x.x: "
									+ std::to_string(state->direction.x.x) + " "
									+ "state->direction.y.x: "
									+ std::to_string(state->direction.y.x) + " "
									+ "state->direction.z.x: "
									+ std::to_string(state->direction.z.x) + " "
									+ "state->direction.x.y: "
									+ std::to_string(state->direction.x.y) + " "
									+ "state->direction.y.y: "
									+ std::to_string(state->direction.y.y) + " "
									+ "state->direction.z.y: "
									+ std::to_string(state->direction.z.y) + " "
									+ "state->direction.x.z: "
									+ std::to_string(state->direction.x.z) + " "
									+ "state->direction.y.z: "
									+ std::to_string(state->direction.y.z) + " "
									+ "state->direction.z.z: "
									+ std::to_string(state->direction.z.z) + " "
									+ "state->attributes: " +
				   std::to_string(state->attributes)
									+ " " + "state->deltaTimeNanoSeconds: "
									+
				   std::to_string(state->deltaTimeNanoSeconds) + " "
									+ std::to_string(state->samplingNumber))
											   .c_str());
				*/

				if((controllerToRecord != -2)
					&& (controllerToRecord == -1 || controllerToRecord != i)) {
					// Fix incoming state
					fixMotionState(&sixAxisStateLeftJoycon[i], state);
					// Push the state onto the backlog
					moveLeftBacklog(i, &sixAxisStateLeftJoycon[i]);
					// Copy the state into the returning state
					memcpy(state, &sixAxisStateLeftJoycon[i],
						sizeof(nn::hid::SixAxisSensorState));
				} else {
					// Just add to the backlog, nothing else
					moveLeftBacklog(i, state);
				}

				wasJustLeft          = true;
				wasJustTASController = true;
				lastControllerId     = i;

				return;
			}

			if(sixAxisHandlesRightJoycon[i] == handle) {
				_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(
					state, handle);

				if((controllerToRecord != -2)
					&& (controllerToRecord == -1 || controllerToRecord != i)) {
					fixMotionState(&sixAxisStateRightJoycon[i], state);
					moveRightBacklog(i, &sixAxisStateRightJoycon[i]);
					memcpy(state, &sixAxisStateRightJoycon[i],
						sizeof(nn::hid::SixAxisSensorState));
				} else {
					moveRightBacklog(i, state);
				}

				wasJustLeft          = false;
				wasJustTASController = true;
				lastControllerId     = i;

				return;
			}
		}
	}

	_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(
		state, handle);
}

int32_t GetSixAxisSensorStates1(nn::hid::SixAxisSensorState* outStates,
	int32_t count, const nn::hid::BasicXpadId& handle) {
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(
		outStates, count, handle);
}

int32_t GetSixAxisSensorStates2(nn::hid::SixAxisSensorState* outStates,
	int32_t count, const nn::hid::SixAxisSensorHandle& handle) {
	if(recordInputs) {
		// Get state as normal to advance the backlog
		nn::hid::SixAxisSensorState dummyState;
		GetSixAxisSensorState(&dummyState, handle);

		if(wasJustTASController) {
			// TAS controller being a kind of controller that would be handled
			// by TAS
			wasJustTASController = false;
			if(wasJustLeft) {
				int32_t backlogSize = max(count, leftJoyconBacklogSize);
				memcpy(outStates,
					sixAxisStateLeftJoyconBacklog[lastControllerId],
					sizeof(nn::hid::SixAxisSensorState) * backlogSize);
				return backlogSize;
			} else {
				int32_t backlogSize = max(count, rightJoyconBacklogSize);
				memcpy(outStates,
					sixAxisStateRightJoyconBacklog[lastControllerId],
					sizeof(nn::hid::SixAxisSensorState) * backlogSize);
				return backlogSize;
			}
		} else {
			// This will trigger for controllers outside of the standard range,
			// including pro controllers and single joycons
			return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(
				outStates, count, handle);
		}
	}

	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(
		outStates, count, handle);
}

int32_t IsSixAxisSensorAtRest(const nn::hid::SixAxisSensorHandle& param_1) {
	return false;
	// return
	// _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

void GetTouchScreenState1Touch(nn::hid::TouchScreenState1Touch* state) {
	if(recordInputs) {
		_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE(
			state);

		if(recordScreenOrKeyboard
				== SaltyNXCommTypes::ThingToRecord::Neither_Touch_Nor_Key
			|| recordScreenOrKeyboard == SaltyNXCommTypes::ThingToRecord::Key) {
			// Fix state and send to game
			fixTouchState(&touchscreenState, state, 1);
			moveTouchBacklog(&touchscreenState, 1);
			memcpy(state, &touchscreenState,
				sizeof(nn::hid::TouchScreenState1Touch));
		} else {
			moveTouchBacklog(state, 1);
		}
		return;
	}

	_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE(
		state);
}

void GetKeyboardState(nn::hid::KeyboardState* state) {
	if(recordInputs) {
		_ZN2nn3hid16GetKeyboardStateEPNS0_13KeyboardStateE(state);

		if(recordScreenOrKeyboard
				== SaltyNXCommTypes::ThingToRecord::Neither_Touch_Nor_Key
			|| recordScreenOrKeyboard
				   == SaltyNXCommTypes::ThingToRecord::Touch) {
			fixKeyboardState(&keyboardState, state);
			moveKeyboardBacklog(&keyboardState);
			memcpy(state, &keyboardStateBacklog,
				sizeof(nn::hid::SixAxisSensorState));
		} else {
			moveKeyboardBacklog(state);
		}
		return;
	}

	_ZN2nn3hid16GetKeyboardStateEPNS0_13KeyboardStateE(state);
}

int32_t GetKeyboardStates(nn::hid::KeyboardState* outStates, int32_t count) {
	if(recordInputs) {
		nn::hid::KeyboardState dummyState;
		GetKeyboardState(&dummyState);

		int32_t backlogSize = max(count, keyboardBacklogSize);
		memcpy(outStates, &keyboardStateBacklog,
			sizeof(nn::hid::KeyboardState) * backlogSize);
		return backlogSize;
	}

	return _ZN2nn3hid17GetKeyboardStatesEPNS0_13KeyboardStateEi(
		outStates, count);
}

void GetMouseState(nn::hid::MouseState* state) {
	if(recordInputs) {
		_ZN2nn3hid13GetMouseStateEPNS0_10MouseStateE(state);

		if(recordScreenOrKeyboard
				== SaltyNXCommTypes::ThingToRecord::Neither_Touch_Nor_Key
			|| recordScreenOrKeyboard
				   == SaltyNXCommTypes::ThingToRecord::Touch) {
			fixMouseState(&mouseState, state);
			moveMouseBacklog(&mouseState);
			memcpy(state, &mouseStateBacklog, sizeof(nn::hid::MouseState));
		} else {
			moveMouseBacklog(state);
		}
		return;
	}

	_ZN2nn3hid13GetMouseStateEPNS0_10MouseStateE(state);
}

int32_t GetMouseStates(nn::hid::MouseState* outStates, int32_t count) {
	if(recordInputs) {
		nn::hid::MouseState dummyState;
		GetMouseState(&dummyState);

		int32_t backlogSize = max(count, mouseBacklogSize);
		memcpy(outStates, &mouseStateBacklog,
			sizeof(nn::hid::MouseState) * backlogSize);
		return backlogSize;
	}

	return _ZN2nn3hid14GetMouseStatesEPNS0_10MouseStateEi(outStates, count);
}

void GetNpadState1(void* state, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		_ZN2nn3hid12GetNpadStateEPNS0_16NpadFullKeyStateERKj(state, id);
	}
}

void GetNpadState2(void* state, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		_ZN2nn3hid12GetNpadStateEPNS0_17NpadHandheldStateERKj(state, id);
	}
}

void GetNpadState3(void* state, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		_ZN2nn3hid12GetNpadStateEPNS0_16NpadJoyDualStateERKj(state, id);
	}
}

void GetNpadState4(void* state, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		_ZN2nn3hid12GetNpadStateEPNS0_16NpadJoyLeftStateERKj(state, id);
	}
}

void GetNpadState5(void* state, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		_ZN2nn3hid12GetNpadStateEPNS0_17NpadJoyRightStateERKj(state, id);
	}
}

uint64_t GetNpadStates1(
	void* state, int32_t unk, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		return _ZN2nn3hid13GetNpadStatesEPNS0_16NpadFullKeyStateEiRKj(
			state, unk, id);
	}
	return unk;
}

uint64_t GetNpadStates2(
	void* state, int32_t unk, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		return _ZN2nn3hid13GetNpadStatesEPNS0_17NpadHandheldStateEiRKj(
			state, unk, id);
	}
	return unk;
}

uint64_t GetNpadStates3(
	void* state, int32_t unk, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		return _ZN2nn3hid13GetNpadStatesEPNS0_16NpadJoyDualStateEiRKj(
			state, unk, id);
	}
	return unk;
}

uint64_t GetNpadStates4(
	void* state, int32_t unk, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		return _ZN2nn3hid13GetNpadStatesEPNS0_16NpadJoyLeftStateEiRKj(
			state, unk, id);
	}
	return unk;
}

uint64_t GetNpadStates5(
	void* state, int32_t unk, const nn::hid::NpadIdType& id) {
	if(numberOfTASControllers == -1 || (int32_t)id < numberOfTASControllers) {
		return _ZN2nn3hid13GetNpadStatesEPNS0_17NpadJoyRightStateEiRKj(
			state, unk, id);
	}
	return unk;
}

uintptr_t ptr_nvnDeviceGetProcAddress;
uintptr_t ptr_nvnQueuePresentTexture;
uintptr_t addr_nvnGetProcAddress;
uintptr_t addr_nvnPresentTexture;
typedef void (*nvnQueuePresentTexture_0)(
	void* unk1_1, void* unk2_1, void* unk3_1);
typedef uintptr_t (*GetProcAddress)(void* unk1_a, const char* nvnFunction_a);

uint32_t vulkanSwap(void* vk_unk1_1, void* vk_unk2_1) {
	uint32_t vulkanResult = vkQueuePresentKHR(vk_unk1_1, vk_unk2_1);
	frameHasPassed        = true;
	return vulkanResult;
}

void eglSwap(void* egl_unk1_1, void* egl_unk2_1) {
	eglSwapBuffers(egl_unk1_1, egl_unk2_1);
	frameHasPassed = true;
	return;
}

void nvnPresentTexture(void* unk1, void* unk2, void* unk3) {
	((nvnQueuePresentTexture_0)(ptr_nvnQueuePresentTexture))(unk1, unk2, unk3);
	frameHasPassed = true;
	return;
}

uintptr_t nvnGetProcAddress(void* unk1, const char* nvnFunction) {
	uintptr_t address
		= ((GetProcAddress)(ptr_nvnDeviceGetProcAddress))(unk1, nvnFunction);
	if(strcmp("nvnDeviceGetProcAddress", nvnFunction) == 0)
		return addr_nvnGetProcAddress;
	else if(strcmp("nvnQueuePresentTexture", nvnFunction) == 0) {
		ptr_nvnQueuePresentTexture = address;
		return addr_nvnPresentTexture;
	} else
		return address;
}

uintptr_t nvnBootstrapLoader_1(const char* nvnName) {
	if(strcmp(nvnName, "nvnDeviceGetProcAddress") == 0) {
		ptr_nvnDeviceGetProcAddress
			= nvnBootstrapLoader("nvnDeviceGetProcAddress");
		return addr_nvnGetProcAddress;
	}
	uintptr_t ptrret = nvnBootstrapLoader(nvnName);
	return ptrret;
}

uint32_t GetPerformanceMode() {
	return performanceMode;
}

uint8_t GetOperationMode() {
	return performanceMode;
}

void writePointerToFile(void* ptr, FILE* file) {
	SaltySDCore_fwrite(&ptr, sizeof(ptr), 1, file);
}

int main(int argc, char* argv[]) {
	writeToLog("Alive\n");

	const char* pointersPath = "sdmc:/SaltySD/SwiTAS_SaltyPlugin_Offsets.hex";
	FILE* offsets            = SaltySDCore_fopen(pointersPath, "wb");

	// Frame has passed indicator
	writePointerToFile(&frameHasPassed, offsets);

	// Log handling
	writePointerToFile(&logStringIndex, offsets);
	writePointerToFile(&logString, offsets);

	// Controller to record from the system
	writePointerToFile(&controllerToRecord, offsets);

	// State to send to six axis
	writePointerToFile(&sixAxisStateLeftJoycon, offsets);
	writePointerToFile(&sixAxisStateRightJoycon, offsets);

	// Recorded inputs of six axis
	writePointerToFile(&sixAxisStateLeftJoyconBacklog, offsets);
	writePointerToFile(&sixAxisStateRightJoyconBacklog, offsets);

	// Whether to record touch screen or keyboard
	writePointerToFile(&recordScreenOrKeyboard, offsets);

	// State to send to touch screen
	writePointerToFile(&touchscreenState, offsets);

	// Recorded inputs of touch screen
	writePointerToFile(&touchScreenStateBacklog, offsets);

	// State to send to keyboard
	writePointerToFile(&keyboardState, offsets);

	// Recorded inputs of keyboard
	writePointerToFile(&keyboardStateBacklog, offsets);

	// State to send to mouse
	writePointerToFile(&mouseState, offsets);

	// Recorded inputs of mouse
	writePointerToFile(&mouseStateBacklog, offsets);

	// Number of TAS controllers right now
	writePointerToFile(&numberOfTASControllers, offsets);

	SaltySDCore_fclose(offsets);

	FILE* performanceFile;
	if(performanceFile
		= SaltySDCore_fopen("/SaltySD/flags/handheld.flag", "r")) {
		performanceMode = SaltyNXCommTypes::PerformanceType::Handheld;
		SaltySDCore_fclose(performanceFile);
	} else if(performanceFile
			  = SaltySDCore_fopen("/SaltySD/flags/docked.flag", "r")) {
		performanceMode = SaltyNXCommTypes::PerformanceType::Docked;
		SaltySDCore_fclose(performanceFile);
	} else {
		// Default to docked
		performanceMode = SaltyNXCommTypes::PerformanceType::Docked;
	}

	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE",
		(void*)&GetSixAxisSensorHandle1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE",
		(void*)&GetSixAxisSensorHandle2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE",
		(void*)&GetSixAxisSensorHandles1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE",
		(void*)&GetSixAxisSensorHandles2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE",
		(void*)&GetSixAxisSensorState);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE",
		(void*)&GetSixAxisSensorStates1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE",
		(void*)&GetSixAxisSensorStates2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE",
		(void*)&IsSixAxisSensorAtRest);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE",
		(void*)&GetTouchScreenState1Touch);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid16GetKeyboardStateEPNS0_13KeyboardStateE",
		(void*)&GetKeyboardState);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid17GetKeyboardStatesEPNS0_13KeyboardStateEi",
		(void*)&GetKeyboardStates);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetMouseStateEPNS0_10MouseStateE", (void*)&GetMouseState);
	SaltySDCore_ReplaceImport("_ZN2nn3hid14GetMouseStatesEPNS0_10MouseStateEi",
		(void*)&GetMouseStates);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid12GetNpadStateEPNS0_16NpadFullKeyStateERKj",
		(void*)&GetNpadState1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid12GetNpadStateEPNS0_17NpadHandheldStateERKj",
		(void*)&GetNpadState2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid12GetNpadStateEPNS0_16NpadJoyDualStateERKj",
		(void*)&GetNpadState3);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid12GetNpadStateEPNS0_16NpadJoyLeftStateERKj",
		(void*)&GetNpadState4);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid12GetNpadStateEPNS0_17NpadJoyRightStateERKj",
		(void*)&GetNpadState5);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetNpadStatesEPNS0_16NpadFullKeyStateEiRKj",
		(void*)&GetNpadStates1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetNpadStatesEPNS0_17NpadHandheldStateEiRKj",
		(void*)&GetNpadStates2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetNpadStatesEPNS0_16NpadJoyDualStateEiRKj",
		(void*)&GetNpadStates3);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetNpadStatesEPNS0_16NpadJoyLeftStateEiRKj",
		(void*)&GetNpadStates4);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid13GetNpadStatesEPNS0_17NpadJoyRightStateEiRKj",
		(void*)&GetNpadStates5);

	addr_nvnGetProcAddress = (uint64_t)&nvnGetProcAddress;
	addr_nvnPresentTexture = (uint64_t)&nvnPresentTexture;
	SaltySDCore_ReplaceImport(
		"nvnBootstrapLoader", (void*)nvnBootstrapLoader_1);
	SaltySDCore_ReplaceImport("eglSwapBuffers", (void*)eglSwap);
	SaltySDCore_ReplaceImport("vkQueuePresentKHR", (void*)vulkanSwap);

	SaltySDCore_ReplaceImport(
		"_ZN2nn2oe18GetPerformanceModeEv", (void*)GetPerformanceMode);
	SaltySDCore_ReplaceImport(
		"_ZN2nn2oe16GetOperationModeEv", (void*)GetOperationMode);

	writeToLog("Injection finished\n");
}
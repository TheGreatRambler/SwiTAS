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
uint8_t recordInputs = false;

uint8_t frameHasPassed = false;

uint16_t logStringIndex = 0;
char logString[1024];

uint8_t wasJustLeft;
uint8_t wasJustTASController;
int32_t lastControllerId;

// This is the controller that needs to record, all other controllers get their state
// From the exposed sensor states
// If -1, every controller records
int32_t controllerToRecord = (int32_t)nn::hid::NpadIdType::None;

nn::hid::SixAxisSensorHandle sixAxisHandlesLeftJoycon[8]  = { 0 };
nn::hid::SixAxisSensorHandle sixAxisHandlesRightJoycon[8] = { 0 };

// Update state of left and right joycons along with the touchscreen
nn::hid::SixAxisSensorState sixAxisStateLeftJoycon[8]  = { 0 };
nn::hid::SixAxisSensorState sixAxisStateRightJoycon[8] = { 0 };
nn::hid::TouchScreenState16Touch touchscreenState      = { 0 };
nn::hid::KeyboardState keyboardState                   = { 0 };
nn::hid::MouseState mouseState                         = { 0 };

int32_t leftJoyconBacklogSize                                                                      = 0;
int32_t rightJoyconBacklogSize                                                                     = 0;
nn::hid::SixAxisSensorState sixAxisStateLeftJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax]  = { 0 };
nn::hid::SixAxisSensorState sixAxisStateRightJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax] = { 0 };

// Updated with the real values, to spoof touch values better
uint64_t touchScreenLastAccessTime = ticksToNanoseconds(_ZN2nn2os13GetSystemTickEv());

// Whether to record the touchscreen or the keyboard (and mouse)
// Zero means record both, 1 means touchscreen, 2 means keyboard and mouse
uint8_t recordScreenOrKeyboard = 0;

int32_t touchScreenBacklogSize                                                              = 0;
nn::hid::TouchScreenState16Touch touchScreenStateBacklog[nn::hid::TouchScreenStateCountMax] = { 0 };

int32_t keyboardBacklogSize                                                 = 0;
nn::hid::KeyboardState keyboardStateBacklog[nn::hid::KeyboardStateCountMax] = { 0 };

int32_t mouseBacklogSize                                           = 0;
nn::hid::MouseState mouseStateBacklog[nn::hid::MouseStateCountMax] = { 0 };

// ONLY values we will TAS
/*
	nn::util::Float3 acceleration;
	nn::util::Float3 angularVelocity;
	nn::util::Float3 angle;
*/
// All these values are floats

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
	memmove(&sixAxisStateLeftJoyconBacklog[i][1], &sixAxisStateLeftJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * (nn::hid::SixAxisSensorStateCountMax - 1));
	if(leftJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
		leftJoyconBacklogSize++;
	}

	memcpy(&sixAxisStateLeftJoyconBacklog[i][0], state, sizeof(nn::hid::SixAxisSensorState));
}

void moveRightBacklog(int32_t i, nn::hid::SixAxisSensorState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&sixAxisStateRightJoyconBacklog[i][1], &sixAxisStateRightJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * (nn::hid::SixAxisSensorStateCountMax - 1));
	if(rightJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
		rightJoyconBacklogSize++;
	}

	memcpy(&sixAxisStateRightJoyconBacklog[i][0], state, sizeof(nn::hid::SixAxisSensorState));
}

void moveTouchBacklog(void* state, int32_t numOfMaxTouches) {
	// Get state as normal and move back backlog by 1
	memmove(&touchScreenStateBacklog[1], &touchScreenStateBacklog[0], sizeof(nn::hid::TouchScreenState16Touch) * (nn::hid::TouchScreenStateCountMax - 1));
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
	memmove(&mouseStateBacklog[1], &mouseStateBacklog[0], sizeof(nn::hid::MouseState) * (nn::hid::MouseStateCountMax - 1));
	if(mouseBacklogSize != nn::hid::MouseStateCountMax) {
		mouseBacklogSize++;
	}

	memcpy(&mouseStateBacklog[0], state, sizeof(nn::hid::MouseState));
}

void moveKeyboardBacklog(nn::hid::KeyboardState* state) {
	// Get state as normal and move back backlog by 1
	memmove(&keyboardStateBacklog[1], &keyboardStateBacklog[0], sizeof(nn::hid::KeyboardState) * (nn::hid::KeyboardStateCountMax - 1));
	if(keyboardBacklogSize != nn::hid::KeyboardStateCountMax) {
		keyboardBacklogSize++;
	}

	memcpy(&keyboardStateBacklog[0], state, sizeof(nn::hid::KeyboardState));
}

void fixMotionState(nn::hid::SixAxisSensorState* dest, nn::hid::SixAxisSensorState* orig) {
	dest->deltaTimeNanoSeconds = orig->deltaTimeNanoSeconds;
	dest->samplingNumber       = orig->samplingNumber;
	// I dunno
	dest->attributes |= (uint8_t)nn::hid::SixAxisSensorAttribute::IsInterpolated;
	// Direction state is also TASed
}

void fixTouchState(nn::hid::TouchScreenState16Touch* dest, void* orig, int32_t numOfMaxTouches) {
	nn::hid::TouchScreenState16Touch* orig16Touch = (nn::hid::TouchScreenState16Touch*)orig;
	dest->samplingNumber                          = orig16Touch->samplingNumber;

	uint64_t currentNanosecond = ticksToNanoseconds(_ZN2nn2os13GetSystemTickEv());
	uint64_t accessDuration    = currentNanosecond - touchScreenLastAccessTime;
	touchScreenLastAccessTime  = currentNanosecond;

	for(int32_t i = 0; i < dest->count; i++) {
		// TODO the following need to be made into defaults
		dest->touches[i].attributes    = 0;
		dest->touches[i].rotationAngle = 0;
		dest->touches[i].diameterX     = 15;
		dest->touches[i].diameterY     = 15;

		dest->touches[i].touchIndex           = i;
		dest->touches[i].deltaTimeNanoSeconds = accessDuration;
	}
}

void applyToStandardTouchBacklog(void* outStates, int32_t numOfMaxTouches) {
	// Header is 16 bytes
	size_t sizeOf1Struct = 16 + sizeof(nn::hid::TouchState) * numOfMaxTouches;
	for(int32_t i = 0; i < touchScreenBacklogSize; i++) {
		memcpy(outStates + sizeOf1Struct * i, &touchScreenStateBacklog[i], sizeOf1Struct);
	}
}

void fixKeyboardState(nn::hid::KeyboardState* dest, nn::hid::KeyboardState* orig) {
	dest->samplingNumber = orig->samplingNumber;
	// Keyboard is always listed as connected
	dest->attributes |= (int32_t)nn::hid::KeyboardAttribute::IsConnected;
}

void fixMouseState(nn::hid::MouseState* dest, nn::hid::MouseState* orig) {
	dest->samplingNumber = orig->samplingNumber;
	// Mouse is always listed as connected
	dest->attributes |= (int32_t)nn::hid::MouseAttribute::IsConnected;
}

// Motion spoofing

/* nn::hid::GetSixAxisSensorHandle(nn::hid::ConsoleSixAxisSensorHandle*) */
void GetSixAxisSensorHandle1(nn::hid::ConsoleSixAxisSensorHandle* handle) {
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(handle);
}

/* nn::hid::GetSixAxisSensorHandle(nn::hid::SixAxisSensorHandle*, nn::hid::BasicXpadId) */
void GetSixAxisSensorHandle2(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) {
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(handle, param_2);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, nn::hid::SixAxisSensorHandle*,
   nn::hid::JoyXpadId) */
void GetSixAxisSensorHandles1(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) {
	_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(handle1, handle2, param_3);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, int, unsigned int const&,
   nn::util::BitFlagSet<32, nn::hid::NpadStyleTag>) */
int32_t GetSixAxisSensorHandles2(nn::hid::SixAxisSensorHandle* handles, int32_t numOfHandles, const nn::hid::NpadIdType& id, int32_t npadStyleBitflags) {
	// Ignore numOfHandles
	// To see what kind of controller, and the bit flags with the chosen NpadStyleTag
	// if (npadStyleBitflags & nn::hid::NpadStyleTag::ProController)
	// Returned number is the number of successful handles, I think
	if(recordInputs) {
		if(id != nn::hid::NpadIdType::Unknown && id != nn::hid::NpadIdType::Handheld) {
			// We're dealing with a potential hid:dbg controller
			// Check if it is dual joycons
			if(numOfHandles == 2) {
				int32_t res = _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handles, numOfHandles, id, npadStyleBitflags);

				sixAxisHandlesLeftJoycon[(int32_t)id]  = handles[0];
				sixAxisHandlesRightJoycon[(int32_t)id] = handles[1];

				wasJustTASController = true;

				return res;
			}
		}
	}

	// Simply passthrough if need be
	return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handles, numOfHandles, id, npadStyleBitflags);
}

/* nn::hid::GetSixAxisSensorState(nn::hid::SixAxisSensorState*, nn::hid::SixAxisSensorHandle const&)
 */
void GetSixAxisSensorState(nn::hid::SixAxisSensorState* state, const nn::hid::SixAxisSensorHandle& handle) {
	if(recordInputs) {
		for(int32_t i = 0; i < 8; i++) {
			if(sixAxisHandlesLeftJoycon[i] == handle) {
				_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);

				if(controllerToRecord != i && controllerToRecord != -1) {
					fixMotionState(&sixAxisStateLeftJoycon[i], state);
					memcpy(state, &sixAxisStateLeftJoycon[i], sizeof(nn::hid::SixAxisSensorState));
				}

				moveLeftBacklog(i, state);

				wasJustLeft          = true;
				wasJustTASController = true;
				lastControllerId     = i;

				return;
			}

			if(sixAxisHandlesRightJoycon[i] == handle) {
				_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);

				if(controllerToRecord != i && controllerToRecord != -1) {
					fixMotionState(&sixAxisStateRightJoycon[i], state);
					memcpy(state, &sixAxisStateRightJoycon[i], sizeof(nn::hid::SixAxisSensorState));
				}

				moveRightBacklog(i, state);

				wasJustLeft          = false;
				wasJustTASController = true;
				lastControllerId     = i;

				return;
			}
		}
	}

	_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::BasicXpadId const&)
 */
int32_t GetSixAxisSensorStates1(nn::hid::SixAxisSensorState* outStates, int32_t count, const nn::hid::BasicXpadId& handle) {
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(outStates, count, handle);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::SixAxisSensorHandle
   const&) */
int32_t GetSixAxisSensorStates2(nn::hid::SixAxisSensorState* outStates, int32_t count, const nn::hid::SixAxisSensorHandle& handle) {
	if(recordInputs) {
		// Get state as normal to advance the backlog
		nn::hid::SixAxisSensorState dummyState;
		GetSixAxisSensorState(&dummyState, handle);

		if(wasJustTASController) {
			// TAS controller being a kind of controller that would be handled by TAS
			wasJustTASController = false;
			if(wasJustLeft) {
				int32_t backlogSize = max(count, leftJoyconBacklogSize);
				memcpy(outStates, &sixAxisStateLeftJoyconBacklog[lastControllerId][0], sizeof(nn::hid::SixAxisSensorState) * backlogSize);
				return backlogSize;
			} else {
				int32_t backlogSize = max(count, rightJoyconBacklogSize);
				memcpy(outStates, &sixAxisStateRightJoyconBacklog[lastControllerId][0], sizeof(nn::hid::SixAxisSensorState) * backlogSize);
				return backlogSize;
			}
		} else {
			// This will trigger for controllers outside of the standard range, including pro controllers and single joycons
			return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
		}
	}

	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
}

/* nn::hid::IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle const&) */
int32_t IsSixAxisSensorAtRest(const nn::hid::SixAxisSensorHandle& param_1) {
	return false;
	// return _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

// Touch screen spoofing
// void nn::hid::GetTouchScreenState (TouchScreenState< N > *pOutValue)
void GetTouchScreenState1Touch(nn::hid::TouchScreenState1Touch* state) {
	if(recordInputs) {
		_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE(state);

		if(recordScreenOrKeyboard == 2) {
			// Fix state and send to game
			fixTouchState(&touchscreenState, state, 1);
			memcpy(state, &touchscreenState, sizeof(nn::hid::TouchScreenState1Touch));
		}

		moveTouchBacklog(state, 1);

		return;
	}

	_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE(state);
	/*
		if(canWriteToLog()) {
			if(state->count == 1) {
				nn::hid::TouchState& touchState = state->touches[0];

				// clang-format off
					std::string diagInfo =
						"count: " + std::to_string(state->count) + "\n" +
						"samplingNumber: " + std::to_string(state->samplingNumber) + "\n" +
						"attributes: " + std::to_string(touchState.attributes) + "\n" +
						"deltaTimeNanoSeconds: " + std::to_string(touchState.deltaTimeNanoSeconds) + "\n" +
						"diameterX: " + std::to_string(touchState.diameterX) + "\n" +
						"diameterY: " + std::to_string(touchState.diameterY) + "\n" +
						"touchIndex: " + std::to_string(touchState.touchIndex) + "\n" +
						"rotationAngle: " + std::to_string(touchState.rotationAngle) + "\n" +
						"x: " + std::to_string(touchState.x) + "\n" +
						"y: " + std::to_string(touchState.y) + "\n";
				// clang-format on

				writeToLog(diagInfo.c_str());
			} else {
				writeToLog("Not a suitable amount of touches");
			}
		}
	*/
}

uintptr_t ptr_nvnDeviceGetProcAddress;
uintptr_t ptr_nvnQueuePresentTexture;
uintptr_t addr_nvnGetProcAddress;
uintptr_t addr_nvnPresentTexture;
typedef void (*nvnQueuePresentTexture_0)(void* unk1_1, void* unk2_1, void* unk3_1);
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
	uintptr_t address = ((GetProcAddress)(ptr_nvnDeviceGetProcAddress))(unk1, nvnFunction);
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
		ptr_nvnDeviceGetProcAddress = nvnBootstrapLoader("nvnDeviceGetProcAddress");
		return addr_nvnGetProcAddress;
	}
	uintptr_t ptrret = nvnBootstrapLoader(nvnName);
	return ptrret;
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

	SaltySDCore_fclose(offsets);

	// clang-format off
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE",
		(void*) &GetSixAxisSensorHandle1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE",
		(void*) &GetSixAxisSensorHandle2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE",
		(void*) &GetSixAxisSensorHandles1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE",
		(void*) &GetSixAxisSensorHandles2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE",
		(void*) &GetSixAxisSensorState);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE",
		(void*) &GetSixAxisSensorStates1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE",
		(void*) &GetSixAxisSensorStates2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE",
		(void*) &IsSixAxisSensorAtRest);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE",
		(void*) &GetTouchScreenState1Touch);
	// clang-format on

	addr_nvnGetProcAddress = (uint64_t)&nvnGetProcAddress;
	addr_nvnPresentTexture = (uint64_t)&nvnPresentTexture;
	SaltySDCore_ReplaceImport("nvnBootstrapLoader", (void*)nvnBootstrapLoader_1);
	SaltySDCore_ReplaceImport("eglSwapBuffers", (void*)eglSwap);
	SaltySDCore_ReplaceImport("vkQueuePresentKHR", (void*)vulkanSwap);

	writeToLog("Injection finished\n");
}
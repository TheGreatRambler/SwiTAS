#include <cstring>
#include <saltysd/SaltySD_core.h>
#include <saltysd/SaltySD_dynamic.h>
#include <saltysd/SaltySD_ipc.h>
#include <string>
#include <switch_min.h>

#include "sdkTypes.hpp"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

extern "C" {
extern u32 __start__;

static char g_heap[0x10000];

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr);
void __attribute__((weak)) NORETURN __libnx_exit(int rc);
void __attribute__((weak)) __libnx_exception_handler(ThreadExceptionDump* ctx);
void __nx_exit(int, void*);
void __libc_fini_array(void);
void __libc_init_array(void);

// Functions for frame handling
extern u64 nvnBootstrapLoader(const char* nvnName) LINKABLE;
extern void eglSwapBuffers(void* egl_unk1, void* egl_unk2) LINKABLE;
extern u32 vkQueuePresentKHR(void* vk_unk1, void* vk_unk2) LINKABLE;

// Six Axis external functions
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(nn::hid::ConsoleSixAxisSensorHandle* handle) LINKABLE;
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) LINKABLE;
extern void _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) LINKABLE;
extern int32_t _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(nn::hid::SixAxisSensorHandle* handle, int32_t numOfHandles, const nn::hid::NpadIdType& id, int32_t npadStyleBitflags) LINKABLE;
extern void _ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, const nn::hid::SixAxisSensorHandle& handle) LINKABLE;
extern int32_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(nn::hid::SixAxisSensorState* param_1, int32_t param_2, const nn::hid::BasicXpadId& handle) LINKABLE;
extern int32_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, int32_t param_2, const nn::hid::SixAxisSensorHandle& handle) LINKABLE;
extern int32_t _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(const nn::hid::ConsoleSixAxisSensorHandle& param_1) LINKABLE;
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(const nn::hid::ConsoleSixAxisSensorHandle& param_1) LINKABLE;
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
// Touch external functions
extern int32_t _ZN2nn3hid6detail19GetTouchScreenStateILm16EEENS_6ResultEPNS0_16TouchScreenStateIXT_EEE(nn::hid::TouchScreenState* state) LINKABLE;
// Don't know why that int* is there, it's in the decomp
// THIS IS THE WRONG ONE
extern int32_t _ZN2nn3hid6detail20GetTouchScreenStatesILm16EEENS_6ResultEPiPNS0_16TouchScreenStateIXT_EEEi(int32_t* unk1, nn::hid::TouchScreenState* outStates, int32_t unk2) LINKABLE;
}

u32 __nx_applet_type = AppletType_None;
Handle orig_main_thread;
void* orig_ctx;
void* orig_saved_lr;

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr) {
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = &g_heap[0];
	fake_heap_end   = &g_heap[sizeof g_heap];

	orig_ctx         = ctx;
	orig_main_thread = main_thread;
	orig_saved_lr    = saved_lr;

	// Call constructors.
	// void __libc_init_array(void);
	__libc_init_array();
}

void __attribute__((weak)) NORETURN __libnx_exit(int rc) {
	// Call destructors.
	// void __libc_fini_array(void);
	__libc_fini_array();

	SaltySD_printf("SaltySD Plugin: jumping to %p\n", orig_saved_lr);
	/*

	const char* logPath = "sdmc:/SaltySD/SwiTAS_SaltyPlugin.log";
	SaltySDCore_remove(logPath);
	FILE* logFile = SaltySDCore_fopen(logPath, "w");

	logString += "Done";

	SaltySDCore_fwrite(logString.c_str(), logString.length(), 1, logFile);

	SaltySDCore_fclose(logFile);
	*/

	__nx_exit(0, orig_saved_lr);
	while(true)
		;
}

void __attribute__((weak)) __libnx_exception_handler(ThreadExceptionDump* ctx) {
	// I dunno
}

// Print log info
uint8_t dumpDebugInfo = true;
// Allow SwiTAS to edit the motion sent to the game
uint8_t spoofMotionRequests = false;
// This needs to be set to spoof
uint8_t recordInputs = false;

uint8_t frameHasPassed = false;

uint16_t logStringIndex = 0;
char logString[1000];

uint8_t wasJustLeft;
uint8_t wasJustTASController;

nn::hid::SixAxisSensorHandle sixAxisHandlesLeftJoycon[8] = { 0 };
nn::hid::SixAxisSensorHandle sixAxisHandlesRightJoycon[8] = { 0 };

// Update state of left and right joycons along with the touchscreen
nn::hid::SixAxisSensorState sixAxisStateLeftJoycon[8]                   = { 0 };
nn::hid::SixAxisSensorState sixAxisStateRightJoycon[8]                   = { 0 };
nn::hid::TouchState touchscreenState[8] = { 0 };

int32_t leftJoyconBacklogSize = 0;
int32_t rightJoyconBacklogSize = 0;
nn::hid::SixAxisSensorState sixAxisStateLeftJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax]                   = { 0 };
nn::hid::SixAxisSensorState sixAxisStateRightJoyconBacklog[8][nn::hid::SixAxisSensorStateCountMax]                   = { 0 };

// Updated with the real values, for recording purposes
nn::hid::TouchState originalTouchscreenStateBacklog[8][nn::hid::TouchStateCountMax] = { 0 };

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

void moveLeftBacklog() {
	// Get state as normal and move back backlog by 1
memmove(&sixAxisStateLeftJoyconBacklog[i][1], &sixAxisStateLeftJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * (nn::hid::SixAxisSensorStateCountMax - 1));
if (leftJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
	leftJoyconBacklogSize++;
}
}

void moveRightBacklog() {
	// Get state as normal and move back backlog by 1
memmove(&sixAxisStateRightJoyconBacklog[i][1], &sixAxisStateRightJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * (nn::hid::SixAxisSensorStateCountMax - 1));
if (rightJoyconBacklogSize != nn::hid::SixAxisSensorStateCountMax) {
	rightJoyconBacklogSize++;
}
}

void fixMotionState(nn::hid::SixAxisSensorState& dest, nn::hid::SixAxisSensorState& orig) {
	dest.deltaTimeNanoSeconds = orig.deltaTimeNanoSeconds;
	dest.samplingNumber       = orig.samplingNumber;
	// System default
	dest.direction.x = { 1.0, 0.0, 0.0 };
	dest.direction.y = { 0.0, 1.0, 0.0 };
	dest.direction.z = { 0.0, 0.0, 1.0 };
}

/*
void setTouchState(nn::hid::TouchScreenState& dest, nn::hid::TouchState[nn::hid::TouchStateCountMax] states, int32_t numOfStates) {
	memcpy(&dest.touches, states, sizeof(nn::hid::TouchState) * numOfStates);
	dest.count          = numOfStates;
	dest.samplingNumber = orig.samplingNumber;

	// Need to test these data points by holding my finger on the switch, todo
	dest[0].attributes           = orig[0].attributes;
	dest[0].deltaTimeNanoSeconds = orig[0].deltaTimeNanoSeconds;
	dest[0].diameterX            = orig[0].diameterX;
	dest[0].diameterY            = orig[0].diameterY;
	dest[0].fingerId             = 0;
	dest[0].rotationAngle        = orig[0].rotationAngle;
}
*/

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
		if (id != n::hid::NpadIdType::Unknown && id != n::hid::NpadIdType::Handheld) {
			// We're dealing with a potential hid:dbg controller
			// Check if it is dual joycons
			if (numOfHandles == 2) {
				int32_t res = _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handles, numOfHandles, id, npadStyleBitflags);

				sixAxisHandlesLeftJoycon[(int32_t)id] = handles[0];
				sixAxisHandlesRightJoycon[(int32_t)id] = handles[1];

				wasJustTASController = true;

				return res;
			}

		}
	}

// Simply passthrough if need be
	return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handle, numOfHandles, id, npadStyleBitflags);
}

/* nn::hid::GetSixAxisSensorState(nn::hid::SixAxisSensorState*, nn::hid::SixAxisSensorHandle const&)
 */
void GetSixAxisSensorState(nn::hid::SixAxisSensorState* state, const nn::hid::SixAxisSensorHandle& handle) {
	if(recordInputs) {
			for(int32_t i = 0; i < 8; i++) {
				if(sixAxisHandlesLeftJoycon[i] == handle) {
					_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);

					if(spoofMotionRequests) {
						fixMotionState(sixAxisStateLeftJoycon[i], *state);
						memcpy(state, &sixAxisStateLeftJoycon[i], sizeof(nn::hid::SixAxisSensorState));
					}

moveLeftBacklog();
					memcpy(&sixAxisStateLeftJoyconBacklog[i][0], state, sizeof(nn::hid::SixAxisSensorState));

					wasJustLeft = true;
					wasJustTASController = true;

					return;
				}

				if(sixAxisHandlesRightJoycon[i] == handle) {
					_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);

					if(spoofMotionRequests) {
						fixMotionState(sixAxisStateRightJoycon[i], *state);
						memcpy(state, &sixAxisStateRightJoycon[i], sizeof(nn::hid::SixAxisSensorState));
					}

moveRightBacklog();
					memcpy(&sixAxisStateRightJoyconBacklog[i][0], state, sizeof(nn::hid::SixAxisSensorState));

					wasJustLeft = false;
					wasJustTASController = true;

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

if (wasJustTASController) {
	// TAS controller being a kind of controller that would be handled by TAS
if (wasJustLeft) {
	int32_t backlogSize = max(count, leftJoyconBacklogSize);
memcpy(outStates, &sixAxisStateLeftJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * backlogSize);
return backlogSize;
} else {
	int32_t backlogSize = max(count, rightJoyconBacklogSize);
memcpy(outStates, &sixAxisStateRightJoyconBacklog[i][0], sizeof(nn::hid::SixAxisSensorState) * backlogSize);
return backlogSize;
}

	wasJustTASController = false;
} else {
	// This will trigger for controllers outside of the standard range, including pro controllers and single joycons
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
}
}

	}
		return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
}

/* nn::hid::IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle const&) */
int32_t IsSixAxisSensorAtRest(const nn::hid::SixAxisSensorHandle& param_1) {
	return _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::StartSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StartSixAxisSensor1(const nn::hid::ConsoleSixAxisSensorHandle& param_1) {
	_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StartSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StartSixAxisSensor2(const nn::hid::SixAxisSensorHandle& param_1) {
	// Standard used by games
	_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StopSixAxisSensor1(const nn::hid::ConsoleSixAxisSensorHandle& param_1) {
	_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StopSixAxisSensor2(const nn::hid::SixAxisSensorHandle& param_1) {
	_ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

/*
// Touch screen spoofing
// void nn::hid::GetTouchScreenState (TouchScreenState< N > *pOutValue)
// Dunno how to handle templates
void GetTouchScreenState(nn::hid::TouchScreenState* state) {

	// Return the most recent state
	// Each state can have multiple touches, indicated by the template
	// The touches are an array within the struct
	// It is an option just to handle the largest, 16
	// You can have up to 16 touches at a time
	// When called, set `count` to the number of TASed touchscreen inputs
	// The internal array is always as large as how it was defined, so likely 16
	// Just set the first touch state in the array and set count to 1 to TAS
	// May support multiple touches, I dunno

	if(recordInputs) {
		whateverthecallis(state);

		memcpy(&originalTouchscreenState, &state.touches, sizeof(originalTouchscreenState));

		if(spoofMotionRequests) {
			// Fix state and send to game
		}
	} else {
		whateverthecallis(state);

		if(canWriteToLog()) {
			if(state->count == 1) {
				nn::hid::TouchState& touchState = state->touches[0];

				// clang-format off
				std::string diagInfo =
					"attributes: " + std::to_string(touchState.attributes) + "\n" +
					"deltaTimeNanoSeconds: " + std::to_string(touchState.deltaTimeNanoSeconds) + "\n" +
					"diameterX: " + std::to_string(touchState.diameterX) + "\n" +
					"diameterY: " + std::to_string(touchState.diameterY) + "\n" +
					"fingerId: " + std::to_string(touchState.fingerId) + "\n" +
					"rotationAngle: " + std::to_string(touchState.rotationAngle) + "\n" +
					"x: " + std::to_string(touchState.x) + "\n" +
					"y: " + std::to_string(touchState.y) + "\n";
				// clang-format on

				writeToLog(diagInfo.c_str());
			}
		}
	}
}

// int nn::hid::GetTouchScreenStates (TouchScreenState< N > *outStates, int count)
int32_t GetTouchScreenStates(nn::hid::TouchScreenState* outStates, int32_t count) {
	// Return all past states, don't really want to use this one

	int32_t successfulWrittenStates;

	if(recordInputs) {
		successfulWrittenStates = whateverthecallis(outstates, count);

		memcpy(&originalTouchscreenState, &outStates[0].touches, sizeof(originalTouchscreenState));

		if(spoofMotionRequests) {
		}
	} else {
		return whateverthecallis(outstates, count);
	}
}
*/

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
	// writeToLog("Alive\n");

	const char* pointersPath = "sdmc:/SaltySD/SwiTAS_SaltyPlugin_Offsets.hex";
	FILE* offsets            = SaltySDCore_fopen(pointersPath, "wb");

	writePointerToFile(&frameHasPassed, offsets);
	writePointerToFile(&logStringIndex, offsets);
	writePointerToFile(&logString, offsets);
	writePointerToFile(&mainSixAxisState, offsets);
	writePointerToFile(&handheldSixAxisState, offsets);
	writePointerToFile(&originalMainSixAxisState, offsets);
	writePointerToFile(&originalHandheldSixAxisState, offsets);

	SaltySDCore_fclose(offsets);

	// clang-format off
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid25EnableSixAxisSensorFusionERKNS0_19SixAxisSensorHandleEb",
		(void*) &EnableSixAxisSensorFusion);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE",
		(void*) &GetSixAxisSensorFusionParameters);
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
		"_ZN2nn3hid30InitializeConsoleSixAxisSensorEv",
		(void*) &InitializeConsoleSixAxisSensor);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE",
		(void*) &IsSixAxisSensorAtRest);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE",
		(void*) &IsSixAxisSensorFusionEnabled);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE",
		(void*) &ResetSixAxisSensorFusionParameters);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff",
		(void*) &SetSixAxisSensorFusionParameters);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE",
		(void*) &StartSixAxisSensor1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE",
		(void*) &StartSixAxisSensor2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE",
		(void*) &StopSixAxisSensor1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE",
		(void*) &StopSixAxisSensor2);
	// clang-format on

	addr_nvnGetProcAddress = (uint64_t)&nvnGetProcAddress;
	addr_nvnPresentTexture = (uint64_t)&nvnPresentTexture;
	SaltySDCore_ReplaceImport("nvnBootstrapLoader", (void*)nvnBootstrapLoader_1);
	SaltySDCore_ReplaceImport("eglSwapBuffers", (void*)eglSwap);
	SaltySDCore_ReplaceImport("vkQueuePresentKHR", (void*)vulkanSwap);

	// writeToLog("Injection finished\n");
}
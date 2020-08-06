#include <cstring>
#include <saltysd/SaltySD_core.h>
#include <saltysd/SaltySD_dynamic.h>
#include <saltysd/SaltySD_ipc.h>
#include <switch_min.h>

#include "sdkTypes.hpp"

extern "C" {
extern u32 __start__;

static char g_heap[0x8000];

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr);
void __attribute__((weak)) NORETURN __libnx_exit(int rc);
void __nx_exit(int, void*);
void __libc_fini_array(void);
void __libc_init_array(void);

// Six Axis external functions
extern void _ZN2nn3hid25EnableSixAxisSensorFusionERKNS0_19SixAxisSensorHandleEb(nn::hid::SixAxisSensorHandle* handle, bool param_2) LINKABLE;
extern void _ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE(float* param_1, float* param_2, nn::hid::SixAxisSensorHandle* handle) LINKABLE;
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(nn::hid::ConsoleSixAxisSensorHandle* handle) LINKABLE;
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) LINKABLE;
extern void _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) LINKABLE;
extern uint64_t _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(nn::hid::SixAxisSensorHandle* handleArr, int numOfControllers, nn::hid::NpadIdType id, uint32_t npadStyleBitflags) LINKABLE;
extern void _ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, nn::hid::SixAxisSensorHandle* param_2) LINKABLE;
extern uint64_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::BasicXpadId* param_3) LINKABLE;
extern uint64_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::SixAxisSensorHandle* param_3) LINKABLE;
extern void _ZN2nn3hid30InitializeConsoleSixAxisSensorEv(void) LINKABLE;
extern uint64_t _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorHandle* param_1) LINKABLE;
extern uint64_t _ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorHandle* param_1) LINKABLE;
extern void _ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorHandle* param_1) LINKABLE;
extern void _ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff(nn::hid::SixAxisSensorHandle* param_1, float param_2, float param_3) LINKABLE;
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(nn::hid::ConsoleSixAxisSensorHandle* param_1) LINKABLE;
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorHandle* param_1) LINKABLE;
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(nn::hid::ConsoleSixAxisSensorHandle* param_1) LINKABLE;
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorHandle* param_1) LINKABLE;
// Filesystem external functions
// Result OpenFile(nn::fs::FileHandle *, char const* path, s32);
extern uint64_t _ZN2nn2fs8OpenFileEPNS0_10FileHandleEPKci(nn::fs::FileHandle* handle, char const* path, int32_t mode) LINKABLE;
// void CloseFile(FileHandle fileHandle);
extern void _ZN2nn2fs9CloseFileENS0_10FileHandleE(nn::fs::FileHandle handle) LINKABLE;
// Result ReadFile(u64 *, nn::fs::FileHandle, s64, void *, u64);
extern void _ZN2nn2fs8ReadFileENS0_10FileHandleElPvm(nn::fs::FileHandle handle, int64_t offset, void* data, uint64_t length) LINKABLE;
// Result WriteFile(FileHandle handle, s64 fileOffset, void const* buff, u64 size, WriteOption const& option);
extern uint64_t _ZN2nn2fs9WriteFileENS0_10FileHandleElPKvmRKNS0_11WriteOptionE(nn::fs::FileHandle handle, int64_t fileOffset, void* buf, int64_t size, nn::fs::WriteOption options) LINKABLE;
// Result GetFileSize(s64* size, FileHandle fileHandle);
extern uint64_t _ZN2nn2fs11GetFileSizeEPlNS0_10FileHandleE(int64_t* size, nn::fs::FileHandle fileHandle) LINKABLE;
// Result SetFileSize(FileHandle fileHandle, s64 filesize);
extern uint64_t _ZN2nn2fs11SetFileSizeENS0_10FileHandleEl(nn::fs::FileHandle fileHandle, int64_t filesize) LINKABLE;
}

u32 __nx_applet_type = AppletType_None;
Handle orig_main_thread;
void* orig_ctx;
void* orig_saved_lr;

// Print log info
uint8_t dumpDebugInfo = true;
// Allow SwiTAS to edit the motion sent to the game
uint8_t spoofMotionRequests = false;
// This needs to be set to spoof
uint8_t recordMotionInputs = false;

nn::fs::FileHandle logHandle;

nn::hid::SixAxisSensorHandle* mainHandles[8] = { 0 };
nn::hid::SixAxisSensorHandle* handheldHandle = { 0 };

// Updated externally by SwiTAS
nn::hid::SixAxisSensorState mainSixAxisState[8]  = { 0 };
nn::hid::SixAxisSensorState handheldSixAxisState = { 0 };

// Updated with the real values, for recording purposes
nn::hid::SixAxisSensorState originalMainSixAxisState[8]  = { 0 };
nn::hid::SixAxisSensorState originalHandheldSixAxisState = { 0 };

void openLogFile() {
	_ZN2nn2fs8OpenFileEPNS0_10FileHandleEPKci(&logHandle, "sdmc:/SaltySD/SwiTAS_MotionPlugin.log", nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append);
}

void closeLogFile() {
	_ZN2nn2fs9CloseFileENS0_10FileHandleE(logHandle);
}

void writeToFile(const char* str) {
	// https://github.com/skyline-dev/skyline/blob/master/source/skyline/utils/cpputils.cpp#L128
	int64_t strLength = strlen(str);

	int64_t fileSize;
	_ZN2nn2fs11GetFileSizeEPlNS0_10FileHandleE(&fileSize, logHandle);

	if(fileSize < strLength) { // make sure we have enough space
		_ZN2nn2fs11SetFileSizeENS0_10FileHandleEl(logHandle, strLength);
	}

	_ZN2nn2fs9WriteFileENS0_10FileHandleElPKvmRKNS0_11WriteOptionE(logHandle, 0, (void*)str, strLength, nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));
}

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

	closeLogFile();

	__nx_exit(0, orig_saved_lr);
	while(true)
		;
}

/* nn::hid::EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle const&, bool) */
void EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle* handle, bool param_2) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: EnableSixAxisSensorFusion called\n");
	}
	_ZN2nn3hid25EnableSixAxisSensorFusionERKNS0_19SixAxisSensorHandleEb(handle, param_2);
}

/* nn::hid::GetSixAxisSensorFusionParameters(float*, float*, nn::hid::SixAxisSensorHandle const&) */
void GetSixAxisSensorFusionParameters(float* param_1, float* param_2, nn::hid::SixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE(param_1, param_2, handle);
}

/* nn::hid::GetSixAxisSensorHandle(nn::hid::ConsoleSixAxisSensorHandle*) */
void GetSixAxisSensorHandle1(nn::hid::ConsoleSixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorHandle1 called\n");
	}
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(handle);
}

/* nn::hid::GetSixAxisSensorHandle(nn::hid::SixAxisSensorHandle*, nn::hid::BasicXpadId) */
void GetSixAxisSensorHandle2(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorHandle2 called\n");
	}
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(handle, param_2);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, nn::hid::SixAxisSensorHandle*,
   nn::hid::JoyXpadId) */
void GetSixAxisSensorHandles1(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorHandles1 called\n");
	}
	_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(handle1, handle2, param_3);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, int, unsigned int const&,
   nn::util::BitFlagSet<32, nn::hid::NpadStyleTag>) */
uint64_t GetSixAxisSensorHandles2(nn::hid::SixAxisSensorHandle* handle, int numOfHandles, nn::hid::NpadIdType id, uint32_t npadStyleBitflags) {
	// Ignore numOfHandles
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorHandles2 called\n");
	}
	// To see what kind of controller, and the bit flags with the chosen NpadStyleTag
	// if (npadStyleBitflags & nn::hid::NpadStyleTag::ProController)
	// Returned number is the number of successful handles

	if(recordMotionInputs) {
		if(id == nn::hid::NpadIdType::Handheld) {
			handheldHandle = handle;
		} else {
			if((uint32_t)id < 8) {
				mainHandles[(uint32_t)id] = handle;
			} else {
				if(dumpDebugInfo) {
					writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorHandles2: Invalid ID\n");
				}
			}
		}

		// Always handle one handle, even if the game requests more
		return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handle, 1, id, npadStyleBitflags);
	} else {
		return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handle, numOfHandles, id, npadStyleBitflags);
	}
}

/* nn::hid::GetSixAxisSensorState(nn::hid::SixAxisSensorState*, nn::hid::SixAxisSensorHandle const&)
 */
void GetSixAxisSensorState(nn::hid::SixAxisSensorState* state, nn::hid::SixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorState called\n");
	}

	if(recordMotionInputs) {
		if(handle == handheldHandle) {
			_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(&originalHandheldSixAxisState, handle);

			if(spoofMotionRequests) {
				handheldSixAxisState.samplingNumber = originalHandheldSixAxisState.samplingNumber;
				memcpy(state, &handheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
			} else {
				memcpy(state, &originalHandheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
			}
		} else {
			for(uint32_t i = 0; i < 8; i++) {
				if(mainHandles[i] == handle) {
					_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(&originalMainSixAxisState[i], handle);

					if(spoofMotionRequests) {
						mainSixAxisState[i].samplingNumber = originalMainSixAxisState[i].samplingNumber;
						memcpy(state, &mainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
						return;
					} else {
						memcpy(state, &originalMainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
						return;
					}
				}
			}
		}
		if(dumpDebugInfo && spoofMotionRequests) {
			writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorState: Corresponding handle was not found\n");
		}
	} else {
		_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(state, handle);
	}
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::BasicXpadId const&)
 */
uint64_t GetSixAxisSensorStates1(nn::hid::SixAxisSensorState* outStates, int count, nn::hid::BasicXpadId* handle) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorStates1 called\n");
	}
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(outStates, count, handle);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::SixAxisSensorHandle
   const&) */
uint64_t GetSixAxisSensorStates2(nn::hid::SixAxisSensorState* outStates, int count, nn::hid::SixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorStates2 called\n");
	}

	if(recordMotionInputs) {

		// Pretty certain a few parts of each struct have to be modified to make this work
		uint64_t successfulWrittenStates;

		if(handle == handheldHandle) {
			// Initially copy all states
			successfulWrittenStates = _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
			//_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(&originalHandheldSixAxisState, handle);

			// Copy first element into buffer
			memcpy(&originalHandheldSixAxisState, &outStates[0], sizeof(nn::hid::SixAxisSensorState));

			/*
					for(int stateNum = 0; stateNum < count; stateNum++) {
						if(spoofMotionRequests) {
							handheldSixAxisState.samplingNumber = originalHandheldSixAxisState.samplingNumber;
							memcpy(&outStates[stateNum], &handheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
						} else {
							memcpy(&outStates[stateNum], &originalHandheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
						}
						// Pretend to take it back by 1 every time
						originalHandheldSixAxisState.samplingNumber -= 1;
					}
					*/
			// Only attempt the first one
			if(spoofMotionRequests) {
				handheldSixAxisState.samplingNumber = originalHandheldSixAxisState.samplingNumber;
				memcpy(&outStates[0], &handheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
			} else {
				memcpy(&outStates[0], &originalHandheldSixAxisState, sizeof(nn::hid::SixAxisSensorState));
			}

			return successfulWrittenStates;
		} else {
			for(uint32_t i = 0; i < 8; i++) {
				if(mainHandles[i] == handle) {
					// Initially copy all states
					successfulWrittenStates = _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);

					// Copy first element into buffer
					memcpy(&originalMainSixAxisState[i], &outStates[0], sizeof(nn::hid::SixAxisSensorState));

					/*
									for(int stateI = 0; stateI < count; i++) {
										if(spoofMotionRequests) {
											mainSixAxisState[i].samplingNumber = originalMainSixAxisState[i].samplingNumber;
											memcpy(&outStates[stateI], &mainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
										} else {
											memcpy(&outStates[stateI], &originalMainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
										}
										// Pretend to take it back by 1 every time
										originalMainSixAxisState[i].samplingNumber -= 1;
									}
									*/

					if(spoofMotionRequests) {
						mainSixAxisState[i].samplingNumber = originalMainSixAxisState[i].samplingNumber;
						memcpy(&outStates[0], &mainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
					} else {
						memcpy(&outStates[0], &originalMainSixAxisState[i], sizeof(nn::hid::SixAxisSensorState));
					}

					return successfulWrittenStates;
				}
			}
			if(dumpDebugInfo && spoofMotionRequests) {
				writeToFile("SwiTAS_MotionPlugin: GetSixAxisSensorState: Corresponding handle was not found\n");
			}
		}

		// Should not get here, just go back to original
		return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);

		// return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
		// Maybe the count of states, I dunno
		// return count;
	} else {
		return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(outStates, count, handle);
	}
}

/* nn::hid::InitializeConsoleSixAxisSensor() */
void InitializeConsoleSixAxisSensor(void) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: InitializeConsoleSixAxisSensor called\n");
	}
	_ZN2nn3hid30InitializeConsoleSixAxisSensorEv();
}

/* nn::hid::IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: IsSixAxisSensorAtRest called\n");
	}
	return _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: IsSixAxisSensorFusionEnabled called\n");
	}
	return _ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&) */
void ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: ResetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&, float, float) */
void SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1, float param_2, float param_3) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: SetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff(param_1, param_2, param_3);
}

/* nn::hid::StartSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StartSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: StartSixAxisSensor1 called\n");
	}
	_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StartSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StartSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: StartSixAxisSensor2 called\n");
	}
	// Standard used by games
	_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StopSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: StopSixAxisSensor1 called\n");
	}
	_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StopSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		writeToFile("SwiTAS_MotionPlugin: StopSixAxisSensor2 called\n");
	}
	_ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

int main(int argc, char* argv[]) {
	SaltySD_printf("SwiTAS_MotionPlugin: alive\n");

	const char* pointersPath = "sdmc:/SaltySD/SwiTAS_MotionPlugin_Offsets.hex";
	SaltySDCore_remove(pointersPath);
	FILE* offsets = SaltySDCore_fopen(pointersPath, "wb");

	uint64_t mainSixAxisStateAddr = (uint64_t)&mainSixAxisState;
	SaltySDCore_fwrite(&mainSixAxisStateAddr, sizeof(mainSixAxisStateAddr), 1, offsets);

	uint64_t handheldSixAxisStateAddr = (uint64_t)&handheldSixAxisState;
	SaltySDCore_fwrite(&handheldSixAxisStateAddr, sizeof(handheldSixAxisStateAddr), 1, offsets);

	uint64_t originalMainSixAxisStateAddr = (uint64_t)&originalMainSixAxisState;
	SaltySDCore_fwrite(&originalMainSixAxisStateAddr, sizeof(originalMainSixAxisStateAddr), 1, offsets);

	uint64_t originalHandheldSixAxisStateAddr = (uint64_t)&originalHandheldSixAxisState;
	SaltySDCore_fwrite(&originalHandheldSixAxisStateAddr, sizeof(originalHandheldSixAxisStateAddr), 1, offsets);

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

	SaltySD_printf("SwiTAS_MotionPlugin: injection finished\n");

	openLogFile();
}
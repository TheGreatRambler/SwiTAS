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

	__nx_exit(0, orig_saved_lr);
	while(true)
		;
}

uint8_t dumpDebugInfo = true;

/* nn::hid::EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle const&, bool) */
void EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle* handle, bool param_2) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: EnableSixAxisSensorFusion called\n");
	}
	_ZN2nn3hid25EnableSixAxisSensorFusionERKNS0_19SixAxisSensorHandleEb(handle, param_2);
}

/* nn::hid::GetSixAxisSensorFusionParameters(float*, float*, nn::hid::SixAxisSensorHandle const&) */
void GetSixAxisSensorFusionParameters(float* param_1, float* param_2, nn::hid::SixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE(param_1, param_2, handle);
}

/* nn::hid::GetSixAxisSensorHandle(nn::hid::ConsoleSixAxisSensorHandle*) */
void GetSixAxisSensorHandle1(nn::hid::ConsoleSixAxisSensorHandle* handle) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorHandle1 called\n");
	}
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(handle);
}

/* nn::hid::GetSixAxisSensorHandle(nn::hid::SixAxisSensorHandle*, nn::hid::BasicXpadId) */
void GetSixAxisSensorHandle2(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorHandle2 called\n");
	}
	_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(handle, param_2);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, nn::hid::SixAxisSensorHandle*,
   nn::hid::JoyXpadId) */
void GetSixAxisSensorHandles1(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorHandles1 called\n");
	}
	_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(handle1, handle2, param_3);
}

/* nn::hid::GetSixAxisSensorHandles(nn::hid::SixAxisSensorHandle*, int, unsigned int const&,
   nn::util::BitFlagSet<32, nn::hid::NpadStyleTag>) */
uint64_t GetSixAxisSensorHandles2(nn::hid::SixAxisSensorHandle* handleArr, int numOfControllers, nn::hid::NpadIdType id, uint32_t npadStyleBitflags) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorHandles2 called\n");
	}
	return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(handleArr, numOfControllers, id, npadStyleBitflags);
}

/* nn::hid::GetSixAxisSensorState(nn::hid::SixAxisSensorState*, nn::hid::SixAxisSensorHandle const&)
 */
void GetSixAxisSensorState(nn::hid::SixAxisSensorState* param_1, nn::hid::SixAxisSensorHandle* param_2) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorState called\n");
	}
	_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(param_1, param_2);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::BasicXpadId const&)
 */
uint64_t GetSixAxisSensorStates1(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::BasicXpadId* param_3) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorStates1 called\n");
	}
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(param_1, param_2, param_3);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::SixAxisSensorHandle
   const&) */
uint64_t GetSixAxisSensorStates2(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::SixAxisSensorHandle* param_3) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: GetSixAxisSensorStates2 called\n");
	}
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(param_1, param_2, param_3);
}

/* nn::hid::InitializeConsoleSixAxisSensor() */
void InitializeConsoleSixAxisSensor(void) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: InitializeConsoleSixAxisSensor called\n");
	}
	_ZN2nn3hid30InitializeConsoleSixAxisSensorEv();
}

/* nn::hid::IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: IsSixAxisSensorAtRest called\n");
	}
	return _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: IsSixAxisSensorFusionEnabled called\n");
	}
	return _ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&) */
void ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: ResetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&, float, float) */
void SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1, float param_2, float param_3) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: SetSixAxisSensorFusionParameters called\n");
	}
	_ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff(param_1, param_2, param_3);
}

/* nn::hid::StartSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StartSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: StartSixAxisSensor1 called\n");
	}
	_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StartSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StartSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: StartSixAxisSensor2 called\n");
	}
	_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StopSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: StopSixAxisSensor1 called\n");
	}
	_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StopSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
	if(dumpDebugInfo) {
		SaltySD_printf("SwiTAS_MotionPlugin: StopSixAxisSensor2 called\n");
	}
	_ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

int main(int argc, char* argv[]) {
	SaltySD_printf("SwiTAS_MotionPlugin: alive\n");
	/*
	uint64_t addr_FPS = (uint64_t)&FPS;
	FILE* offset      = SaltySDCore_fopen("sdmc:/SaltySD/FPSoffset.hex", "wb");
	SaltySDCore_fwrite(&addr_FPS, 0x5, 1, offset);
	SaltySDCore_fclose(offset);
	addr_nvnGetProcAddress = (uint64_t)&nvnGetProcAddress;
	addr_nvnPresentTexture = (uint64_t)&nvnPresentTexture;
	SaltySDCore_ReplaceImport("nvnBootstrapLoader", (void*)nvnBootstrapLoader_1);
	SaltySDCore_ReplaceImport("eglSwapBuffers", (void*)eglSwap);
	SaltySDCore_ReplaceImport("vkQueuePresentKHR", (void*)vulkanSwap);
	*/
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
}
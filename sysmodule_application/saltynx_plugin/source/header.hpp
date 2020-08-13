#pragma once

#include <saltysd/SaltySD_core.h>
#include <saltysd/SaltySD_dynamic.h>
#include <saltysd/SaltySD_ipc.h>
#include <switch_min.h>

#include "sdkTypes.hpp"

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
// nn::hid::GetSixAxisSensorHandle
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE(nn::hid::ConsoleSixAxisSensorHandle* handle) LINKABLE;
// nn::hid::GetSixAxisSensorHandle
extern void _ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE(nn::hid::SixAxisSensorHandle* handle, nn::hid::BasicXpadId param_2) LINKABLE;
// nn::hid::GetSixAxisSensorHandles
extern void _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE(nn::hid::SixAxisSensorHandle* handle1, nn::hid::SixAxisSensorHandle* handle2, nn::hid::JoyXpadId param_3) LINKABLE;
// nn::hid::GetSixAxisSensorHandles
extern int32_t _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(nn::hid::SixAxisSensorHandle* handle, int32_t numOfHandles, const nn::hid::NpadIdType& id, int32_t npadStyleBitflags) LINKABLE;
// nn::hid::GetSixAxisSensorState
extern void _ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, const nn::hid::SixAxisSensorHandle& handle) LINKABLE;
// nn::hid::GetSixAxisSensorStates
extern int32_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(nn::hid::SixAxisSensorState* param_1, int32_t param_2, const nn::hid::BasicXpadId& handle) LINKABLE;
// nn::hid::GetSixAxisSensorStates
extern int32_t _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(nn::hid::SixAxisSensorState* param_1, int32_t param_2, const nn::hid::SixAxisSensorHandle& handle) LINKABLE;
// nn::hid::GetSixAxisSensorAsRest
extern int32_t _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
// nn::hid::StartSixAxisSensor
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(const nn::hid::ConsoleSixAxisSensorHandle& param_1) LINKABLE;
// nn::hid::StartSixAxisSensor
extern void _ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
// nn::hid::StopSixAxisSensor
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(const nn::hid::ConsoleSixAxisSensorHandle& param_1) LINKABLE;
// nn::hid::StopSixAxisSensor
extern void _ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE(const nn::hid::SixAxisSensorHandle& param_1) LINKABLE;
// Touch external functions
// nn::hid::GetTouchScreenState<1>
extern void _ZN2nn3hid19GetTouchScreenStateILm1EEEvPNS0_16TouchScreenStateIXT_EEE(nn::hid::TouchScreenState1Touch* state) LINKABLE;
// nn::hid::GetTouchScreenState<5>
extern void _ZN2nn3hid19GetTouchScreenStateILm5EEEvPNS0_16TouchScreenStateIXT_EEE(nn::hid::TouchScreenState5Touch* state) LINKABLE;
// nn::hid::GetTouchScreenState<16>
extern void _ZN2nn3hid19GetTouchScreenStateILm16EEEvPNS0_16TouchScreenStateIXT_EEE(nn::hid::TouchScreenState16Touch* state) LINKABLE;
// System functions
// nn::os::GetSystemTick
extern uint64_t _ZN2nn2os13GetSystemTickEv() LINKABLE;
// Mouse and keyboard external functions
// nn::hid::GetKeyboardState
// extern void _ZN2nn3hid16GetKeyboardStateEPNS0_13KeyboardStateE(nn::hid::KeyboardState* state) LINKABLE;
// nn::hid::GetKeyboardStates
// extern int32_t _ZN2nn3hid17GetKeyboardStatesEPNS0_13KeyboardStateEi(nn::hid::KeyboardState* outStates, int32_t count) LINKABLE;
// nn::hid::GetMouseState
// extern void _ZN2nn3hid13GetMouseStateEPNS0_10MouseStateE(nn::hid::MouseState* state) LINKABLE;
// nn::hid::GetMouseStates
// extern int32_t _ZN2nn3hid14GetMouseStatesEPNS0_10MouseStateEi(nn::hid::MouseState* outStates, int32_t count) LINKABLE;
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

void __attribute__((weak)) __libnx_exception_handler(ThreadExceptionDump* ctx) {
	// I dunno
}
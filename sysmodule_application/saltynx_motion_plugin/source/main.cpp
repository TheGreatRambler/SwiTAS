#include <saltysd/SaltySD_core.h>
#include <saltysd/SaltySD_dynamic.h>
#include <saltysd/SaltySD_ipc.h>
#include <switch_min.h>
#include <type_traits>

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
extern uint64_t _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(nn::hid::SixAxisSensorHandle* param_1, int param_2, uint* param_3, nn::util::BitFlagSet<32, nn::hid::NpadStyleTag> param_4) LINKABLE;
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

SixAxisSensorValues spoofedMotionValues;

/*

uint8_t FPS  = 0xFF;
float FPSavg = 255;
uintptr_t ptr_nvnDeviceGetProcAddress;
uintptr_t ptr_nvnQueuePresentTexture;
uintptr_t addr_nvnGetProcAddress;
uintptr_t addr_nvnPresentTexture;
float systemtickfrequency = 19200000;
typedef void (*nvnQueuePresentTexture_0)(void* unk1_1, void* unk2_1, void* unk3_1);
typedef uintptr_t (*GetProcAddress)(void* unk1_a, const char* nvnFunction_a);

uint32_t vulkanSwap(void* vk_unk1_1, void* vk_unk2_1) {
	static uint8_t FPS_temp    = 0;
	static uint64_t starttick  = 0;
	static uint64_t endtick    = 0;
	static uint64_t deltatick  = 0;
	static uint64_t frameend   = 0;
	static uint64_t framedelta = 0;
	static uint64_t frameavg   = 0;

	if(starttick == 0)
		starttick = _ZN2nn2os13GetSystemTickEv();
	uint32_t vulkanResult = vkQueuePresentKHR(vk_unk1_1, vk_unk2_1);
	endtick               = _ZN2nn2os13GetSystemTickEv();
	framedelta            = endtick - frameend;
	frameavg              = ((9 * frameavg) + framedelta) / 10;
	FPSavg                = systemtickfrequency / (float)frameavg;
	frameend              = endtick;

	FPS_temp++;
	deltatick = endtick - starttick;
	if(deltatick >= 19200000) {
		starttick = _ZN2nn2os13GetSystemTickEv();
		FPS       = FPS_temp - 1;
		FPS_temp  = 0;
	}

	return vulkanResult;
}

void eglSwap(void* egl_unk1_1, void* egl_unk2_1) {
	static uint8_t FPS_temp    = 0;
	static uint64_t starttick  = 0;
	static uint64_t endtick    = 0;
	static uint64_t deltatick  = 0;
	static uint64_t frameend   = 0;
	static uint64_t framedelta = 0;
	static uint64_t frameavg   = 0;

	if(starttick == 0)
		starttick = _ZN2nn2os13GetSystemTickEv();
	eglSwapBuffers(egl_unk1_1, egl_unk2_1);
	endtick    = _ZN2nn2os13GetSystemTickEv();
	framedelta = endtick - frameend;
	frameavg   = ((9 * frameavg) + framedelta) / 10;
	FPSavg     = systemtickfrequency / (float)frameavg;
	frameend   = endtick;

	FPS_temp++;
	deltatick = endtick - starttick;
	if(deltatick >= 19200000) {
		starttick = _ZN2nn2os13GetSystemTickEv();
		FPS       = FPS_temp - 1;
		FPS_temp  = 0;
	}

	return;
}

void nvnPresentTexture(void* unk1, void* unk2, void* unk3) {
	static uint8_t FPS_temp    = 0;
	static uint64_t starttick  = 0;
	static uint64_t endtick    = 0;
	static uint64_t deltatick  = 0;
	static uint64_t frameend   = 0;
	static uint64_t framedelta = 0;
	static uint64_t frameavg   = 0;

	if(starttick == 0)
		starttick = _ZN2nn2os13GetSystemTickEv();
	((nvnQueuePresentTexture_0)(ptr_nvnQueuePresentTexture))(unk1, unk2, unk3);
	endtick    = _ZN2nn2os13GetSystemTickEv();
	framedelta = endtick - frameend;
	frameavg   = ((9 * frameavg) + framedelta) / 10;
	FPSavg     = systemtickfrequency / (float)frameavg;
	frameend   = endtick;

	FPS_temp++;
	deltatick = endtick - starttick;
	if(deltatick >= 19200000) {
		starttick = _ZN2nn2os13GetSystemTickEv();
		FPS       = FPS_temp - 1;
		FPS_temp  = 0;
	}

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

int main(int argc, char* argv[]) {
	SaltySD_printf("NX-FPS: alive\n");
	uint64_t addr_FPS = (uint64_t)&FPS;
	FILE* offset      = SaltySDCore_fopen("sdmc:/SaltySD/FPSoffset.hex", "wb");
	SaltySDCore_fwrite(&addr_FPS, 0x5, 1, offset);
	SaltySDCore_fclose(offset);
	addr_nvnGetProcAddress = (uint64_t)&nvnGetProcAddress;
	addr_nvnPresentTexture = (uint64_t)&nvnPresentTexture;
	SaltySDCore_ReplaceImport("nvnBootstrapLoader", (void*)nvnBootstrapLoader_1);
	SaltySDCore_ReplaceImport("eglSwapBuffers", (void*)eglSwap);
	SaltySDCore_ReplaceImport("vkQueuePresentKHR", (void*)vulkanSwap);
	SaltySD_printf("NX-FPS: injection finished\n");
}

/*
nn:hid::StartSixAxisSensor:
_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE

nn::hid::GetSixAxisSensorStates:
_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE

nn::hid::GetSixAxisSensorHandles:
_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE
*/

/* nn::hid::EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle const&, bool) */

namespace nn {
	namespace hid {
		// nn.hid.GyroscopeZeroDriftMode
		struct GyroscopeZeroDriftMode {
			// System.Int32 nn.hid.GyroscopeZeroDriftMode::value__
			int32_t value;
		};

		// nn.hid.DirectionState
		struct DirectionState {
			// nn.util.Float3 nn.hid.DirectionState::x
			nn::util::Float3 x;
			// nn.util.Float3 nn.hid.DirectionState::y
			nn::util::Float3 y;
			// nn.util.Float3 nn.hid.DirectionState::z
			nn::util::Float3 z;
		};

		// nn.hid.SixAxisSensorState
		struct SixAxisSensorState {
			// System.Int64 nn.hid.SixAxisSensorState::deltaTimeNanoSeconds
			int64_t deltaTimeNanoSeconds;
			// System.Int64 nn.hid.SixAxisSensorState::samplingNumber
			int64_t samplingNumber;
			// nn.util.Float3 nn.hid.SixAxisSensorState::acceleration
			nn::util::Float3 acceleration;
			// nn.util.Float3 nn.hid.SixAxisSensorState::angularVelocity
			nn::util::Float3 angularVelocity;
			// nn.util.Float3 nn.hid.SixAxisSensorState::angle
			nn::util::Float3 angle;
			// nn.hid.DirectionState nn.hid.SixAxisSensorState::direction
			nn::hid::DirectionState direction;
			// nn.hid.SixAxisSensorAttribute nn.hid.SixAxisSensorState::attributes
			int32_t attributes;
		};

		typedef uint32_t BasicXpadId;
		typedef uint32_t JoyXpadId;
		typedef uint32_t SixAxisSensorHandle;
		typedef uint32_t ConsoleSixAxisSensorHandle;

		enum NpadStyleTag : uint32_t {
			ProController = BIT(0),
			Handheld      = BIT(1),
			JoyconPair    = BIT(2),
			JoyconLeft    = BIT(3),
			JoyconRight   = BIT(4),
			SystemExt     = BIT(29),
			System        = BIT(30),
		};
	};

	namespace util {
		// nn.util.Float3
		struct Float3 {
			// System.Single nn.util.Float3::x
			float x;
			// System.Single nn.util.Float3::y
			float y;
			// System.Single nn.util.Float3::z
			float z;
		};

		template <typename T> inline constexpr T AlignUp(T value, u64 align) {
			const auto inv_mask = align - 1;
			return static_cast<T>((value + inv_mask) & ~inv_mask);
		};

		template <size_t N, typename T = void> struct BitFlagSet {
			using Storage                           = typename std::conditional<N <= BITSIZEOF(uint32_t), uint32_t, uint64_t>::type;
			static constexpr size_t StorageBitCount = BITSIZEOF(Storage);
			static constexpr size_t StorageCount    = util::AlignUp(N, StorageBitCount) / StorageBitCount;
			Storage _storage[StorageCount];
		};
	};
};

/* nn::hid::EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle const&, bool) */
void EnableSixAxisSensorFusion(nn::hid::SixAxisSensorHandle* handle, bool param_2) {
	_ZN2nn3hid25EnableSixAxisSensorFusionERKNS0_19SixAxisSensorHandleEb(handle, param_2);
}

/* nn::hid::GetSixAxisSensorFusionParameters(float*, float*, nn::hid::SixAxisSensorHandle const&) */
void GetSixAxisSensorFusionParameters(float* param_1, float* param_2, nn::hid::SixAxisSensorHandle* handle) {
	_ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE(param_1, param_2, handle);
}

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
uint64_t GetSixAxisSensorHandles2(nn::hid::SixAxisSensorHandle* param_1, int param_2, uint* param_3, nn::util::BitFlagSet<32, nn::hid::NpadStyleTag> param_4) {
	return _ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE(param_1, param_2, param_3, param_4);
}

/* nn::hid::GetSixAxisSensorState(nn::hid::SixAxisSensorState*, nn::hid::SixAxisSensorHandle const&)
 */
void GetSixAxisSensorState(nn::hid::SixAxisSensorState* param_1, nn::hid::SixAxisSensorHandle* param_2) {
	_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE(param_1, param_2);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::BasicXpadId const&)
 */
uint64_t GetSixAxisSensorStates1(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::BasicXpadId* param_3) {
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE(param_1, param_2, param_3);
}

/* nn::hid::GetSixAxisSensorStates(nn::hid::SixAxisSensorState*, int, nn::hid::SixAxisSensorHandle
   const&) */
uint64_t GetSixAxisSensorStates2(nn::hid::SixAxisSensorState* param_1, int param_2, nn::hid::SixAxisSensorHandle* param_3) {
	return _ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE(param_1, param_2, param_3);
}

/* nn::hid::InitializeConsoleSixAxisSensor() */
void InitializeConsoleSixAxisSensor(void) {
	_ZN2nn3hid30InitializeConsoleSixAxisSensorEv();
}

/* nn::hid::IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorAtRest(nn::hid::SixAxisSensorHandle* param_1) {
	return _ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle const&) */
uint64_t IsSixAxisSensorFusionEnabled(nn::hid::SixAxisSensorHandle* param_1) {
	return _ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&) */
void ResetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1) {
	_ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle const&, float, float) */
void SetSixAxisSensorFusionParameters(nn::hid::SixAxisSensorHandle* param_1, float param_2, float param_3) {
	_ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff(param_1, param_2, param_3);
}

/* nn::hid::StartSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StartSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StartSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StartSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
	_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::ConsoleSixAxisSensorHandle const&) */
void StopSixAxisSensor1(nn::hid::ConsoleSixAxisSensorHandle* param_1) {
	_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE(param_1);
}

/* nn::hid::StopSixAxisSensor(nn::hid::SixAxisSensorHandle const&) */
void StopSixAxisSensor2(nn::hid::SixAxisSensorHandle* param_1) {
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
		&EnableSixAxisSensorFusion);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid32GetSixAxisSensorFusionParametersEPfS1_RKNS0_19SixAxisSensorHandleE",
		&GetSixAxisSensorFusionParameters);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_26ConsoleSixAxisSensorHandleE",
		&GetSixAxisSensorHandle1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorHandleEPNS0_19SixAxisSensorHandleENS0_11BasicXpadIdE",
		&GetSixAxisSensorHandle2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleES2_NS0_9JoyXpadIdE",
		&GetSixAxisSensorHandles1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE",
		&GetSixAxisSensorHandles2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21GetSixAxisSensorStateEPNS0_18SixAxisSensorStateERKNS0_19SixAxisSensorHandleE",
		&GetSixAxisSensorState);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_11BasicXpadIdE",
		&GetSixAxisSensorStates1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE",
		&GetSixAxisSensorStates2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid30InitializeConsoleSixAxisSensorEv",
		&InitializeConsoleSixAxisSensor);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid21IsSixAxisSensorAtRestERKNS0_19SixAxisSensorHandleE",
		&IsSixAxisSensorAtRest);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid28IsSixAxisSensorFusionEnabledERKNS0_19SixAxisSensorHandleE",
		&IsSixAxisSensorFusionEnabled);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid34ResetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleE",
		&ResetSixAxisSensorFusionParameters);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid32SetSixAxisSensorFusionParametersERKNS0_19SixAxisSensorHandleEff",
		&SetSixAxisSensorFusionParameters);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid18StartSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE",
		&StartSixAxisSensor1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE",
		&StartSixAxisSensor2);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid17StopSixAxisSensorERKNS0_26ConsoleSixAxisSensorHandleE",
		&StopSixAxisSensor1);
	SaltySDCore_ReplaceImport(
		"_ZN2nn3hid17StopSixAxisSensorERKNS0_19SixAxisSensorHandleE",
		&StopSixAxisSensor2);
	// clang-format on

	SaltySD_printf("SwiTAS_MotionPlugin: injection finished\n");
}
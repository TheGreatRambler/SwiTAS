#pragma once

#define BIT(n) (1U << (n))
#define IS_KEYBOARD_HELD(data, key) data[(key / 32)] & (1ULL << (key % 32))
// clang-format off
#define SET_KEYBOARD_HELD(data, key, state) \
	if(state) { \
		data[(key / 32)] |= (1ULL << (key % 32)); \
	} else { \
		data[(key / 32)] &= ~(1ULL << (key % 32)); \
	}
// clang-format on

#include <cstdint>
//#include <switch_min.h>
#include <type_traits>

namespace nn {
	namespace fs {
		struct FileHandle {
			void* handle;
		};

		enum OpenMode {
			OpenMode_Read   = BIT(0),
			OpenMode_Write  = BIT(1),
			OpenMode_Append = BIT(2),

			OpenMode_ReadWrite = OpenMode_Read | OpenMode_Write
		};

		struct WriteOption {
			int32_t flags;

			static WriteOption CreateOption(int flags) {
				WriteOption op;
				op.flags = flags;
				return op;
			}
		};

		enum WriteOptionFlag { WriteOptionFlag_Flush = BIT(0) };
	};

	namespace util {
		struct Float3 {
			float x;
			float y;
			float z;
		};

		template <typename T> inline constexpr T AlignUp(T value, uint64_t align) {
			const auto inv_mask = align - 1;
			return static_cast<T>((value + inv_mask) & ~inv_mask);
		};
	};

	namespace hid {
		typedef uint32_t BasicXpadId;
		typedef uint32_t JoyXpadId;
		typedef uint32_t SixAxisSensorHandle;
		typedef uint32_t ConsoleSixAxisSensorHandle;

		// Both positive and negative
		const float AccelerometerMax   = 7.0f;
		const float AngularVelocityMax = 5.0f;
		// Angle is unbounded, but 1 is equal to one full rotation
		const int32_t SixAxisSensorStateCountMax = 16;
		const int32_t TouchStateCountMax         = 16;
		const int32_t TouchScreenStateCountMax   = 16;
		const int32_t KeyboardStateCountMax      = 16;
		const int32_t MouseStateCountMax         = 16;

		enum class GyroscopeZeroDriftMode : int32_t {
			GyroscopeZeroDriftMode_Loose    = 0,
			GyroscopeZeroDriftMode_Standard = 1,
			GyroscopeZeroDriftMode_Tight    = 2,
		};

		enum class NpadStyleTag : int32_t {
			ProController = BIT(0),
			Handheld      = BIT(1),
			JoyconPair    = BIT(2),
			JoyconLeft    = BIT(3),
			JoyconRight   = BIT(4),
			SystemExt     = BIT(29),
			System        = BIT(30),
		};

		enum class KeyboardAttribute : int32_t {
			IsConnected = BIT(0),
		};

		// https://switchbrew.github.io/libnx/hid_8h_source.html
		// Each of these correspond to a bit
		enum class KeyboardKey : int32_t {
			None,
			Error,

			A = 4,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			N1,
			N2,
			N3,
			N4,
			N5,
			N6,
			N7,
			N8,
			N9,
			N0,
			Enter,
			Escape,
			Backspace,
			Tab,
			Space,
			Minus,
			Equal,
			LeftBrace,
			RightBrace,
			Backslash,
			Tilde,
			Semicolon,
			Apostrophe,
			Grave,
			Comma,
			Dot,
			Slash,
			CapsLockKey,

			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,

			SystemRequest,
			ScrollLockKey,
			Pause,
			Insert,
			Home,
			PageUp,
			Delete,
			End,
			PageDown,
			Right,
			Left,
			Down,
			Up,

			NumLockKey,
			KPSlash,
			KPAsterisk,
			KPMinus,
			KPPlus,
			KPEnter,
			KP1,
			KP2,
			KP3,
			KP4,
			KP5,
			KP6,
			KP7,
			KP8,
			KP9,
			KP0,
			KPDot,

			Key102,
			Compose,
			Power,
			KPEqual,

			F13,
			F14,
			F15,
			F16,
			F17,
			F18,
			F19,
			F20,
			F21,
			F22,
			F23,
			F24,

			Open,
			Help,
			Properties,
			Front,
			Stop,
			Repeat,
			Undo,
			Cut,
			Copy,
			Paste,
			Find,
			Mute,
			VolumeUp,
			VolumeDown,
			CapsLockActive,
			NumLockActive,
			ScrollLockActive,
			KPComma,

			KPLeftParenthesis,
			KPRightParenthesis,

			LeftControlKey = 0xE0,
			LeftShiftKey,
			LeftAltKey,
			LeftMetaKey,
			RightControlKey,
			RightShiftKey,
			RightAltKey,
			RightMetaKey,

			MediaPlayPause,
			MediaStopCD,
			MediaPrevious,
			MediaNext,
			MediaEject,
			MediaVolumeUp,
			MediaVolumeDown,
			MediaMute,
			MediaWebsite,
			MediaBack,
			MediaForward,
			MediaStop,
			MediaFind,
			MediaScrollUp,
			MediaScrollDown,
			MediaEdit,
			MediaSleep,
			MediaCoffee,
			MediaRefresh,
			MediaCalculator,

			NumKeyboardKeys,
		};

		enum class KeyboardModifier : int32_t {
			CapsLock   = BIT(8),
			Control    = BIT(0),
			Gui        = BIT(4),
			LeftAlt    = BIT(2),
			NumLock    = BIT(10),
			RightAlt   = BIT(3),
			ScrollLock = BIT(9),
			Shift      = BIT(1),
		};

		enum class MouseAttribute : int32_t {
			Transferable = BIT(0),
			IsConnected  = BIT(1),
		};

		enum class MouseButton : int32_t {
			Back    = BIT(4),
			Forward = BIT(3),
			Left    = BIT(0),
			Middle  = BIT(2),
			Right   = BIT(1),
		};

		enum class TouchAttribute : int32_t {
			Start = BIT(0),
			End   = BIT(1),
		};

		enum class SixAxisSensorAttribute : int32_t {
			IsConnected    = BIT(0),
			IsInterpolated = BIT(1),
		};

		// nn.hid.DirectionState
		struct DirectionState {
			nn::util::Float3 x;
			nn::util::Float3 y;
			nn::util::Float3 z;
		};

		enum class NpadStyle {
			// These both return one handle
			NpadStyleFullKey  = BIT(0),
			NpadStyleHandheld = BIT(1),
			// These do not
			NpadStyleJoyDual  = BIT(2),
			NpadStyleJoyLeft  = BIT(3),
			NpadStyleJoyRight = BIT(4),
		};

		enum class NpadIdType : int32_t {
			None     = -1,
			Player1  = 0,
			Player2  = 1,
			Player3  = 2,
			Player4  = 3,
			Player5  = 4,
			Player6  = 5,
			Player7  = 6,
			Player8  = 7,
			Unknown  = 16,
			Handheld = 32,
		};

		struct SixAxisSensorState {
			uint64_t deltaTimeNanoSeconds;
			int64_t samplingNumber;
			nn::util::Float3 acceleration;
			nn::util::Float3 angularVelocity;
			nn::util::Float3 angle;
			DirectionState direction;
			int32_t attributes;
		};

		struct TouchState {
			uint64_t deltaTimeNanoSeconds;
			int32_t attributes;
			int32_t touchIndex;
			int32_t x;
			int32_t y;
			int32_t diameterX;
			int32_t diameterY;
			int32_t rotationAngle;
		};

		struct TouchScreenState1Touch {
			int64_t samplingNumber;
			int32_t count;
			nn::hid::TouchState touches[1];
		};

		struct TouchScreenState5Touch {
			int64_t samplingNumber;
			int32_t count;
			nn::hid::TouchState touches[5];
		};

		struct TouchScreenState16Touch {
			int64_t samplingNumber;
			int32_t count;
			nn::hid::TouchState touches[16];
		};

		struct KeyboardState {
			int64_t samplingNumber;
			int32_t attributes;
			int32_t modifiers;
			uint32_t keys[8];
		};

		struct MouseState {
			int64_t samplingNumber;
			int64_t attributes;
			int32_t x;
			int32_t y;
			int32_t velocityX;
			int32_t velocityY;
			int32_t scrollVelocityX;
			int32_t scrollVelocityY;
			int32_t buttons;
		};
	};
};
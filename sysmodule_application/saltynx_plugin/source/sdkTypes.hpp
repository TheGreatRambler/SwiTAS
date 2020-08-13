#pragma once

#include <cstdint>
#include <switch_min.h>
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

		/*
				template <size_t N, typename T = void> struct BitFlagSet {
					using Storage                           = typename std::conditional<N <= BITSIZEOF(uint32_t), uint32_t, uint64_t>::type;
					static constexpr size_t StorageBitCount = BITSIZEOF(Storage);
					static constexpr size_t StorageCount    = util::AlignUp(N, StorageBitCount) / StorageBitCount;
					Storage _storage[StorageCount];
				};
		*/
	};

	namespace hid {
		typedef uint32_t BasicXpadId;
		typedef uint32_t JoyXpadId;
		typedef uint32_t SixAxisSensorHandle;
		typedef uint32_t ConsoleSixAxisSensorHandle;

		const float AccelerometerMax             = 7.0f;
		const float AngularVelocityMax           = 5.0f;
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
			A               = 0x04,
			Application     = 0x65,
			B               = 0x05,
			Backquote       = 0x35,
			Backslash       = 0x64,
			Backspace       = 0x2A,
			C               = 0x06,
			CapsLock        = 0x39,
			CloseBracket    = 0x30,
			Comma           = 0x36,
			D               = 0x07,
			D0              = 0x27,
			D1              = 0x1E,
			D2              = 0x1F,
			D3              = 0x20,
			D4              = 0x21,
			D5              = 0x22,
			D6              = 0x23,
			D7              = 0x24,
			D8              = 0x25,
			D9              = 0x26,
			De              = 0x4C,
			Do              = 0x51,
			E               = 0x08,
			End             = 0x4D,
			Esc             = 0x29,
			F               = 0x09,
			F1              = 0x3A,
			F10             = 0x43,
			F11             = 0x44,
			F12             = 0x45,
			F13             = 0x68,
			F14             = 0x69,
			F15             = 0x6A,
			F16             = 0x6B,
			F17             = 0x6C,
			F18             = 0x6D,
			F19             = 0x6E,
			F2              = 0x3B,
			F20             = 0x6F,
			F21             = 0x70,
			F22             = 0x71,
			F23             = 0x72,
			F24             = 0x73,
			F3              = 0x3C,
			F4              = 0x3D,
			F5              = 0x3E,
			F6              = 0x3F,
			F7              = 0x40,
			F8              = 0x41,
			F9              = 0x42,
			G               = 0x0A,
			H               = 0x0B,
			Home            = 0x4A,
			I               = 0x0C,
			Insert          = 0x49,
			J               = 0x0D,
			K               = 0x0E,
			L               = 0x0F,
			LeftAlt         = 0xE2,
			LeftArrow       = 0x50,
			LeftControl     = 0xE0,
			LeftGui         = 0xE3,
			LeftShift       = 0xE1,
			M               = 0x10,
			Minus           = 0x2D,
			Muhenkan        = 0x8B,
			N               = 0x11,
			NumLock         = 0x53,
			NumPad0         = 0x62,
			NumPad1         = 0x59,
			NumPad2         = 0x5A,
			NumPad3         = 0x5B,
			NumPad4         = 0x5C,
			NumPad5         = 0x5D,
			NumPad6         = 0x5E,
			NumPad7         = 0x5F,
			NumPad8         = 0x60,
			NumPad9         = 0x61,
			NumPadAdd       = 0x57,
			NumPadComma     = 0x85,
			NumPadCommaPc98 = 0x8C,
			NumPadDivide    = 0x54,
			NumPadDot       = 0x63,
			NumPadEnter     = 0x58,
			NumPadEquals    = 0x67,
			NumPadMultiply  = 0x55,
			NumPadSubtract  = 0x56,
			O               = 0x12,
			OpenBracket     = 0x2F,
			P               = 0x13,
			PageDown        = 0x4E,
			PageUp          = 0x4B,
			Pause           = 0x48,
			Period          = 0x37,
			Pipe            = 0x31,
			Plus            = 0x2E,
			Power           = 0x66,
			PrintScreen     = 0x46,
			Q               = 0x14,
			Quote           = 0x34,
			R               = 0x15,
			Return          = 0x28,
			RightAlt        = 0xE6,
			RightArrow      = 0x4F,
			RightControl    = 0xE4,
			RightGui        = 0xE7,
			RightShift      = 0xE5,
			Ro              = 0x87,
			S               = 0x16,
			ScrollLock      = 0x47,
			Semicolon       = 0x33,
			Slash           = 0x38,
			Space           = 0x2C,
			T               = 0x17,
			Tab             = 0x2B,
			Tilde           = 0x32,
			U               = 0x18,
			UpArrow         = 0x52,
			V               = 0x19,
			W               = 0x1A,
			X               = 0x1B,
			Y               = 0x1C,
			Z               = 0x1D,
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
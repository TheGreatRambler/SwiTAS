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
			int flags;

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
		enum class GyroscopeZeroDriftMode : uint32_t {
			GyroscopeZeroDriftMode_Loose,
			GyroscopeZeroDriftMode_Standard,
			GyroscopeZeroDriftMode_Tight,
		};

		enum class NpadStyleTag : uint32_t {
			ProController = BIT(0),
			Handheld      = BIT(1),
			JoyconPair    = BIT(2),
			JoyconLeft    = BIT(3),
			JoyconRight   = BIT(4),
			SystemExt     = BIT(29),
			System        = BIT(30),
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

		enum class NpadIdType : uint32_t {
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
			uint64 deltaTimeNanoSeconds;
			int64_t samplingNumber;
			nn::util::Float3 acceleration;
			nn::util::Float3 angularVelocity;
			nn::util::Float3 angle;
			nn::hid::DirectionState direction;
			int32_t attributes;
		};

		struct TouchState {
			uint32_t attributes;
			uint64_t deltaTimeNanoSeconds;
			int32_t diameterX;
			int32_t diameterY;
			int32_t fingerId;
			int32_t rotationAngle;
			int32_t x;
			int32_t y;
		};

		struct TouchScreenState {
			int32_t count;
			int64_t samplingNumber;
			nn::hid::TouchState touches[nn::hid::TouchStateCountMax];
		};

		typedef uint32_t BasicXpadId;
		typedef uint32_t JoyXpadId;
		typedef uint32_t SixAxisSensorHandle;
		typedef uint32_t ConsoleSixAxisSensorHandle;

		const float AccelerometerMax         = 7.0f;
		const float AngularVelocityMax       = 5.0f;
		const int SixAxisSensorStateCountMax = 16;
		const int TouchStateCountMax         = 16;
		const int TouchScreenStateCountMax   = 16;
	};
};
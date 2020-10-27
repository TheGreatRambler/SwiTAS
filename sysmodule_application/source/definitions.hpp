#pragma once

#define PLUGIN_INTERFACE_VERSION 1

#define BIT(n) (1U << (n))

#include <cstdint>
#include <cstring>

namespace PluginDefinitions {

	enum class LogLevel : uint8_t {
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Critical,
	};

	enum class EnableInputType : uint16_t {
		EnableController1        = 1,
		EnableController2        = 2,
		EnableController3        = 3,
		EnableController4        = 4,
		EnableController5        = 5,
		EnableController6        = 6,
		EnableController7        = 7,
		EnableController8        = 8,
		EnableControllerHandheld = 9,
		EnableTouchscreen        = 10,
		EnableMouse              = 11,
		EnableKeyboard           = 12,
		All                      = 13,
	};

	enum class YuzuJoystickType : uint8_t {
		LeftX  = 0,
		LeftY  = 1,
		RightX = 2,
		RightY = 3,
	};

	// Lifted from settings.h
	enum class ButtonValues : uint8_t {
		A,
		B,
		X,
		Y,
		LStick,
		RStick,
		L,
		R,
		ZL,
		ZR,
		Plus,
		Minus,

		DLeft,
		DUp,
		DRight,
		DDown,

		LStick_Left,
		LStick_Up,
		LStick_Right,
		LStick_Down,

		RStick_Left,
		RStick_Up,
		RStick_Right,
		RStick_Down,

		SL,
		SR,

		Home,
		Screenshot,

		NumButtons,
	};

	enum class ControllerType : uint8_t {
		None,
		ProController,
		Handheld,
		JoyDual,
		JoyLeft,
		JoyRight,
		Pokeball,
	};

	enum class ControllerNumber : uint8_t {
		Controller1,
		Controller2,
		Controller3,
		Controller4,
		Controller5,
		Controller6,
		Controller7,
		Controller8,
		Handheld,
		Unknown,
	};

	enum class KeyboardValues : uint8_t {
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

	enum class KeyboardModifiers : uint8_t {
		LeftControl,
		LeftShift,
		LeftAlt,
		LeftMeta,
		RightControl,
		RightShift,
		RightAlt,
		RightMeta,
		CapsLock,
		ScrollLock,
		NumLock,

		NumKeyboardMods,
	};

	enum class MouseButton : uint8_t {
		Left,
		Right,
		Middle,
		Forward,
		Back,
	};

	enum class MouseTypes : uint8_t {
		X,
		Y,
		DeltaX,
		DeltaY,
		WheelX,
		WheelY,
	};

	enum class TouchTypes : uint8_t {
		X,
		Y,
		DiameterX,
		DiameterY,
		RotationAngle,
	};

	enum class SixAxisMotionTypes : uint8_t {
		AccelerationX,
		AccelerationY,
		AccelerationZ,
		AngularVelocityX,
		AngularVelocityY,
		AngularVelocityZ,
		AngleX,
		AngleY,
		AngleZ,
		DirectionXX,
		DirectionXY,
		DirectionXZ,
		DirectionYX,
		DirectionYY,
		DirectionYZ,
		DirectionZX,
		DirectionZY,
		DirectionZZ,
	};

	typedef void(meta_setup_plugin)();
	typedef void(meta_handle_main_loop)();
	typedef uint64_t(meta_getplugininterfaceversion)();
	typedef void(meta_handle_close)();

	typedef void(meta_free)(void*);
	typedef void(emu_frameadvance)(void* ctx);
	typedef void(emu_pause)(void* ctx);
	typedef void(emu_unpause)(void* ctx);
	typedef int32_t(emu_framecount)(void* ctx);
	typedef float(emu_fps)(void* ctx);
	typedef uint8_t(emu_emulating)(void* ctx);
	/*
	typedef char*(emu_getgamedir)(void* ctx);
	typedef void(emu_loadrom)(void* ctx, const char* filename);
	typedef uint8_t*(emu_getscreenframebuffer)(void* ctx, uint64_t* size);
	typedef uint8_t*(emu_getscreenjpeg)(void* ctx, uint64_t* size);
	*/
	typedef char*(emu_romname)(void* ctx);
	typedef uint64_t(emu_getprogramid)(void* ctx);
	typedef uint64_t(emu_getprocessid)(void* ctx);
	typedef uint64_t(emu_getheapstart)(void* ctx);
	typedef uint64_t(emu_getheapsize)(void* ctx);
	typedef uint64_t(emu_getmainstart)(void* ctx);
	typedef uint64_t(emu_getmainsize)(void* ctx);
	typedef uint64_t(emu_getstackstart)(void* ctx);
	typedef uint64_t(emu_getstacksize)(void* ctx);
	typedef void(emu_log)(void* ctx, const char* logmessage, LogLevel level);
	typedef uint8_t(memory_readbyterange)(void* ctx, uint64_t address, uint8_t* bytes, uint64_t length);
	typedef uint8_t(memory_writebyterange)(void* ctx, uint64_t address, uint8_t* bytes, uint64_t length);
	typedef uint64_t(debugger_getclockticks)(void* ctx);
	typedef uint64_t(debugger_getcputicks)(void* ctx);
	typedef uint64_t(joypad_read)(void* ctx, ControllerNumber player);
	typedef void(joypad_set)(void* ctx, ControllerNumber player, uint64_t input);
	typedef int16_t(joypad_readjoystick)(void* ctx, ControllerNumber player, YuzuJoystickType type);
	typedef void(joypad_setjoystick)(void* ctx, ControllerNumber player, YuzuJoystickType type, int16_t val);
	typedef float(joypad_readsixaxis)(void* ctx, ControllerNumber player, SixAxisMotionTypes type);
	typedef void(joypad_setsixaxis)(void* ctx, ControllerNumber player, SixAxisMotionTypes type, float val);
	typedef void(joypad_enablejoypad)(void* ctx, ControllerNumber player, uint8_t enable);
	typedef void(joypad_removealljoypads)(void* ctx);
	typedef void(joypad_setjoypadtype)(void* ctx, ControllerNumber player, ControllerType type);
	typedef uint8_t(joypad_isjoypadconnected)(void* ctx, ControllerNumber player);
	typedef void(input_requeststateupdate)(void* ctx);
	typedef void(input_enablekeyboard)(void* ctx, uint8_t enable);
	typedef void(input_enablemouse)(void* ctx, uint8_t enable);
	typedef void(input_enabletouchscreen)(void* ctx, uint8_t enable);
	typedef uint8_t(input_iskeypressed)(void* ctx, KeyboardValues key);
	typedef void(input_setkeypressed)(void* ctx, KeyboardValues key, uint8_t ispressed);
	typedef uint8_t(input_iskeymodifierpressed)(void* ctx, KeyboardModifiers modifier);
	typedef void(input_setkeymodifierpressed)(void* ctx, KeyboardModifiers modifier, uint8_t ispressed);
	typedef uint8_t(input_ismousepressed)(void* ctx, MouseButton button);
	typedef void(input_setmousepressed)(void* ctx, MouseButton button, uint8_t ispressed);
	typedef uint8_t(input_getnumtouches)(void* ctx);
	typedef void(input_setnumtouches)(void* ctx, uint8_t num);
	typedef uint32_t(joypad_readtouch)(void* ctx, uint8_t idx, TouchTypes type);
	typedef void(joypad_settouch)(void* ctx, uint8_t idx, TouchTypes type, uint32_t val);
	typedef void(joypad_movemouse)(void* ctx, MouseTypes type, int32_t val);
	typedef int32_t(joypad_readmouse)(void* ctx, MouseTypes type);
	typedef void(input_enableoutsideinput)(void* ctx, EnableInputType typetoenable, uint8_t enable);
	typedef uint32_t(gui_getwidth)(void* ctx);
	typedef uint32_t(gui_getheight)(void* ctx);
	typedef void(gui_clearscreen)(void* ctx);
	typedef void(gui_render)(void* ctx);
	typedef void(gui_drawpixel)(void* ctx, uint32_t x, uint32_t y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
	typedef bool(gui_savescreenshotas)(void* ctx, const char* path);
	typedef void(gui_drawimage)(void* ctx, int32_t dx, int32_t dy, const char* path, int32_t sx, int32_t sy, int32_t sw, int32_t sh);
	typedef void(gui_popup)(void* ctx, const char* title, const char* message, const char* type);
	typedef uint8_t*(gui_savescreenshotmemory)(void* ctx, uint64_t* size);
} // namespace PluginDefinitions
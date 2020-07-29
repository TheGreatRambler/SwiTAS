#pragma once

#define PLUGIN_INTERFACE_VERSION 0

#include <cstdint>
#include <cstring>

enum YuzuJoystickType : uint8_t {
	LeftX  = 0,
	LeftY  = 1,
	RightX = 2,
	RightY = 3,
};

// NOTE: Every time a char string is returned, it must be freed by the DLL

typedef void(meta_setup_plugin)(void*);
typedef void(meta_handle_main_loop)();
typedef void(meta_add_function)(void*);
typedef uint64_t(meta_getplugininterfaceversion)();

// Emu library

// emu.poweron() ignored
// emu.softreset() ignored
typedef void(emu_speedmode)(void* ctx, const char* mode);
typedef void(emu_frameadvance)(void* ctx);
typedef void(emu_pause)(void* ctx);
typedef void(emu_unpause)(void* ctx);
// emu.exec_count(int count, function func) ignored
// emu.exec_time(int time, function func) ignored
// emu.setrenderplanes(bool sprites, bool background) ignored
typedef void(emu_message)(void* ctx, char* mode);
typedef int(emu_framecount)(void* ctx);
// int emu.lagcount() ignored
// bool emu.lagged()
// emu.setlagflag(bool value) ignored
typedef uint8_t(emu_emulating)(void* ctx);
typedef uint8_t(emu_paused)(void* ctx);
// bool emu.readonly() ignored
// emu.setreadonly(bool state) ignored
typedef char*(emu_getdir)(void* ctx);
typedef void(emu_loadrom)(void* ctx, char* filename);
// emu.registerbefore(function func) handled outside of Yuzu
// emu.registerafter(function func) handled outside of Yuzu
// emu.registerexit(function func) handled outside of Yuzu
// bool emu.addgamegenie(string str) ignored
// bool emu.delgamegenie(string str) ignored
typedef void(emu_print)(void* ctx, uint8_t mode);
typedef uint8_t*(emu_getscreenframebuffer)(void* ctx, uint64_t* size);
typedef uint8_t*(emu_getscreenjpeg)(void* ctx, uint64_t* size);

// Check if game is opened
typedef uint8_t(emu_isromopened)(void* ctx);
// Get the name of the currently running game
typedef char*(emu_romname)(void* ctx);
// Get program ID
typedef uint64_t(emu_getprogramid)(void* ctx);
// Get process ID
typedef uint64_t(emu_getprocessid)(void* ctx);
// Get heap start
typedef uint64_t(emu_getheapstart)(void* ctx);
// Get main start
typedef uint64_t(emu_getmainstart)(void* ctx);
// Log on Yuzu
typedef void(emu_log)(void* ctx, const char* logmessage);

// ROM Library (handled differently since the games are bigger)

typedef uint8_t(rom_readbyteunsigned)(void* ctx, uint64_t address);
typedef int8_t(rom_readbytesigned)(void* ctx, uint64_t address);
typedef void(rom_writebyte)(void* ctx, uint64_t address, uint8_t byte);

// Memory Library

typedef uint8_t(memory_readbyteunsigned)(void* ctx, uint64_t address);
typedef uint8_t*(memory_readbyterange)(void* ctx, uint64_t address, uint64_t length);
typedef int8_t(memory_readbytesigned)(void* ctx, uint64_t address);
// memory.readword(int addressLow, [int addressHigh]) ignored
// memory.readwordunsigned(int addressLow, [int addressHigh]) ignored
// memory.readwordsigned(int addressLow, [int addressHigh]) ignored
typedef void(memory_writebyte)(void* ctx, uint64_t address, uint8_t byte);
// int memory.getregister(cpuregistername) ignored, believe the name is different
// memory.setregister(string cpuregistername, int value) ignored, same as above
// memory.register(int address, [int size,] function func) ignored
// memory.registerwrite(int address, [int size,] function func) ignored
// memory.registerexec(int address, [int size,] function func) ignored, talk to Shadow
// memory.registerrun(int address, [int size,] function func) ignored, same as above
// memory.registerexecute(int address, [int size,] function func) ignored, same as above

// Debugger Library

typedef void(debugger_hitbreakpoint)(void* ctx);
typedef uint64_t(debugger_getcyclescount)(void* ctx);
typedef uint64_t(debugger_getinstructionscount)(void* ctx);
typedef void(debugger_resetcyclescount)(void* ctx);
typedef void(debugger_resetinstructionscount)(void* ctx);

// Joypad Library (Modified, based on libnx standards)

typedef uint64_t(joypad_read)(void* ctx, uint8_t player);
typedef uint64_t(joypad_immediate)(void* ctx, uint8_t player);
// table joypad.getdown(int player) ignored
// table joypad.readdown(int player) ignored
// table joypad.getup(int player) ignored
// table joypad.readup(int player) ignored
typedef void(joypad_set)(void* ctx, uint8_t player, uint64_t input);

// Joystick, accel and gyro based on enums
typedef int16_t(joypad_readjoystick)(void* ctx, uint8_t player, YuzuJoystickType type);
// Joystick, accel and gyro based on enums
typedef void(joypad_setjoystick)(void* ctx, uint8_t player, YuzuJoystickType type, int16_t val);
// Disable input entering from outside the script, this allows the script to set input without interruption
typedef void(joypad_enableoutsideinput)(void* ctx, uint8_t enable);
// Set number of joysticks in use, will reset all controllers and replace them
typedef void(joypad_setnumjoypads)(void* ctx, uint8_t numofplayers);
// Add controllers
typedef void(joypad_addjoypad)(void* ctx);
// Get number of controllers
typedef uint8_t(joypad_getnumjoypads)(void* ctx);

// Input Library

// table input.get() ignored
// table input.read() ignored
// string input.popup ignored

// Keyboard machanism based on enums
typedef uint8_t(input_ispressed)(void* ctx, uint8_t key);

// Savestate Library implemented in dll

// Movie Library implemented in dll

// GUI Library (Most functions handled DLL side, even text handling)

typedef void(gui_drawpixel)(void* ctx, int x, int y, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue);
// gui.getpixel(int x, int y) ignored
// gui.box(int x1, int y1, int x2, int y2 [, fillcolor [, outlinecolor]]))
// gui.drawbox(int x1, int y1, int x2, int y2 [, fillcolor [, outlinecolor]]))
// gui.rect(int x1, int y1, int x2, int y2 [, fillcolor [, outlinecolor]]))
// gui.drawrect(int x1, int y1, int x2, int y2 [, fillcolor [, outlinecolor]]))
// gui.text(int x, int y, string str [, textcolor [, backcolor]])
// gui.drawtext(int x, int y, string str [, textcolor [, backcolor]])
// gui.parsecolor(color) ignored
typedef char*(gui_savescreenshot)(void* ctx);
typedef void(gui_savescreenshotas)(void* ctx, char* path);
typedef void(gui_drawimage)(void* ctx, int dx, int dy, char* path, int sx, int sy, int sw, int sh, float alphamul);
// gui.opacity(int alpha) ignored
// gui.transparency(int trans) ignored
// function gui.register(function func) ignored
typedef void(gui_popup)(void* ctx, char* message, char* type, char* icon);

// Saves screenshot into byte array as raw framebuffer
typedef uint8_t*(gui_savescreenshotmemory)(void* ctx, uint64_t* size);

// Sound Library ignored

// TAS Editor Library implemented in DLL

// Bitwise Operations implemented in DLL
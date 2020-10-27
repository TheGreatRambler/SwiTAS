#pragma once

#include "dllFunctionDefinitions.hpp"

DLL_EXPORT void* yuzuInstance;

YUZU_FUNC(meta_free)
YUZU_FUNC(emu_frameadvance)
YUZU_FUNC(emu_pause)
YUZU_FUNC(emu_unpause)
YUZU_FUNC(emu_framecount)
YUZU_FUNC(emu_fps)
YUZU_FUNC(emu_emulating)
YUZU_FUNC(emu_romname)
YUZU_FUNC(emu_getprogramid)
YUZU_FUNC(emu_getprocessid)
YUZU_FUNC(emu_getheapstart)
YUZU_FUNC(emu_getheapsize)
YUZU_FUNC(emu_getmainstart)
YUZU_FUNC(emu_getmainsize)
YUZU_FUNC(emu_getstackstart)
YUZU_FUNC(emu_getstacksize)
YUZU_FUNC(emu_log)
YUZU_FUNC(memory_readbyterange)
YUZU_FUNC(memory_writebyterange)
YUZU_FUNC(debugger_getclockticks)
YUZU_FUNC(debugger_getcputicks)
YUZU_FUNC(joypad_read)
YUZU_FUNC(joypad_set)
YUZU_FUNC(joypad_readjoystick)
YUZU_FUNC(joypad_setjoystick)
YUZU_FUNC(joypad_readsixaxis)
YUZU_FUNC(joypad_setsixaxis)
YUZU_FUNC(joypad_enablejoypad)
YUZU_FUNC(joypad_removealljoypads)
YUZU_FUNC(joypad_setjoypadtype)
YUZU_FUNC(joypad_isjoypadconnected)
YUZU_FUNC(input_requeststateupdate)
YUZU_FUNC(input_enablekeyboard)
YUZU_FUNC(input_enablemouse)
YUZU_FUNC(input_enabletouchscreen)
YUZU_FUNC(input_iskeypressed)
YUZU_FUNC(input_setkeypressed)
YUZU_FUNC(input_iskeymodifierpressed)
YUZU_FUNC(input_setkeymodifierpressed)
YUZU_FUNC(input_ismousepressed)
YUZU_FUNC(input_setmousepressed)
YUZU_FUNC(input_getnumtouches)
YUZU_FUNC(input_setnumtouches)
YUZU_FUNC(joypad_readtouch)
YUZU_FUNC(joypad_settouch)
YUZU_FUNC(joypad_movemouse)
YUZU_FUNC(joypad_readmouse)
YUZU_FUNC(input_enableoutsideinput)
YUZU_FUNC(gui_getwidth)
YUZU_FUNC(gui_getheight)
YUZU_FUNC(gui_clearscreen)
YUZU_FUNC(gui_render)
YUZU_FUNC(gui_drawpixel)
YUZU_FUNC(gui_savescreenshotas)
YUZU_FUNC(gui_drawimage)
YUZU_FUNC(gui_popup)
YUZU_FUNC(gui_savescreenshotmemory)
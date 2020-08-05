#pragma once

// clang-format off
#ifdef __SWITCH__
	#define DLL_EXPORT
#else
	#ifdef _WIN32
		#define DLL_EXPORT extern "C" __declspec(dllexport)
	#else
		#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
	#endif
#endif
// clang-format on

// clang-format off
#define YUZU_FUNC(func) \
	func* function_##func = nullptr; \
	void set_##func(func* function) { \
		function_##func = function; \
	}
// clang-format on
// The calling has to be done manually

// clang-format off
#define SET_YUZU_FUNC(class, func) void yuzupluginset_##func(func* function) { \
		class->set_##func(function); \
	}
// clang-format on

// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
// Bools become uint8_t due to portability issues
// Everu function is preceded by a pointer to the Yuzu context

#include "definitions.hpp"
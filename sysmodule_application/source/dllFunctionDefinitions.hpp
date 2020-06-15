#pragma once

// clang-format off
#define YUZU_FUNC(func, returntype, arguments) \
	func* function_##func = nullptr; \
	void set_##func(func* function) { \
		function_##func = function; \
	}
// clang-format on
// The calling has to be done manually

// clang-format off
#define SET_YUZU_FUNC(class, func) void set_##func(func* function) { \
		class.set_##func(function); \
	}
// clang-format on

// http://www.fceux.com/web/help/fceux.html?LuaFunctionsList.html
// Bools become uint8_t due to portability issues
// Everu function is preceded by a pointer to the Yuzu context

#include "definitions.hpp"
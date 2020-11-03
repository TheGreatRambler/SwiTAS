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
#ifdef __SWITCH__
	#define YUZU_FUNC(func)
#else
	#ifdef _WIN32
		#define YUZU_FUNC(func) \
			extern "C" __declspec(dllexport) PluginDefinitions::func* yuzu_##func;
	#else
		#define YUZU_FUNC(func) \
			extern "C" __attribute__((visibility("default"))) PluginDefinitions::func* yuzu_##func;
	#endif
#endif
// clang-format on

#define YUZU_FUNC_DEFINE(func) PluginDefinitions::func* yuzu_##func = nullptr;

#include "definitions.hpp"
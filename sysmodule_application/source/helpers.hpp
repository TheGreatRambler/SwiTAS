#pragma once

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace HELPERS {
	void getMemUsage(uint64_t* memAvaliable, uint64_t* memUsed);
}
#include "helpers.hpp"

void HELPERS::getMemUsage(uint64_t* memAvaliable, uint64_t* memUsed) {
#ifdef __SWITCH__
	svcGetInfo(memAvaliable, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
	svcGetInfo(memUsed, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
#endif
}
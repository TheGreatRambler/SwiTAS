#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <switch.h>

#define SERVER_IMP
#include "../../sharedNetworkCode/networkInterface.hpp"

class MainLoop {
private:
	Result rc;
	u64 applicationProcessId  = 0;
	u64 applicationProgramId  = 0;
	uint8_t applicationOpened = false;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	static char* getAppName(u64 application_id);

public:
	MainLoop();

	void mainLoopHandler();
};
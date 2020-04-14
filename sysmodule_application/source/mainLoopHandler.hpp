#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <plog/Log.h>
#include <switch.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "controller.hpp"

class MainLoop {
private:
	Result rc;
	u64 applicationProcessId  = 0;
	u64 applicationProgramId  = 0;
	uint8_t applicationOpened = false;
	uint8_t internetConnected = false;

	Event vsyncEvent;

	std::unique_ptr<ControllerHandler> controller;

	std::shared_ptr<CommunicateWithNetwork> networkInstance;

	static char* getAppName(u64 application_id);

	void handleNetworkUpdates();

public:
	MainLoop();

	void mainLoopHandler();

	~MainLoop();
};
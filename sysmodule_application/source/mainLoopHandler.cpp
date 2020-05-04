#include "mainLoopHandler.hpp"

MainLoop::MainLoop() {
	LOGD << "Start networking";
	// Start networking with set queues
	networkInstance = std::make_shared<CommunicateWithNetwork>(
		[](CommunicateWithNetwork* self) {
			SEND_QUEUE_DATA(RecieveFlag)
			SEND_QUEUE_DATA(RecieveGameInfo)
			SEND_QUEUE_DATA(RecieveGameFramebuffer)
			SEND_QUEUE_DATA(RecieveApplicationConnected)
			SEND_QUEUE_DATA(RecieveLogging)
			SEND_QUEUE_DATA(RecieveMemoryRegion)
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(SendFlag)
			RECIEVE_QUEUE_DATA(SendRunFrame)
			RECIEVE_QUEUE_DATA(SendLogging)
			RECIEVE_QUEUE_DATA(SendTrackMemoryRegion)
		});

	LOGD << "Open display";
	ViDisplay disp;
	rc = viOpenDefaultDisplay(&disp);
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Get vsync event";
	rc = viGetDisplayVsyncEvent(&disp, &vsyncEvent);
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Attach work buffers";
	// Attach Work Buffer
	rc = hiddbgAttachHdlsWorkBuffer();
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Create controller";
	controller = std::make_unique<ControllerHandler>(networkInstance);
}

void MainLoop::mainLoopHandler() {
	rc = pmdmntGetApplicationProcessId(&applicationProcessId);

	// Lifted from switchPresense-Rewritten
	if(R_SUCCEEDED(rc)) {
		// Application connected
		// Get application info
		rc = pminfoGetProgramId(&applicationProgramId, applicationProcessId);
		if(R_SUCCEEDED(rc)) {
			if(!applicationOpened) {
				gameName = std::string(getAppName(applicationProgramId));
				LOGD << "Application " + gameName + " opened";
				ADD_TO_QUEUE(RecieveApplicationConnected, networkInstance, {
					data.applicationName      = gameName;
					data.applicationProgramId = applicationProgramId;
					data.applicationProcessId = applicationProcessId;
				})

				applicationOpened = true;

				// Start the whole main loop
				// Set the application for the controller
				LOGD << "Start controllers";
				waitForVsync();
				pauseApp();
			}
		}
	} else {
		// I believe this means that there is no application running
		// If there was just an application open, let the PC know
		if(applicationOpened) {
			LOGD << "Application closed";
			// clang-format off
			ADD_TO_QUEUE(RecieveFlag, networkInstance, {
				data.actFlag = RecieveInfo::APPLICATION_DISCONNECTED;
			})
			// clang-format on
			applicationOpened = false;
		}
	}

	if(networkInstance->isConnected()) {
		if(!internetConnected) {
			LOGD << "Internet connected";
			internetConnected = true;
		}
	} else {
		if(internetConnected) {
			LOGD << "Internet disconnected";
			internetConnected = false;

			// Force unpause to not get user stuck if network cuts out
			reset();
		}
	}

	// handle network updates always, they are stored in the queue regardless of the internet
	handleNetworkUpdates();

	// A reasonable time to sleep the thread
	// 1 millisecond
	svcSleepThread(1000000);
}

void MainLoop::handleNetworkUpdates() {
	CHECK_QUEUE(networkInstance, SendRunFrame, {
		LOGD << "Running frame";
		controller->runFrameWithPause(data.controllerData);
	})

	CHECK_QUEUE(networkInstance, SendTrackMemoryRegion, {
		LOGD << "Track memory region";
		memoryRegions.push_back(std::pair<uint64_t, uint64_t>(data.startByte, data.size));
	})

	CHECK_QUEUE(networkInstance, SendFlag, {
		if(data.actFlag == SendInfo::PAUSE_DEBUG) {
			// Precaution to prevent the app getting stuck without the
			// User able to unpause it
			if(applicationOpened && internetConnected) {
				LOGD << "Pause app";
				pauseApp();
			}
		} else if(data.actFlag == SendInfo::UNPAUSE_DEBUG) {
			if(applicationOpened) {
				LOGD << "Unpause app";
				unpauseApp();
			}
		} else if(data.actFlag == SendInfo::GET_FRAMEBUFFER) {
			if(applicationOpened) {
				LOGD << "Get framebuffer";
				getFramebuffer();
			}
		}
	})
}

void MainLoop::sendGameInfo() {
	if(applicationOpened) {

		std::vector<GameMemoryInfo> memoryInfo;

		// Will get more info via
		// https://github.com/switchbrew/switch-examples/blob/master/account/source/main.c

		uint64_t addr = 0;
		controller->pauseApp();
		Handle applicationHandle = controller->getApplicationDebugHandle();
		while(true) {
			MemoryInfo info = { 0 };
			uint32_t pageinfo;
			rc = svcQueryDebugProcessMemory(&info, &pageinfo, applicationHandle, addr);
			memoryInfo.push_back(getGameMemoryInfo(info));
			addr += info.size;

			if(R_FAILED(rc)) {
				break;
			}
		}
		controller->unpauseApp();

		ADD_TO_QUEUE(RecieveGameInfo, networkInstance, {
			data.applicationName      = gameName;
			data.applicationProgramId = applicationProgramId;
			data.applicationProcessId = applicationProcessId;
			data.memoryInfo           = memoryInfo;
		})
	}
}

char* MainLoop::getAppName(u64 application_id) {
	static NsApplicationControlData appControlData = { 0 };
	size_t appControlDataSize                      = 0;
	NacpLanguageEntry* languageEntry               = nullptr;

	if(R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, application_id, &appControlData, sizeof(NsApplicationControlData), &appControlDataSize))) {
		if(R_SUCCEEDED(nacpGetLanguageEntry(&appControlData.nacp, &languageEntry))) {
			if(languageEntry != nullptr)
				return languageEntry->name;
		}
	}
	return (char*)"Game Not Defined";
}

GameMemoryInfo MainLoop::getGameMemoryInfo(MemoryInfo memInfo) {
	GameMemoryInfo info;
	info.addr            = memInfo.addr;
	info.size            = memInfo.size;
	info.type            = memInfo.type;
	info.attr            = memInfo.attr;
	info.perm            = memInfo.perm;
	info.device_refcount = memInfo.device_refcount;
	info.ipc_refcount    = memInfo.ipc_refcount;
	info.padding         = memInfo.padding;
	return info;
}

void MainLoop::pauseApp() {
	if(!isPaused) {
		// Debug application again
		LOGD << "Pausing";
		rc       = svcDebugActiveProcess(&applicationDebug, applicationPID);
		isPaused = true;

		screenshotHandler.writeFramebuffer(networkInstance);

		for(auto const& memoryRegion : memoryRegions) {
			std::vector<uint8_t> buf(memoryRegion.second);
			svcReadDebugProcessMemory(buf->data(), applicationHandle, memoryRegion.first, memoryRegion.second);

			ADD_TO_QUEUE(RecieveMemoryRegion, networkInstance, {
				data.startByte = memoryRegion.first;
				data.size      = memoryRegion.second;
				data.memory    = buf;
			})
		}
	}
}

MainLoop::~MainLoop() {
	LOGD << "Exiting app";
	rc = hiddbgReleaseHdlsWorkBuffer();

	// Make absolutely sure the app is unpaused on close
	controller->reset();

	hiddbgExit();
}
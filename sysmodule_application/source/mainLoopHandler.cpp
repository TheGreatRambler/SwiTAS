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
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(SendFlag)
			RECIEVE_QUEUE_DATA(SendRunFrame)
			RECIEVE_QUEUE_DATA(SendLogging)
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
	controller = std::make_unique<ControllerHandler>(&vsyncEvent, networkInstance);
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
				LOGD << "Start controller";
				controller->setApplicationProcessId(applicationProcessId);
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
			controller->reset();
		}
	}

	// handle network updates always, they are stored in the queue regardless of the internet
	handleNetworkUpdates();

	// A reasonable time to sleep the thread
	// 1 millisecond
	svcSleepThread(1000000);
}

void MainLoop::handleNetworkUpdates() {
	CHECK_QUEUE(networkInstance, SendRunFrame,
		{
			// blah
		})

	CHECK_QUEUE(networkInstance, SendFlag, {
		if(data.actFlag == SendInfo::PAUSE_DEBUG) {
			// Precaution to prevent the app getting stuck without the
			// User able to unpause it
			if(applicationOpened && internetConnected) {
				LOGD << "Pause app";
				controller->pauseApp();
			}
		} else if(data.actFlag == SendInfo::UNPAUSE_DEBUG) {
			if(applicationOpened) {
				LOGD << "Unpause app";
				controller->unpauseApp();
			}
		} else if(data.actFlag == SendInfo::GET_FRAMEBUFFER) {
			if(applicationOpened) {
				LOGD << "Get framebuffer";
				controller->getFramebuffer();
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

MainLoop::~MainLoop() {
	LOGD << "Exiting app";
	rc = hiddbgReleaseHdlsWorkBuffer();

	// Make absolutely sure the app is unpaused on close
	controller->reset();

	hiddbgExit();
}
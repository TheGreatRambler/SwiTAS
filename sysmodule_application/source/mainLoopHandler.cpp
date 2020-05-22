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
			SEND_QUEUE_DATA(RecieveAutoRunControllerData)
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(SendFlag)
			RECIEVE_QUEUE_DATA(SendFrameData)
			RECIEVE_QUEUE_DATA(SendLogging)
			RECIEVE_QUEUE_DATA(SendTrackMemoryRegion)
			RECIEVE_QUEUE_DATA(SendSetNumControllers)
			RECIEVE_QUEUE_DATA(SendAutoRun)
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
				// LOGD << "Start controllers";
				// pauseApp();
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

	// Check auto run
	if(autoRunOn && networkInstance->isConnected()) {
		u64 currentTime = armTicksToNs(armGetSystemTick());
		if(lastAutorunTime == 0 || (currentTime - lastAutorunTime) > nanosecondsBetweenAutorun) {
			// TODO handle for any controller
			matchFirstControllerToTASController(0);

			// clang-format off
			ADD_TO_QUEUE(RecieveAutoRunControllerData, networkInstance, {
				data.controllerData = controllers[0]->getControllerData();
			})
			// clang-format on

			runSingleFrame();

			lastAutorunTime = currentTime;
		}
		// Get nanosecond time
		// Check if
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void MainLoop::handleNetworkUpdates() {
	CHECK_QUEUE(networkInstance, SendFrameData, {
		controllers[data.playerIndex]->setFrame(data.controllerData);
		if(data.incrementFrame) {
			LOGD << "Increment frame";
			runSingleFrame();
		}
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
				screenshotHandler.writeFramebuffer(networkInstance);
			}
		} else if(data.actFlag == SendInfo::RUN_BLANK_FRAME) {
			LOGD << "Run blank frame";
			matchFirstControllerToTASController(0);
			runSingleFrame();
		} else if(data.actFlag == SendInfo::START_TAS_MODE) {
			LOGD << "Start TAS mode";
			pauseApp();
		}
	})

	// clang-format off
	CHECK_QUEUE(networkInstance, SendSetNumControllers, {
		LOGD << "Set controller number";
		setControllerNumber(data.size);
	})
	// clang-format on

	CHECK_QUEUE(networkInstance, SendAutoRun, {
		autoRunOn = data.start;
		if(data.start) {
			nanosecondsBetweenAutorun = (1000 / (float)data.fps) * 1000000;
		}
	})
}

void MainLoop::sendGameInfo() {
	if(applicationOpened) {

		std::vector<GameMemoryInfo> memoryInfo;

		// Will get more info via
		// https://github.com/switchbrew/switch-examples/blob/master/account/source/main.c

		uint64_t addr = 0;
		pauseApp();
		while(true) {
			MemoryInfo info = { 0 };
			uint32_t pageinfo;
			rc = svcQueryDebugProcessMemory(&info, &pageinfo, applicationDebug, addr);
			memoryInfo.push_back(getGameMemoryInfo(info));
			addr += info.size;

			if(R_FAILED(rc)) {
				break;
			}
		}
		unpauseApp();

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

void MainLoop::setControllerNumber(uint8_t numOfControllers) {
	controllers.clear();
	// Wait for all controllers to be disconnected
	while(true) {
		// User has to disconnect controllers
		// Before this loop is unblocked
		if(getNumControllers() == 0) {
			break;
		}
		LOGD << "Waiting for disconnect controllers: " << (int)getNumControllers();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	LOGD << "All controllers disconnected";
	for(uint8_t i = 0; i < numOfControllers; i++) {
		controllers.push_back(std::make_unique<ControllerHandler>(networkInstance));
	}
	// clang-format off
	ADD_TO_QUEUE(RecieveFlag, networkInstance, {
		data.actFlag = RecieveInfo::CONTROLLERS_CONNECTED;
	})
	// clang-format on
	// Now, user is required to reconnect any controllers manually
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

void MainLoop::runSingleFrame() {
	if(isPaused) {
		LOGD << "Running frame";
		unpauseApp();
		waitForVsync();
		screenshotHandler.writeFramebuffer(networkInstance);
		pauseApp();
	}
}

void MainLoop::pauseApp() {
	if(!isPaused) {
		// Debug application again
		LOGD << "Pausing";

		rc       = svcDebugActiveProcess(&applicationDebug, applicationProcessId);
		isPaused = true;

		screenshotHandler.writeFramebuffer(networkInstance);
		/*
				for(auto const& memoryRegion : memoryRegions) {
					std::vector<uint8_t> buf(memoryRegion.second);
					svcReadDebugProcessMemory(buf.data(), applicationDebug, memoryRegion.first, memoryRegion.second);

					ADD_TO_QUEUE(RecieveMemoryRegion, networkInstance, {
						data.startByte = memoryRegion.first;
						data.size      = memoryRegion.second;
						data.memory    = buf;
					})
				}
				*/
	}
}

void MainLoop::matchFirstControllerToTASController(uint8_t player) {
	if(getNumControllers() > controllers.size()) {
		// This should get the first non-TAS controller
		HidControllerID id = (HidControllerID)((int)controllers.size());

		u64 buttons = hidKeysHeld(id);
		JoystickPosition left;
		JoystickPosition right;
		hidJoystickRead(&left, id, JOYSTICK_LEFT);
		hidJoystickRead(&right, id, JOYSTICK_RIGHT);

		controllers[player]->setFrame(buttons, left, right);
	}
}

MainLoop::~MainLoop() {
	LOGD << "Exiting app";
	rc = hiddbgReleaseHdlsWorkBuffer();

	// Make absolutely sure the app is unpaused on close
	reset();

	hiddbgExit();
}
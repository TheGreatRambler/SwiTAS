#include "mainLoopHandler.hpp"

MainLoop::MainLoop() {
#ifdef __SWITCH__
	LOGD << "Start networking";
#endif
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
			RECIEVE_QUEUE_DATA(SendFrameData)
			RECIEVE_QUEUE_DATA(SendLogging)
			RECIEVE_QUEUE_DATA(SendTrackMemoryRegion)
			RECIEVE_QUEUE_DATA(SendSetNumControllers)
			RECIEVE_QUEUE_DATA(SendAutoRun)
		});

#ifdef __SWITCH__
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
#endif
}

void MainLoop::mainLoopHandler() {
	if(!isPaused) {
#ifdef __SWITCH__
		// Being debugged might break this application
		rc = pmdmntGetApplicationProcessId(&applicationProcessId);
		// Lifted from switchPresense-Rewritten
		uint8_t succeeded = R_SUCCEEDED(rc);
#else
		uint8_t succeeded = true;
#endif

		if(succeeded) {
// Application connected
// Get application info
#ifdef __SWITCH__
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
#endif
		} else {
			// I believe this means that there is no application running
			// If there was just an application open, let the PC know
			if(applicationOpened) {
#ifdef __SWITCH__
				LOGD << "Application closed";
#endif
				// clang-format off
			ADD_TO_QUEUE(RecieveFlag, networkInstance, {
				data.actFlag = RecieveInfo::APPLICATION_DISCONNECTED;
			})
				// clang-format on
				applicationOpened = false;
			}
		}
	}

	if(networkInstance->isConnected()) {
		if(!internetConnected) {
#ifdef __SWITCH__
			LOGD << "Internet connected";
#endif
			internetConnected = true;
		}
	} else {
		if(internetConnected) {
#ifdef __SWITCH__
			LOGD << "Internet disconnected";
#endif
			internetConnected = false;

			// Force unpause to not get user stuck if network cuts out
			reset();
		}
	}

	if(applicationOpened) {
		// handle network updates always, they are stored in the queue regardless of the internet
		handleNetworkUpdates();
	}

	// Match first controller inputs as often as possible
	if(!isPaused) {
		// TODO handle when running final TAS
		matchFirstControllerToTASController(0);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void MainLoop::handleNetworkUpdates() {
	CHECK_QUEUE(networkInstance, SendFrameData, {
		controllers[data.playerIndex]->setFrame(data.controllerData);
		if(data.incrementFrame) {
			runSingleFrame(true, false, data.frame, data.savestateHookNum, data.playerIndex);
		}
	})

	CHECK_QUEUE(networkInstance, SendTrackMemoryRegion, {
#ifdef __SWITCH__
		LOGD << "Track memory region";
#endif
		memoryRegions.push_back(std::pair<uint64_t, uint64_t>(data.startByte, data.size));
	})

	CHECK_QUEUE(networkInstance, SendFlag, {
		if(data.actFlag == SendInfo::PAUSE_DEBUG) {
			// Precaution to prevent the app getting stuck without the
			// User able to unpause it
			if(applicationOpened && internetConnected) {
				pauseApp(false, false, 0, 0, 0);
			}
		} else if(data.actFlag == SendInfo::UNPAUSE_DEBUG) {
			if(applicationOpened) {
				clearEveryController();
				unpauseApp();
			}
		} else if(data.actFlag == SendInfo::GET_FRAMEBUFFER) {
			if(applicationOpened) {
				// For now, unsupported
				// screenshotHandler.writeFramebuffer(networkInstance, 0, 0, 0, 0);
			}
		} else if(data.actFlag == SendInfo::RUN_BLANK_FRAME) {
			matchFirstControllerToTASController(0);
			runSingleFrame(false, false, 0, 0, 0);
		} else if(data.actFlag == SendInfo::START_TAS_MODE) {
			pauseApp(false, false, 0, 0, 0);
		} else if(data.actFlag == SendInfo::PAUSE) {
			pauseApp(false, false, 0, 0, 0);
		} else if(data.actFlag == SendInfo::UNPAUSE) {
			clearEveryController();
			unpauseApp();
		}
	})

	// clang-format off
	CHECK_QUEUE(networkInstance, SendSetNumControllers, {
		#ifdef __SWITCH__
		LOGD << "Set controller number";
		#endif
		setControllerNumber(data.size);
	})
	// clang-format on

	// TODO add logic to handle lua scripting

	// This is essentially auto advance but with frame linked framebuffers
	CHECK_QUEUE(networkInstance, SendAutoRun, {
		matchFirstControllerToTASController(0);
		runSingleFrame(true, true, data.frameReturn, data.savestateHookNum, data.playerIndex);
	})
}

void MainLoop::sendGameInfo() {
	if(applicationOpened) {

		std::vector<GameMemoryInfo> memoryInfo;

		// Will get more info via
		// https://github.com/switchbrew/switch-examples/blob/master/account/source/main.c

		uint64_t addr = 0;
		pauseApp(false, false, 0, 0, 0);
#ifdef __SWITCH__
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
#endif
		unpauseApp();

		ADD_TO_QUEUE(RecieveGameInfo, networkInstance, {
			data.applicationName      = gameName;
			data.applicationProgramId = applicationProgramId;
			data.applicationProcessId = applicationProcessId;
			data.memoryInfo           = memoryInfo;
		})
	}
}

#ifdef __SWITCH__
char* MainLoop::getAppName(u64 application_id) {
#ifdef __SWITCH__
	static NsApplicationControlData appControlData = { 0 };
	size_t appControlDataSize                      = 0;
	NacpLanguageEntry* languageEntry               = nullptr;

	if(R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, application_id, &appControlData, sizeof(NsApplicationControlData), &appControlDataSize))) {
		if(R_SUCCEEDED(nacpGetLanguageEntry(&appControlData.nacp, &languageEntry))) {
			if(languageEntry != nullptr)
				return languageEntry->name;
		}
	}
#endif
	return (char*)"Game Not Defined";
}
#endif

uint8_t MainLoop::getNumControllers() {
#ifdef __SWITCH__
	uint8_t num = 0;

	hidScanInput();
	for(int i = 0; i < 10; i++) {
		if(hidIsControllerConnected((HidControllerID)i)) {
			num++;
		}
	}

	return num;
#endif
}

void MainLoop::setControllerNumber(uint8_t numOfControllers) {
#ifdef __SWITCH__
	controllers.clear();
	// Wait for all controllers to be disconnected
	LOGD << (int)getNumControllers();
	while(getNumControllers() != 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	for(uint8_t i = 0; i < numOfControllers; i++) {
		controllers.push_back(std::make_unique<ControllerHandler>(networkInstance));
	}
	// clang-format off
	ADD_TO_QUEUE(RecieveFlag, networkInstance, {
		data.actFlag = RecieveInfo::CONTROLLERS_CONNECTED;
	})
	// clang-format on
	// Now, user is required to reconnect any controllers manually
#endif
}

#ifdef __SWITCH__
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
#endif

void MainLoop::runSingleFrame(uint8_t linkedWithFrameAdvance, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex) {
	if(isPaused) {
#ifdef __SWITCH__
		LOGD << "Running frame";
#endif
		unpauseApp();
		waitForVsync();
		pauseApp(linkedWithFrameAdvance, autoAdvance, frame, savestateHookNum, playerIndex);
	}
}

void MainLoop::clearEveryController() {
	for(uint8_t i = 0; i < controllers.size(); i++) {
		controllers[i]->clearState();
		controllers[i]->setInput();
	}
}

void MainLoop::pauseApp(uint8_t linkedWithFrameAdvance, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint8_t playerIndex) {
	// This is aborting for some reason
	if(!isPaused) {
		// Debug application again

#ifdef __SWITCH__
		LOGD << "Pausing";
		rc       = svcDebugActiveProcess(&applicationDebug, applicationProcessId);
		isPaused = true;
#endif

		if(networkInstance->isConnected()) {
			// Framebuffers should not be stored in memory unless they will be sent over internet
			LOGD << "Oh no";
			std::vector<uint8_t> jpegBuf(JPEG_BUF_SIZE);
			screenshotHandler.writeFramebuffer(jpegBuf);
			LOGD << "Written";
			ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
				data.buf              = jpegBuf;
				data.fromFrameAdvance = linkedWithFrameAdvance;
				data.frame            = frame;
				data.savestateHookNum = savestateHookNum;
				data.playerIndex      = playerIndex;
				if(autoAdvance) {
					data.controllerData = controllers[0]->getControllerData();
				} else {
					data.controllerData = nullptr;
				}
			})
			LOGD << "Sent";
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
}

void MainLoop::matchFirstControllerToTASController(uint8_t player) {
#ifdef __SWITCH__
	if(getNumControllers() > controllers.size() && controllers.size() != 0) {
		hidScanInput();
		// This should get the first non-TAS controller
		HidControllerID id = (HidControllerID)((int)controllers.size());

		u64 buttons = hidKeysHeld(id) & 65535;
		JoystickPosition left;
		JoystickPosition right;
		hidJoystickRead(&left, id, JOYSTICK_LEFT);
		hidJoystickRead(&right, id, JOYSTICK_RIGHT);

		controllers[player]->setFrame(buttons, left, right);
	}
#endif
}

MainLoop::~MainLoop() {
#ifdef __SWITCH__
	LOGD << "Exiting app";
	rc = hiddbgReleaseHdlsWorkBuffer();
	hiddbgExit();
#endif

	// Make absolutely sure the app is unpaused on close
	reset();
}
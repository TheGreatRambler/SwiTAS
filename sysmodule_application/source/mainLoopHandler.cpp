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
			RECIEVE_QUEUE_DATA(SendAddMemoryRegion)
			RECIEVE_QUEUE_DATA(SendStartFinalTas)
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
		if(data.incrementFrame) {
			runSingleFrame(true, data.includeFramebuffer, false, data.frame, data.savestateHookNum, data.branchIndex, data.playerIndex);
		} else if(data.isAutoRun) {
			matchFirstControllerToTASController(data.playerIndex);
			runSingleFrame(true, data.includeFramebuffer, true, data.frame, data.savestateHookNum, data.branchIndex, data.playerIndex);
		} else {
			controllers[data.playerIndex]->setFrame(data.controllerData);
		}
	})

	/*
		CHECK_QUEUE(networkInstance, SendTrackMemoryRegion, {
	#ifdef __SWITCH__
			LOGD << "Track memory region";
	#endif
			memoryRegions.push_back(std::pair<uint64_t, uint64_t>(data.startByte, data.size));
		})
		*/

	CHECK_QUEUE(networkInstance, SendFlag, {
		if(data.actFlag == SendInfo::PAUSE_DEBUG) {
			// Precaution to prevent the app getting stuck without the
			// User able to unpause it
			if(applicationOpened && internetConnected) {
				pauseApp(false, true, false, 0, 0, 0, 0);
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
			runSingleFrame(false, true, false, 0, 0, 0, 0);
		} else if(data.actFlag == SendInfo::START_TAS_MODE) {
			// pauseApp(false, true, false, 0, 0, 0, 0);
		} else if(data.actFlag == SendInfo::PAUSE) {
			pauseApp(false, true, false, 0, 0, 0, 0);
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

	CHECK_QUEUE(networkInstance, SendAddMemoryRegion, {
		if(data.clearAllRegions) {
			currentMemoryRegions.clear();
		} else {
			MemoryRegionInfo info;

			// prepareMemoryRegionMath(info.func, data.pointerDefinition);

			info.type = data.type;
			info.u    = data.u;
			info.size = data.dataSize;
			currentMemoryRegions.push_back(info);
		}
	})

	// clang-format off
	CHECK_QUEUE(networkInstance, SendStartFinalTas, {
		finalTasShouldRun = true;
		runFinalTas(data.scriptPaths);
	})
	// clang-format on

	// TODO add logic to handle lua scripting
}

void MainLoop::sendGameInfo() {
	if(applicationOpened) {

		std::vector<GameMemoryInfo> memoryInfo;

		// Will get more info via
		// https://github.com/switchbrew/switch-examples/blob/master/account/source/main.c

		uint64_t addr = 0;
		pauseApp(false, true, false, 0, 0, 0, 0);
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

void MainLoop::runFinalTas(std::vector<std::string> scriptPaths) {
	std::vector<FILE*> files;
	for(auto const& path : scriptPaths) {
		files.push_back(fopen(path.c_str(), "rb"));
	}

	uint8_t filesSize = files.size();

	unpauseApp();

	while(true) {
		// Run half a second of data before checking network
		if(!finalTasShouldRun)
			break;

		for(uint8_t i = 0; i < 30; i++) {
			// File reading can't slow down at all

			for(uint8_t player = 0; player < filesSize; player++) {
				// Based on code in project handler without compression
				uint8_t controllerSize;
				readFullFileData(files[player], &controllerSize, sizeof(controllerSize));

				uint8_t controllerDataBuf[controllerSize];
				readFullFileData(files[player], controllerDataBuf, sizeof(controllerDataBuf));

				ControllerData data;
				serializeProtocol.binaryToData<ControllerData>(data, controllerDataBuf, controllerSize);

				controllers[player]->setFrame(data);
			}

			// Either put this before or after
			waitForVsync();
		}

		handleNetworkUpdates();
	}

	for(auto const& file : files) {
		fclose(file);
	}
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

void MainLoop::runSingleFrame(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex) {
	if(isPaused) {
#ifdef __SWITCH__
		LOGD << "Running frame";
#endif
		waitForVsync();
		unpauseApp();
		waitForVsync();
		pauseApp(linkedWithFrameAdvance, includeFramebuffer, autoAdvance, frame, savestateHookNum, branchIndex, playerIndex);
	}
}

void MainLoop::clearEveryController() {
	for(uint8_t i = 0; i < controllers.size(); i++) {
		controllers[i]->clearState();
		controllers[i]->setInput();
	}
}

void MainLoop::pauseApp(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex) {
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
			std::vector<uint8_t> jpegBuf;
			std::string dhash;

			if(includeFramebuffer) {
				screenshotHandler.writeFramebuffer(jpegBuf, dhash);
			}

			ADD_TO_QUEUE(RecieveGameFramebuffer, networkInstance, {
				data.buf = jpegBuf;
				// if(includeFramebuffer) {
				//	data.dhash = dhash;
				//}
				data.fromFrameAdvance       = linkedWithFrameAdvance;
				data.frame                  = frame;
				data.savestateHookNum       = savestateHookNum;
				data.branchIndex            = branchIndex;
				data.playerIndex            = playerIndex;
				data.controllerDataIncluded = autoAdvance;
				if(autoAdvance) {
					data.controllerData = *controllers[0]->getControllerData();
				}
			})

			// TODO set main and handle types correctly
			// Put data into a vector<uint_t> first
			for(uint16_t i = 0; i < currentMemoryRegions.size(); i++) {
				uint64_t addr      = 0; // currentMemoryRegions[i].func.Eval();
				uint8_t isUnsigned = currentMemoryRegions[i].u;

				MemoryRegionTypes type = currentMemoryRegions[i].type;
				std::vector<uint8_t> bytes;
				std::string stringVersion;

				switch(type) {
				case MemoryRegionTypes::Bit8:
					bytes = getMemory(addr, sizeof(uint8_t));
					if(isUnsigned) {
						stringVersion = std::to_string(*(uint8_t*)bytes.data());
					} else {
						stringVersion = std::to_string(*(int8_t*)bytes.data());
					}
					break;
				case MemoryRegionTypes::Bit16:
					bytes = getMemory(addr, sizeof(uint16_t));
					if(isUnsigned) {
						stringVersion = std::to_string(*(uint16_t*)bytes.data());
					} else {
						stringVersion = std::to_string(*(int16_t*)bytes.data());
					}
					break;
				case MemoryRegionTypes::Bit32:
					bytes = getMemory(addr, sizeof(uint32_t));
					if(isUnsigned) {
						stringVersion = std::to_string(*(uint32_t*)bytes.data());
					} else {
						stringVersion = std::to_string(*(int32_t*)bytes.data());
					}
					break;
				case MemoryRegionTypes::Bit64:
					bytes = getMemory(addr, sizeof(uint64_t));
					if(isUnsigned) {
						stringVersion = std::to_string(*(uint64_t*)bytes.data());
					} else {
						stringVersion = std::to_string(*(int64_t*)bytes.data());
					}
					break;
				case MemoryRegionTypes::Float:
					bytes         = getMemory(addr, sizeof(float));
					stringVersion = std::to_string(*(float*)bytes.data());
					break;
				case MemoryRegionTypes::Double:
					bytes         = getMemory(addr, sizeof(double));
					stringVersion = std::to_string(*(double*)bytes.data());
					break;
				case MemoryRegionTypes::CharPointer:
					bytes         = getMemory(addr, currentMemoryRegions[i].size);
					stringVersion = std::string((const char*)bytes.data(), bytes.size());
					break;
				case MemoryRegionTypes::ByteArray:
					bytes = getMemory(addr, currentMemoryRegions[i].size);
					// Unused
					stringVersion = "";
					break;
				}

				ADD_TO_QUEUE(RecieveMemoryRegion, networkInstance, {
					data.memory               = bytes;
					data.stringRepresentation = stringVersion;
					data.index                = i;
				})
			}

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
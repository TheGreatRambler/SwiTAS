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
			RECIEVE_QUEUE_DATA(SendFinalTasChunk)
		});

#ifdef __SWITCH__
	LOGD << "Open display";
	rc = viOpenDefaultDisplay(&disp);
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Get vsync event";
	rc = viGetDisplayVsyncEvent(&disp, &vsyncEvent);
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Obtain sleep module";
	// https://github.com/cathery/sys-con/blob/master/source/Sysmodule/source/psc_module.cpp
	const u16 deps[1] = { PscPmModuleId_Fs };
	rc                = pscmGetPmModule(&sleepModule, (PscPmModuleId)127, deps, sizeof(deps), true);
	if(R_FAILED(rc))
		fatalThrow(rc);

	LOGD << "Attach work buffers";
	// Attach Work Buffer
	rc = hiddbgAttachHdlsWorkBuffer();
	if(R_FAILED(rc))
		fatalThrow(rc);

#endif

#ifdef YUZU
	yuzuSyscalls = std::make_shared<Syscalls>();
	screenshotHandler.setYuzuInstance(yuzuInstance);
#endif
}

void MainLoop::mainLoopHandler() {
	/*
	if(checkSleep()) {
		// Close down server
	}

	if(checkAwaken()) {
		// restart server
	}
	*/

	if(!isPaused) {
#ifdef __SWITCH__
		// Being debugged might break this application
		rc = pmdmntGetApplicationProcessId(&applicationProcessId);
		// Lifted from switchPresense-Rewritten
		uint8_t succeeded = R_SUCCEEDED(rc);
#else
		uint8_t succeeded = yuzuSyscalls->function_emu_emulating(yuzuSyscalls->getYuzuInstance());
#endif

		if(succeeded) {
// Application connected
// Get application info
#ifdef __SWITCH__
			if(R_SUCCEEDED(pminfoGetProgramId(&applicationProgramId, applicationProcessId))) {
				if(!applicationOpened) {
					svcSleepThread((s64)1000000 * 1000);

					// Check if this file exists first of all
					LOGD << "Debug application at process: " << std::to_string(applicationProcessId);

					rc = svcDebugActiveProcess(&applicationDebug, applicationProcessId);
					if(R_FAILED(rc))
						fatalThrow(rc);

					LOGD << "Get game name";
					gameName = std::string(getAppName(applicationProgramId));

					LOGD << "Get SaltyNX data";
					FILE* offsets = fopen(saltyPluginPath, "rb");

					// Used to do accurate frame advance
					fread(&saltynxframeHasPassed, sizeof(uint64_t), 1, offsets);
					fread(&saltynxlogStringIndex, sizeof(uint64_t), 1, offsets);
					fread(&saltynxlogString, sizeof(uint64_t), 1, offsets);
					fread(&saltynxcontrollerToRecord, sizeof(uint64_t), 1, offsets);
					fread(&saltynxsixAxisStateLeftJoycon, sizeof(uint64_t), 1, offsets);
					fread(&saltynxsixAxisStateRightJoycon, sizeof(uint64_t), 1, offsets);
					fread(&saltynxsixAxisStateLeftJoyconBacklog, sizeof(uint64_t), 1, offsets);
					fread(&saltynxsixAxisStateRightJoyconBacklog, sizeof(uint64_t), 1, offsets);
					fread(&saltynxrecordScreenOrKeyboard, sizeof(uint64_t), 1, offsets);
					fread(&saltynxtouchscreenState, sizeof(uint64_t), 1, offsets);
					fread(&saltynxtouchScreenStateBacklog, sizeof(uint64_t), 1, offsets);
					fread(&saltynxkeyboardState, sizeof(uint64_t), 1, offsets);
					fread(&saltynxkeyboardStateBacklog, sizeof(uint64_t), 1, offsets);
					fread(&saltynxmouseState, sizeof(uint64_t), 1, offsets);
					fread(&saltynxmouseStateBacklog, sizeof(uint64_t), 1, offsets);

					fclose(offsets);

					// Obtain heap start
					MemoryInfo meminfo;
					u64 lastaddr = 0;
					do {
						lastaddr = meminfo.addr;
						u32 pageinfo;
						svcQueryDebugProcessMemory(&meminfo, &pageinfo, applicationDebug, meminfo.addr + meminfo.size);
						if((meminfo.type & MemType_Heap) == MemType_Heap) {
							heapBase = meminfo.addr;
							break;
						}
					} while(lastaddr < meminfo.addr + meminfo.size);

					// Obtain main start
					LoaderModuleInfo proc_modules[2];
					s32 numModules = 0;
					Result rc      = ldrDmntGetProcessModuleInfo(applicationProcessId, proc_modules, 2, &numModules);

					LoaderModuleInfo* proc_module = 0;
					if(numModules == 2) {
						proc_module = &proc_modules[1];
					} else {
						proc_module = &proc_modules[0];
					}
					mainBase = proc_module->base_address;

					LOGD << "Unpause app";
					rc = svcCloseHandle(applicationDebug);
					if(R_FAILED(rc))
						fatalThrow(rc);

					LOGD << "Application " + gameName + " opened";
					ADD_TO_QUEUE(RecieveApplicationConnected, networkInstance, {
						data.applicationName      = gameName;
						data.applicationProgramId = applicationProgramId;
						data.applicationProcessId = applicationProcessId;
					})

					applicationOpened = true;
				}
			}
#endif
#ifdef YUZU
			char* gameNamePointer = yuzuSyscalls->function_emu_romname(yuzuSyscalls->getYuzuInstance());
			gameName              = std::string(gameNamePointer);
			yuzuSyscalls->function_meta_free(gameNamePointer);

			applicationProgramId = yuzuSyscalls->function_emu_getprogramid(yuzuSyscalls->getYuzuInstance());
			applicationProcessId = yuzuSyscalls->function_emu_getprocessid(yuzuSyscalls->getYuzuInstance());

			// pauseApp(false, false, false, 0, 0, 0, 0);

			heapBase = yuzuSyscalls->function_emu_getheapstart(yuzuSyscalls->getYuzuInstance());
			mainBase = yuzuSyscalls->function_emu_getmainstart(yuzuSyscalls->getYuzuInstance());

			// unpauseApp();

			// yuzuSyscalls->function_emu_log(yuzuSyscalls->getYuzuInstance(), "Application opened");

			ADD_TO_QUEUE(RecieveApplicationConnected, networkInstance, {
				data.applicationName      = gameName;
				data.applicationProgramId = applicationProgramId;
				data.applicationProcessId = applicationProcessId;
			})

			applicationOpened = true;
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
		// Handle SaltyNX output
		uint16_t logOutputSize = getMemoryType<uint16_t>(saltynxlogStringIndex);
		if(logOutputSize != 0) {
			std::vector<unsigned char> logData = getMemory(saltynxlogString, logOutputSize);

			LOGD << std::string((char*)logData.data(), logOutputSize);

			setMemoryType(saltynxlogStringIndex, (uint16_t)0);
		}
	}

	// Match first controller inputs as often as possible
	if(!isPaused) {
		// TODO handle when running final TAS
		matchFirstControllerToTASController(0);
	}

#ifdef __SWITCH__
	// Sleep for 5 milliseconds
	svcSleepThread((s64)1000000 * 5);
#endif

	// I dunno how often to update this honestly
	updateGui();
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

	CHECK_QUEUE(networkInstance, SendFlag, {
		if(data.actFlag == SendInfo::PAUSE_DEBUG) {
			// Precaution to prevent the app getting stuck without the
			// User able to unpause it
			if(applicationOpened && internetConnected) {
				pauseApp(false, true, false, 0, 0, 0, 0);
				lastNanoseconds = 0;
			}
		} else if(data.actFlag == SendInfo::UNPAUSE_DEBUG) {
			if(applicationOpened) {
				clearEveryController();
				unpauseApp();
				lastNanoseconds  = 0;
				lastFrameAttempt = 0;
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
			// waitForVsync();
			pauseApp(false, true, false, 0, 0, 0, 0);
		} else if(data.actFlag == SendInfo::UNPAUSE) {
			clearEveryController();
			unpauseApp();
			lastNanoseconds  = 0;
			lastFrameAttempt = 0;
		} else if(data.actFlag == SendInfo::STOP_FINAL_TAS) {
			finalTasShouldRun = false;
		} else if(data.actFlag == SendInfo::GET_GAME_INFO) {
			sendGameInfo();
		} else if(data.actFlag == SendInfo::GET_IS_YUZU) {
#ifdef __SWITCH__
			// clang-format off
			ADD_TO_QUEUE(RecieveFlag, networkInstance, {
				data.actFlag = RecieveInfo::IS_HOMEBREW;
			})
// clang-format on
#endif
#ifdef YUZU
			// clang-format off
			ADD_TO_QUEUE(RecieveFlag, networkInstance, {
				data.actFlag = RecieveInfo::IS_YUZU;
			})
// clang-format on
#endif
		}
	})

	CHECK_QUEUE(networkInstance, SendFinalTasChunk, {
		if(data.closeFile) {
			fclose(runFinalTasFileHandles[data.path]);
			runFinalTasFileHandles.erase(data.path);
		} else {
			if(data.openFile) {
				remove(data.path.c_str());
				runFinalTasFileHandles[data.path] = fopen(data.path.c_str(), "wb");
			} else {
				fwrite(data.contents.data(), data.contents.size(), 1, runFinalTasFileHandles[data.path]);
			}
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

			info.type              = data.type;
			info.u                 = data.u;
			info.size              = data.dataSize;
			info.pointerDefinition = data.pointerDefinition;
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
		ADD_TO_QUEUE(RecieveGameInfo, networkInstance, {
			data.applicationName      = gameName;
			data.applicationProgramId = applicationProgramId;
			data.applicationProcessId = applicationProcessId;

#ifdef __SWITCH__
			AccountUid uid;
			AccountProfile profile;
			AccountUserData userdata;
			AccountProfileBase profileBase;
			accountGetLastOpenedUser(&uid);
			accountGetProfile(&profile, uid);
			accountProfileGet(&profile, &userdata, &profileBase);

			// If it uses all charactors, (ie has no NULL char), may need to get more smart
			data.userNickname = std::string(profileBase.nickname);
#endif

			// I don't know how to handle this for Yuzu, so ignore for now

#ifdef __SWITCH__
			uint64_t addr = 0;
			while(true) {
				// LOGD << "Obtained memory region at: " << addr;

				MemoryInfo info = { 0 };
				uint32_t pageinfo;
				rc = svcQueryDebugProcessMemory(&info, &pageinfo, applicationDebug, addr);

				addr = info.addr + info.size;

				if(R_FAILED(rc)) {
					break;
				}

				MemoryDataInfo::MemoryInfo memoryInfo = { 0 };

				memoryInfo.addr            = info.addr;
				memoryInfo.size            = info.size;
				memoryInfo.type            = info.type;
				memoryInfo.attr            = info.attr;
				memoryInfo.perm            = info.perm;
				memoryInfo.device_refcount = info.device_refcount;
				memoryInfo.ipc_refcount    = info.ipc_refcount;

				data.memoryInfo.push_back(memoryInfo);
			}
#endif

#ifdef __SWITCH__
			LOGD << "Game info sent";
#endif
		})
	}
}

void MainLoop::updateGui() {
	if(printDebugInfo || printControllerOverlay) {
		if(!gui) {
#ifdef __SWITCH__
			gui = std::make_shared<Gui>(&disp);
#endif
#ifdef YUZU
			gui = std::make_shared<Gui>(yuzuSyscalls);
#endif
		}

		gui->startFrame();

		if(printControllerOverlay) {
			for(uint8_t controllerIndex = 0; controllerIndex < controllers.size(); controllerIndex++) {
				gui->drawControllerOverlay(controllerIndex, controllers[controllerIndex]->getInput());
			}
		}

		if(printDebugInfo) {
			std::string debugInfo;
			// clang-format off
				debugInfo += "TAS Controllers:                " + std::to_string(controllers.size()) + "\n";
				debugInfo += "Real Controllers:               " + std::to_string(getNumControllers()) + "\n";
			if(applicationOpened) {
				debugInfo += "Game Name:                      " + gameName + "\n";
				debugInfo += "Application Program ID:         " + std::to_string(applicationProgramId) + "\n";
				debugInfo += "Application Process ID:         " + std::to_string(applicationProcessId) + "\n";
				debugInfo += "Is Paused:                      " + std::string((isPaused ? "true" : "false")) + "\n";
				debugInfo += "Time Between Frames:            " + lastFrameAttempt != 0 ? std::to_string(lastFrameAttempt) : std::string("NULL") + "\n";
			}
			// clang-format on
			gui->drawText(0, 0, 28, debugInfo);
		}

		gui->endFrame();
	} else {
		if(gui && gui->getWasJustDrawnTo()) {
			gui->clearFrame();
		}
		return;
	}
}

#ifdef __SWITCH__
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
#ifdef YUZU
	return yuzuSyscalls->function_joypad_getnumjoypads(yuzuSyscalls->getYuzuInstance());
#endif
}

void MainLoop::setControllerNumber(uint8_t numOfControllers) {
	controllers.clear();
#ifdef __SWITCH__
	// Wait for all controllers to be disconnected
	LOGD << (int)getNumControllers();
	while(getNumControllers() != 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	// Once user leaves menu, wait 3 seconds
	svcSleepThread((int64_t)1000000 * 3000);
#endif
#ifdef YUZU
	yuzuSyscalls->function_joypad_setnumjoypads(yuzuSyscalls->getYuzuInstance(), 0);
#endif
	for(uint8_t i = 0; i < numOfControllers; i++) {
#ifdef __SWITCH__
		controllers.push_back(std::make_unique<ControllerHandler>(networkInstance, (HidControllerID)getNumControllers()));
#endif
#ifdef YUZU
		controllers.push_back(std::make_unique<ControllerHandler>(networkInstance, yuzuSyscalls));
#endif
	}
	// clang-format off
	ADD_TO_QUEUE(RecieveFlag, networkInstance, {
		data.actFlag = RecieveInfo::CONTROLLERS_CONNECTED;
	})
	// clang-format on
	// Now, user is required to reconnect any controllers manually
}

void MainLoop::runFinalTas(std::vector<std::string> scriptPaths) {
	std::vector<FILE*> files;
	for(auto const& path : scriptPaths) {
#ifdef __SWITCH__
		LOGD << "Open script at: " << path;
#endif
		files.push_back(fopen(path.c_str(), "rb"));
	}

	uint8_t filesSize = files.size();

	// Just in case
	unpauseApp();
	lastNanoseconds = 0;

	uint8_t shouldRun     = true;
	uint8_t shouldAdvance = false;

	uint32_t frameNum = 0;
	while(true) {
		LOGD << "Start half second";
		// Run half a second of data before checking network
		if(!finalTasShouldRun)
			break;

		if(shouldRun) {
			for(uint8_t player = 0; player < filesSize; player++) {
				LOGD << "For player " << (int)player;
				// Based on code in project handler without compression
				uint8_t controllerSize;
				HELPERS::readFullFileData(files[player], &controllerSize, sizeof(controllerSize));

				if(controllerSize == 255) {
					// Skip handling controller data and clear existing buttons
					controllers[player]->clearState();
					controllers[player]->setInput();
					LOGD << "Empty frame";
				} else {
					uint8_t controllerDataBuf[controllerSize];
					HELPERS::readFullFileData(files[player], controllerDataBuf, controllerSize);

					ControllerData data;
					serializeProtocol.binaryToData<ControllerData>(data, controllerDataBuf, controllerSize);

					controllers[player]->setFrame(data);

					LOGD << "Run final TAS frame: " << (int)controllerSize;
				}
			}
		}

		waitForVsync();

		if(shouldAdvance) {
			shouldAdvance = false;
			shouldRun     = false;
			pauseApp(false, false, false, 0, 0, 0, 0);
		}

#ifdef __SWITCH__
		uint8_t numOfControllers = getNumControllers();
		if(numOfControllers > filesSize) {
			hidScanInput();
			uint64_t kDown = hidKeysDown((HidControllerID)filesSize);

			if(kDown & KEY_X) {
				shouldRun = false;
				pauseApp(false, false, false, 0, 0, 0, 0);
			}

			if(kDown & KEY_A) {
				shouldRun = true;
				unpauseApp();
			}

			if(kDown & KEY_DRIGHT) {
				shouldAdvance = true;
				unpauseApp();
			}
		}
#endif

		if(frameNum % 30 == 0) {
			handleNetworkUpdates();
		}

		frameNum++;
	}

	for(auto const& file : files) {
		fclose(file);
	}
}

void MainLoop::runSingleFrame(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex) {
	if(isPaused) {
#ifdef __SWITCH__
		LOGD << "Running frame";
		int32_t currentPriority;
		svcGetThreadPriority(&currentPriority, CUR_THREAD_HANDLE);
		svcSetThreadPriority(CUR_THREAD_HANDLE, 10);
#endif
		unpauseApp();
#ifdef __SWITCH__
		lastNanoseconds = armTicksToNs(armGetSystemTick());
		// waitForVsync();
		// Frame advancing is amazingly inconsistent
		// svcSleepThread(16666666);
		// New SaltyNX based version should be much more accurate
		waitForVsync();
#endif
#ifdef YUZU
		// Yuzu has actually good frame incrementing
		waitForVsync();
#endif
		pauseApp(linkedWithFrameAdvance, includeFramebuffer, autoAdvance, frame, savestateHookNum, branchIndex, playerIndex);
#ifdef __SWITCH__
		svcSetThreadPriority(CUR_THREAD_HANDLE, currentPriority);
#endif
	}
}

void MainLoop::clearEveryController() {
	for(uint8_t i = 0; i < controllers.size(); i++) {
		controllers[i]->clearState();
		controllers[i]->setInput();
	}
}

void MainLoop::pauseApp(uint8_t linkedWithFrameAdvance, uint8_t includeFramebuffer, uint8_t autoAdvance, uint32_t frame, uint16_t savestateHookNum, uint32_t branchIndex, uint8_t playerIndex) {
	if(!isPaused) {
#ifdef __SWITCH__
		LOGD << "Pausing";
		rc = svcDebugActiveProcess(&applicationDebug, applicationProcessId);
		if(R_FAILED(rc))
			fatalThrow(rc);
		isPaused = true;
#endif
#ifdef YUZU
		yuzuSyscalls->function_emu_pause(yuzuSyscalls->getYuzuInstance());
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

			std::vector<std::string> outputs;
			for(uint16_t i = 0; i < currentMemoryRegions.size(); i++) {
				std::string revisedExpression = currentMemoryRegions[i].pointerDefinition;

				HELPERS::replaceInString(revisedExpression, "main", std::to_string(mainBase));
				HELPERS::replaceInString(revisedExpression, "heap", std::to_string(heapBase));

				for(std::size_t outputIndex = 0; outputIndex < outputs.size(); outputIndex++) {
					// For each already generated output, replace in the expression if needed
					// Note, some values will absolutely cause an error
					HELPERS::replaceInString(revisedExpression, "<" + std::to_string(outputIndex) + ">", outputs[outputIndex]);
				}

				MemoryRegionTypes type = currentMemoryRegions[i].type;
				std::vector<uint8_t> bytes;
				std::string stringVersion;

				try {
#ifdef __SWITCH__
					uint64_t addr = calculator::eval<uint64_t>(revisedExpression, applicationDebug);
#endif
#ifdef YUZU
					uint64_t addr = calculator::eval<uint64_t>(revisedExpression, yuzuSyscalls);
#endif
					uint8_t isUnsigned = currentMemoryRegions[i].u;

					switch(type) {
					case MemoryRegionTypes::Bit8:
						bytes = getMemory(addr, sizeof(uint8_t));
						if(isUnsigned) {
							stringVersion = memoryToString<uint8_t>(bytes);
						} else {
							stringVersion = memoryToString<int8_t>(bytes);
						}
						break;
					case MemoryRegionTypes::Bit16:
						bytes = getMemory(addr, sizeof(uint16_t));
						if(isUnsigned) {
							stringVersion = memoryToString<uint16_t>(bytes);
						} else {
							stringVersion = memoryToString<int16_t>(bytes);
						}
						break;
					case MemoryRegionTypes::Bit32:
						bytes = getMemory(addr, sizeof(uint32_t));
						if(isUnsigned) {
							stringVersion = memoryToString<uint32_t>(bytes);
						} else {
							stringVersion = memoryToString<int32_t>(bytes);
						}
						break;
					case MemoryRegionTypes::Bit64:
						bytes = getMemory(addr, sizeof(uint64_t));
						if(isUnsigned) {
							stringVersion = memoryToString<uint64_t>(bytes);
						} else {
							stringVersion = memoryToString<int64_t>(bytes);
						}
						break;
					case MemoryRegionTypes::Float:
						bytes         = getMemory(addr, sizeof(float));
						stringVersion = memoryToString<float>(bytes);
						break;
					case MemoryRegionTypes::Double:
						bytes         = getMemory(addr, sizeof(double));
						stringVersion = memoryToString<double>(bytes);
						break;
					case MemoryRegionTypes::Bool:
						bytes         = getMemory(addr, sizeof(bool));
						stringVersion = *(bool*)bytes.data() ? "true" : "false";
						break;
					case MemoryRegionTypes::CharPointer:
						bytes         = getMemory(addr, currentMemoryRegions[i].size);
						stringVersion = std::string((const char*)bytes.data(), bytes.size());
						break;
					case MemoryRegionTypes::ByteArray:
						bytes = getMemory(addr, currentMemoryRegions[i].size);
						break;
					}

					outputs.push_back(stringVersion);
				} catch(calculator::error& e) {
					// Communicate error out with string version, honestly sus
					stringVersion = e.what();
				}

				ADD_TO_QUEUE(RecieveMemoryRegion, networkInstance, {
					data.memory               = bytes;
					data.stringRepresentation = stringVersion;
					data.index                = i;
				})
			}
		} else {
			LOGD << "Internet not connected, not sending framebuffer";
		}
	}
}

void MainLoop::waitForVsync() {
#ifdef __SWITCH__
	if(isPaused || saltynxframeHasPassed == 0) {
		rc = eventWait(&vsyncEvent, UINT64_MAX);
		if(R_FAILED(rc))
			fatalThrow(rc);
		// svcSleepThread(1000000 * 1);
	} else {
		LOGD << "Wait for vsync";
		while(true) {
			uint8_t frame = getMemoryType<uint8_t>(saltynxframeHasPassed);

			if(frame) {
				// Clear the variable so we can wait for it again
				setMemoryType(saltynxframeHasPassed, (uint8_t) false);
				return;
			} else {
				svcSleepThread(1000000 * 3);
			}
		}
	}
#endif
#ifdef YUZU
	yuzuSyscalls->function_emu_frameadvance(yuzuSyscalls->getYuzuInstance());
#endif
}

uint8_t MainLoop::checkSleep() {
#ifdef __SWITCH__
	// Wait for one millisecond
	if(R_SUCCEEDED(eventWait(&sleepModule.event, 1000000 * 1))) {
		PscPmState pscState;
		u32 out_flags;
		if(R_SUCCEEDED(pscPmModuleGetRequest(&sleepModule, &pscState, &out_flags))) {
			pscPmModuleAcknowledge(&sleepModule, pscState);
			switch(pscState) {
			case PscPmState_Awake:
			case PscPmState_ReadyAwaken:
				return false;
			case PscPmState_ReadySleep:
			case PscPmState_ReadyShutdown:
				return true;
			default:
				return false;
			}
		}
	}
#endif
}
uint8_t MainLoop::checkAwaken() {
#ifdef __SWITCH__
	// Wait for one millisecond
	if(R_SUCCEEDED(eventWait(&sleepModule.event, 1000000 * 1))) {
		PscPmState pscState;
		u32 out_flags;
		if(R_SUCCEEDED(pscPmModuleGetRequest(&sleepModule, &pscState, &out_flags))) {
			pscPmModuleAcknowledge(&sleepModule, pscState);
			switch(pscState) {
			case PscPmState_Awake:
			case PscPmState_ReadyAwaken:
				return true;
			case PscPmState_ReadySleep:
			case PscPmState_ReadyShutdown:
				return false;
			default:
				return false;
			}
		}
	}
#endif
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

		controllers[player]->setFrame(buttons, left.dx, left.dy, right.dx, right.dy);
	}
#endif
#ifdef YUZU
	if(getNumControllers() > controllers.size() && controllers.size() != 0) {
		// This should get the first non-TAS controller
		uint8_t controllerIndex = (uint8_t)controllers.size();

		uint64_t buttons = yuzuSyscalls->function_joypad_read(yuzuSyscalls->getYuzuInstance(), controllerIndex);

		int32_t leftX  = yuzuSyscalls->function_joypad_readjoystick(yuzuSyscalls->getYuzuInstance(), controllerIndex, YuzuJoystickType::LeftX);
		int32_t leftY  = yuzuSyscalls->function_joypad_readjoystick(yuzuSyscalls->getYuzuInstance(), controllerIndex, YuzuJoystickType::LeftY);
		int32_t rightX = yuzuSyscalls->function_joypad_readjoystick(yuzuSyscalls->getYuzuInstance(), controllerIndex, YuzuJoystickType::RightX);
		int32_t rightY = yuzuSyscalls->function_joypad_readjoystick(yuzuSyscalls->getYuzuInstance(), controllerIndex, YuzuJoystickType::RightY);

		controllers[player]->setFrame(buttons, leftX, leftY, rightX, rightY);
	}
#endif
}

void MainLoop::disableSixAxisModifying() {
	if(saltynxcontrollerToRecord != 0) {
		setMemoryType(saltynxcontrollerToRecord, nn::hid::NpadIdType::None);
	}
}

void MainLoop::setSixAxisControllerRecord(int32_t controller) {
	if(saltynxcontrollerToRecord != 0) {
		setMemoryType(saltynxcontrollerToRecord, controller);
	}
}

void MainLoop::disableKeyboardTouchModifying() {
	if(saltynxrecordScreenOrKeyboard != 0) {
		setMemoryType(saltynxrecordScreenOrKeyboard, (uint16_t)0);
	}
}

void MainLoop::setKeyboardRecord() {
	if(saltynxrecordScreenOrKeyboard != 0) {
		setMemoryType(saltynxrecordScreenOrKeyboard, (uint16_t)2);
	}
}

void MainLoop::setTouchRecord() {
	if(saltynxrecordScreenOrKeyboard != 0) {
		setMemoryType(saltynxrecordScreenOrKeyboard, (uint8_t)1);
	}
}

void MainLoop::getSixAxisState(int32_t controller, ControllerData* state) {
	size_t offset                                = sizeof(nn::hid::SixAxisSensorState) * nn::hid::SixAxisSensorStateCountMax * controller;
	nn::hid::SixAxisSensorState sensorStateLeft  = getMemoryType<nn::hid::SixAxisSensorState>(saltynxsixAxisStateLeftJoyconBacklog + offset);
	nn::hid::SixAxisSensorState sensorStateRight = getMemoryType<nn::hid::SixAxisSensorState>(saltynxsixAxisStateRightJoyconBacklog + offset);

	state->ACCEL_X_LEFT  = sensorStateLeft.acceleration.x;
	state->ACCEL_Y_LEFT  = sensorStateLeft.acceleration.y;
	state->ACCEL_Z_LEFT  = sensorStateLeft.acceleration.z;
	state->GYRO_X_LEFT   = sensorStateLeft.angularVelocity.x;
	state->GYRO_Y_LEFT   = sensorStateLeft.angularVelocity.y;
	state->GYRO_Z_LEFT   = sensorStateLeft.angularVelocity.z;
	state->ANGLE_X_LEFT  = sensorStateLeft.angle.x;
	state->ANGLE_Y_LEFT  = sensorStateLeft.angle.y;
	state->ANGLE_Z_LEFT  = sensorStateLeft.angle.z;
	state->ACCEL_X_RIGHT = sensorStateRight.acceleration.x;
	state->ACCEL_Y_RIGHT = sensorStateRight.acceleration.y;
	state->ACCEL_Z_RIGHT = sensorStateRight.acceleration.z;
	state->GYRO_X_RIGHT  = sensorStateRight.angularVelocity.x;
	state->GYRO_Y_RIGHT  = sensorStateRight.angularVelocity.y;
	state->GYRO_Z_RIGHT  = sensorStateRight.angularVelocity.z;
	state->ANGLE_X_RIGHT = sensorStateRight.angle.x;
	state->ANGLE_Y_RIGHT = sensorStateRight.angle.y;
	state->ANGLE_Z_RIGHT = sensorStateRight.angle.z;
}

void MainLoop::setSixAxisState(int32_t controller, ControllerData* state) {
	nn::hid::SixAxisSensorState sensorStateLeft;
	nn::hid::SixAxisSensorState sensorStateRight;

	sensorStateLeft.acceleration.x     = state->ACCEL_X_LEFT;
	sensorStateLeft.acceleration.y     = state->ACCEL_Y_LEFT;
	sensorStateLeft.acceleration.z     = state->ACCEL_Z_LEFT;
	sensorStateLeft.angularVelocity.x  = state->GYRO_X_LEFT;
	sensorStateLeft.angularVelocity.y  = state->GYRO_Y_LEFT;
	sensorStateLeft.angularVelocity.z  = state->GYRO_Z_LEFT;
	sensorStateLeft.angle.x            = state->ANGLE_X_LEFT;
	sensorStateLeft.angle.y            = state->ANGLE_Y_LEFT;
	sensorStateLeft.angle.z            = state->ANGLE_Z_LEFT;
	sensorStateRight.acceleration.x    = state->ACCEL_X_RIGHT;
	sensorStateRight.acceleration.y    = state->ACCEL_Y_RIGHT;
	sensorStateRight.acceleration.z    = state->ACCEL_Z_RIGHT;
	sensorStateRight.angularVelocity.x = state->GYRO_X_RIGHT;
	sensorStateRight.angularVelocity.y = state->GYRO_Y_RIGHT;
	sensorStateRight.angularVelocity.z = state->GYRO_Z_RIGHT;
	sensorStateRight.angle.x           = state->ANGLE_X_RIGHT;
	sensorStateRight.angle.y           = state->ANGLE_Y_RIGHT;
	sensorStateRight.angle.z           = state->ANGLE_Z_RIGHT;

	// Set to correct player
	size_t offset = sizeof(nn::hid::SixAxisSensorState) * controller;
	setMemoryType(saltynxsixAxisStateLeftJoycon + offset, sensorStateLeft);
	setMemoryType(saltynxsixAxisStateRightJoycon + offset, sensorStateRight);
}

void MainLoop::getTouchState(TouchAndKeyboardData* state) {
	nn::hid::TouchScreenState16Touch touchSensorState = getMemoryType<nn::hid::TouchScreenState16Touch>(saltynxtouchScreenStateBacklog);

	// Can't go over 2
	if(touchSensorState.count > 2)
		touchSensorState.count = 2;

	state->touchX1         = touchSensorState.touches[0].x;
	state->touchY1         = touchSensorState.touches[0].y;
	state->touchX2         = touchSensorState.touches[1].x;
	state->touchY2         = touchSensorState.touches[1].y;
	state->numberOfTouches = touchSensorState.count;
}

void MainLoop::setTouchState(TouchAndKeyboardData* state) {
	nn::hid::TouchScreenState16Touch touchSensorState;

	touchSensorState.touches[0].x = state->touchX1;
	touchSensorState.touches[0].y = state->touchY1;
	touchSensorState.touches[1].x = state->touchX2;
	touchSensorState.touches[1].y = state->touchY2;
	touchSensorState.count        = state->numberOfTouches;

	setMemoryType(saltynxtouchscreenState, touchSensorState);
}

void MainLoop::getKeyboardMouseState(TouchAndKeyboardData* state) {
	nn::hid::KeyboardState keyboardSensorState = getMemoryType<nn::hid::KeyboardState>(saltynxkeyboardStateBacklog);
	nn::hid::MouseState mouseSensorState       = getMemoryType<nn::hid::MouseState>(saltynxmouseStateBacklog);

	memcpy(state->keyboardKeys, keyboardSensorState.keys, sizeof(state->keyboardKeys));
	state->keyboardModifiers = keyboardSensorState.modifiers;
	state->mouseX            = mouseSensorState.x;
	state->mouseY            = mouseSensorState.y;
	state->mouseVelocityX    = mouseSensorState.velocityX;
	state->mouseVelocityY    = mouseSensorState.velocityY;
	state->scrollVelocityX   = mouseSensorState.scrollVelocityX;
	state->scrollVelocityY   = mouseSensorState.scrollVelocityY;
	state->mouseButtons      = mouseSensorState.buttons;
}

void MainLoop::setKeyboardMouseState(TouchAndKeyboardData* state) {
	nn::hid::KeyboardState keyboardSensorState;
	nn::hid::MouseState mouseSensorState;

	memcpy(keyboardSensorState.keys, state->keyboardKeys, sizeof(state->keyboardKeys));
	keyboardSensorState.modifiers    = state->keyboardModifiers;
	mouseSensorState.x               = state->mouseX;
	mouseSensorState.y               = state->mouseY;
	mouseSensorState.velocityX       = state->mouseVelocityX;
	mouseSensorState.velocityY       = state->mouseVelocityY;
	mouseSensorState.scrollVelocityX = state->scrollVelocityX;
	mouseSensorState.scrollVelocityY = state->scrollVelocityY;
	mouseSensorState.buttons         = state->mouseButtons;

	setMemoryType(saltynxkeyboardState, keyboardSensorState);
	setMemoryType(saltynxmouseState, mouseSensorState);
}

MainLoop::~MainLoop() {
#ifdef __SWITCH__
	LOGD << "Exiting app";
	rc = hiddbgReleaseHdlsWorkBuffer();
	hiddbgExit();

	viCloseDisplay(&disp);

	pscPmModuleFinalize(&sleepModule);
	pscPmModuleClose(&sleepModule);
#endif

	// Make absolutely sure the app is unpaused on close
	reset();
}
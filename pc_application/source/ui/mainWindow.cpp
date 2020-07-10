#define RAPIDJSON_HAS_STDSTRING 1

#include "mainWindow.hpp"

MainWindow::MainWindow()
	: wxFrame(NULL, wxID_ANY, "SwiTAS | Unnamed", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
	wxImage::AddHandler(new wxPNGHandler());
	wxImage::AddHandler(new wxJPEGHandler());

	// OwO what dis?
	SetDoubleBuffered(true);

	// Initialize FFMS2 because it can only happen once
	FFMS_Init(0, 0);

	// Get the main settings
	std::string settingsFilePath = HELPERS::getMainSettingsPath("switas_settings").GetFullPath().ToStdString();
	mainSettings                 = HELPERS::getSettingsFile(settingsFilePath);

	wxIcon mainIcon;
	mainIcon.LoadFile(HELPERS::resolvePath(mainSettings["programIcon"].GetString()), wxBITMAP_TYPE_PNG);
	SetIcon(mainIcon);

	// https://forums.wxwidgets.org/viewtopic.php?t=28894
	// https://cboard.cprogramming.com/cplusplus-programming/92653-starting-wxwidgets-wxpanel-full-size-frame.html
	// This means some things have to change going on
	mainSizer = new wxBoxSizer(wxHORIZONTAL);

	// Set button data instance
	buttonData = std::make_shared<ButtonData>();
	// Load button data here
	buttonData->setupButtonMapping(&mainSettings);

	// Start networking with set queues
	networkInstance = std::make_shared<CommunicateWithNetwork>(
		[](CommunicateWithNetwork* self) {
			SEND_QUEUE_DATA(SendFlag)
			SEND_QUEUE_DATA(SendFrameData)
			SEND_QUEUE_DATA(SendLogging)
			SEND_QUEUE_DATA(SendTrackMemoryRegion)
			SEND_QUEUE_DATA(SendSetNumControllers)
			SEND_QUEUE_DATA(SendAddMemoryRegion)
			SEND_QUEUE_DATA(SendStartFinalTas)
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(RecieveFlag)
			RECIEVE_QUEUE_DATA(RecieveGameInfo)
			RECIEVE_QUEUE_DATA(RecieveGameFramebuffer)
			RECIEVE_QUEUE_DATA(RecieveApplicationConnected)
			RECIEVE_QUEUE_DATA(RecieveLogging)
			RECIEVE_QUEUE_DATA(RecieveMemoryRegion)
		});

	// DataProcessing can now start with the networking instance
	dataProcessingInstance = new DataProcessing(&mainSettings, buttonData, networkInstance, this);

	projectHandler = std::make_shared<ProjectHandler>(this, dataProcessingInstance, &mainSettings);

	// UI instances
	sideUI   = std::make_shared<SideUI>(this, &mainSettings, projectHandler, mainSizer, dataProcessingInstance, networkInstance, std::bind(&MainWindow::startedIncrementFrame, this));
	bottomUI = std::make_shared<BottomUI>(this, &mainSettings, buttonData, mainSizer, dataProcessingInstance, projectHandler);

	autoFrameAdvanceTimer = new wxTimer(this);
	Bind(wxEVT_TIMER, &MainWindow::onAutoFrameAdvanceTimer, this);

	handleNetworkQueues();

	// HELPERS::addDarkmodeWindows(this);

	// Add the top menubar and the bottom statusbar
	addStatusBar();
	addMenuBar();

	// No fit for now
	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);

	Maximize(true);

	// Override the keypress handler
	// add_events(Gdk::KEY_PRESS_MASK);
	handlePreviousWindowTransform();
}

void MainWindow::onStart() {
	logWindow = new wxLogWindow(this, "Logger", false, false);
	wxLog::SetTimestamp(wxS("%Y-%m-%d %H:%M: %S"));
	wxLog::SetActiveTarget(logWindow);

	debugWindow = new DebugWindow(this, networkInstance);

	ProjectHandlerWindow projectHandlerWindow(this, projectHandler, &mainSettings);

	projectHandlerWindow.ShowModal();

	if(projectHandlerWindow.wasDialogClosedForcefully()) {
		// It was closed with X, terminate this window, and the entire application, as well
		projectHandler->setProjectWasLoaded(false);
		Close(true);
		return;
	}

	if(!projectHandlerWindow.wasProjectChosen()) {
		// Generate a temp one
		projectHandlerWindow.createTempProjectDir();
	}

	dataProcessingInstance->setProjectStart(projectHandler->getProjectStart());

	// Ask for internet connection to get started
	askForIP();
	// Then, create the savestate if its a new project
	if(projectHandlerWindow.loadedNewProject()) {
		sideUI->createSavestateHook();
	} else {
		// Just so image is displayed
		bottomUI->refreshDataViews(true);
	}
}

// clang-format off
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_CHAR_HOOK(MainWindow::keyDownHandler)
	EVT_SIZE(MainWindow::OnSize)
	EVT_IDLE(MainWindow::onIdle)
	EVT_CLOSE(MainWindow::onClose)
END_EVENT_TABLE()
// clang-format on

// Override default signal handler:
void MainWindow::keyDownHandler(wxKeyEvent& event) {
	// Only handle keybard input if control is not held down
	if(event.ControlDown() || !dataProcessingInstance->handleKeyboardInput(event.GetUnicodeKey())) {
		event.Skip();
	}
}

void MainWindow::onAutoFrameAdvanceTimer(wxTimerEvent& event) {
	sideUI->sendAutoRunData();
}

void MainWindow::handlePreviousWindowTransform() {
	// Resize and maximize as needed
	// TODO
}

void MainWindow::OnSize(wxSizeEvent& event) {
	// https://wiki.wxwidgets.org/WxSizer#Sizer_Doesn.27t_Work_When_Making_a_Custom_Control.2FWindow_.28no_autolayout.29
	// https://forums.wxwidgets.org/viewtopic.php?t=28894
	if(GetAutoLayout()) {
		Layout();
	}
}

void MainWindow::onIdle(wxIdleEvent& event) {
	if(IsShown()) {
		// Listen to joystick
		bottomUI->listenToJoystick();
	}

	sideUI->onIdle(event);

	// This handles callbacks for all different classes
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveFlag)
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveGameInfo)
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveGameFramebuffer)
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveApplicationConnected)
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveLogging)
	PROCESS_NETWORK_CALLBACKS(networkInstance, RecieveMemoryRegion)
}

void MainWindow::handleNetworkQueues() {
	// clang-format off
	ADD_NETWORK_CALLBACK(RecieveApplicationConnected, {
		wxLogMessage("Game opened");
	})
	ADD_NETWORK_CALLBACK(RecieveLogging, {
		wxLogMessage(wxString("SWITCH: " + data.log));
	})
	// clang-format on

	ADD_NETWORK_CALLBACK(RecieveGameFramebuffer, {
		uint8_t framebufferIncluded = data.buf.size() == 0 ? false : true;
		if(framebufferIncluded) {
			bottomUI->recieveGameFramebuffer(data.buf);
		}
		if(data.fromFrameAdvance == 1) {
			sideUI->enableAdvance();
			if(framebufferIncluded) {
				wxFileName framebufferFileName = dataProcessingInstance->getFramebufferPath(data.playerIndex, data.savestateHookNum, data.branchIndex, data.frame);
				wxFile file(framebufferFileName.GetFullPath(), wxFile::write);
				file.Write(data.buf.data(), data.buf.size());
				file.Close();
			}
			if(dataProcessingInstance->getNumOfFramesInSavestateHook(data.savestateHookNum, data.playerIndex) == data.frame) {
				dataProcessingInstance->addFrameHere();
			}
			if(data.controllerDataIncluded) {
				dataProcessingInstance->setControllerDataForAutoRun(data.controllerData);
				dataProcessingInstance->runFrame(true, true, true);
			}
			if(sideUI->getAutoRunActive()) {
				autoFrameAdvanceTimer->StartOnce(sideUI->getAutoRunDelay());
			}
			bottomUI->refreshDataViews(true);
		}
	})

	/*
	ADD_NETWORK_CALLBACK(RecieveFlag, {
		if(data.actFlag == RecieveInfo::UNEXPECTED_CONTROLLER_SIZE) {
			// Switch is not in touch with required amount of controllers
			sideUI->handleUnexpectedControllerSize();
		}
	})
	*/

	// clang-format on
	if(networkInstance->hasOtherSideJustDisconnected()) {
		wxLogMessage("Server disconnected, required to re-enter IP");
		SetStatusText("", 0);
		// Show the dialog
		askForIP();
	}
}

void MainWindow::startedIncrementFrame() {
	// bottomUI->getFrameViewerCanvas()->Freeze();
}

void MainWindow::addMenuBar() {
	menuBar = new wxMenuBar();

	wxMenu* fileMenu = new wxMenu();

	selectIPID          = NewControlId();
	exportAsText        = NewControlId();
	importAsText        = NewControlId();
	saveProject         = NewControlId();
	setNameID           = NewControlId();
	toggleLoggingID     = NewControlId();
	toggleDebugMenuID   = NewControlId();
	openGameCorruptorID = NewControlId();
	runFinalTasID       = NewControlId();

	fileMenu->Append(saveProject, "Save Project\tCtrl+S");
	fileMenu->Append(exportAsText, "Export To Text Format\tCtrl+Alt+E");
	fileMenu->Append(importAsText, "Import From Text Format\tCtrl+Alt+I");
	fileMenu->Append(setNameID, "Set Name\tCtrl+Alt+N");
	// Also not finished
	// fileMenu->Append(runFinalTasID, "Run Final TAS\tCtrl+R");

	// Add joystick submenu
	fileMenu->AppendSubMenu(bottomUI->getJoystickMenu(), "&List Joysticks\tCtrl+G");
	fileMenu->AppendSubMenu(projectHandler->getVideoSubmenu(), "List Recent Comparison Videos\tCtrl+Alt+L");

	projectHandler->getVideoSubmenu()->Bind(wxEVT_MENU_OPEN, &MainWindow::onRecentVideosMenuOpen, this);

	fileMenu->AppendSeparator();

	fileMenu->Append(selectIPID, "Set Switch IP\tCtrl+I");
	fileMenu->Append(toggleLoggingID, "Toggle Logging\tCtrl+Shift+L");
	fileMenu->Append(toggleDebugMenuID, "Toggle Debug Menu\tCtrl+D");
	// Not finished as of now
	// fileMenu->Append(openGameCorruptorID, "Open Game Corruptor\tCtrl+B");

	menuBar->Append(fileMenu, "&File");

	Bind(wxEVT_MENU, &MainWindow::handleMenuBar, this, wxID_ANY);

	SetMenuBar(menuBar);
}

void MainWindow::addStatusBar() {
	// 1 element for now
	CreateStatusBar(1);

	SetStatusText("No Network Connected", 0);
}

void MainWindow::handleMenuBar(wxCommandEvent& commandEvent) {
	wxWindowID id = commandEvent.GetId();
	if(id >= projectHandler->videoComparisonEntriesMenuIDBase) {
		projectHandler->openUpVideoComparisonViewer(id - projectHandler->videoComparisonEntriesMenuIDBase);
	} else if(id >= bottomUI->joystickSubmenuIDBase) {
		// Send straight to bottomUI
		bottomUI->onJoystickSelect(commandEvent);
	} else {
		// No switch statements for me
		if(id == selectIPID) {
			askForIP();
		} else if(id == saveProject) {
			projectHandler->saveProject();
		} else if(id == setNameID) {
			// Name needs to be selected
			wxString projectName = wxGetTextFromUser("Please set the new name of the project", "Set name", projectHandler->getProjectName());
			if(!projectName.empty()) {
				projectHandler->setProjectName(projectName.ToStdString());
			}
		} else if(id == toggleLoggingID) {
			logWindow->Show(!logWindow->GetFrame()->IsShown());
			wxLogMessage("Toggled log window");
		} else if(id == toggleDebugMenuID) {
			debugWindow->Show(!debugWindow->IsShown());
			wxLogMessage("Toggled debug window");
		} else if(id == openGameCorruptorID) {
			Show(false);
			sideUI->untether();
			GameCorruptor gameCorruptor(this, projectHandler, networkInstance);
			gameCorruptor.ShowModal();
			Show(true);
		} else if(id == exportAsText) {
			/*
			wxFileName exportedText = projectHandler->getProjectStart();
			exportedText.SetName(wxString::Format("player_%u_exported", dataProcessingInstance->getCurrentPlayer() + 1));
			exportedText.SetExt("ssctf");
			*/

			// User sets their own name
			ScriptExporter scriptExporter(this, projectHandler, dataProcessingInstance->getExportedCurrentPlayer());
			scriptExporter.ShowModal();

		} else if(id == importAsText) {
			wxFileDialog openFileDialog(this, _("Open Script file"), "", "", "Text files (*.txt)|*.txt|nx-TAS script files (*.ssctf)|*.ssctf", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

			if(openFileDialog.ShowModal() == wxID_OK) {
				wxFileName importPath(openFileDialog.GetPath());

				dataProcessingInstance->importFromFile(importPath);
			}
		} else if(id == runFinalTasID) {
			// Open the run final TAS dialog and untether
			sideUI->untether();
			TasRunner tasRunner(this, networkInstance, &mainSettings, dataProcessingInstance);
			tasRunner.ShowModal();
		}
	}
}

bool MainWindow::askForIP() {
	if(!networkInstance->isConnected()) {
		while(true) {
			wxString ipAddress = wxGetTextFromUser("Please enter IP address of Nintendo Switch", "Server connect", wxEmptyString);
			if(!ipAddress.empty()) {
				// IP address entered
				if(networkInstance->attemptConnectionToServer(ipAddress.ToStdString())) {
					// Make sure Switch is good
					sideUI->handleUnexpectedControllerSize();
					SetStatusText(ipAddress + ":" + std::to_string(SERVER_PORT), 0);
					Refresh();
					return true;
				} else {
					wxMessageDialog addressInvalidDialog(this, wxString::Format("This IP address is invalid: %s", networkInstance->getLastErrorMessage().c_str()), "Invalid IP", wxOK | wxICON_ERROR);
					addressInvalidDialog.ShowModal();
					// Run again
					continue;
				}
			} else {
				// If the IP is unentered (cancel button), just pronounce it untethered
				return false;
			}
		}
	} else {
		wxMessageDialog addressInvalidDialog(this, "The server is already running", "Server running", wxOK);
		addressInvalidDialog.ShowModal();
	}
}

void MainWindow::onClose(wxCloseEvent& event) {
	REMOVE_NETWORK_CALLBACK(RecieveApplicationConnected)
	REMOVE_NETWORK_CALLBACK(RecieveLogging)
	REMOVE_NETWORK_CALLBACK(RecieveGameFramebuffer)
	REMOVE_NETWORK_CALLBACK(RecieveFlag)

	// Close project dialog and save
	projectHandler->saveProject();
	networkInstance->endNetwork();

	delete wxLog::SetActiveTarget(NULL);
	delete autoFrameAdvanceTimer;

	// TODO, this raises errors for some reason
	// FFMS_Deinit();

	Destroy();
}
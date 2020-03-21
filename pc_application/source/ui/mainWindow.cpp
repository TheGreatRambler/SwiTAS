#define RAPIDJSON_HAS_STDSTRING 1

#include "mainWindow.hpp"

MainWindow::MainWindow()
	: wxFrame(NULL, wxID_ANY, "NX TAS UI", wxDefaultPosition, wxDefaultSize) {
	wxImage::AddHandler(new wxPNGHandler());
	wxImage::AddHandler(new wxJPEGHandler());

	// OwO what's dis?
	SetDoubleBuffered(true);

	// Get the main settings
	getGlobalSettings(&mainSettings);

	wxIcon mainicon;
	mainicon.LoadFile(HELPERS::resolvePath(mainSettings["programIcon"].GetString()), wxBITMAP_TYPE_PNG);
	SetIcon(mainicon);

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
			SEND_QUEUE_DATA(SendRunFrame)
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(RecieveFlag)
			RECIEVE_QUEUE_DATA(RecieveGameInfo)
			RECIEVE_QUEUE_DATA(RecieveGameFramebuffer)
			RECIEVE_QUEUE_DATA(RecieveApplicationConnected)
		});

	// DataProcessing can now start with the networking instance
	dataProcessingInstance = new DataProcessing(&mainSettings, buttonData, networkInstance, this);

	// UI instances
	sideUI   = std::make_shared<SideUI>(this, &mainSettings, mainSizer, dataProcessingInstance);
	bottomUI = std::make_shared<BottomUI>(this, &mainSettings, buttonData, mainSizer, dataProcessingInstance);

	// Add the top menubar and the bottom statusbar
	addStatusBar();
	addMenuBar();

	// No fit for now
	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

#ifdef _WIN32
	// Enable dark mode, super experimential, apparently
	// needs to be applied to every window, however
	SetWindowTheme(GetHWND(), L"DarkMode_Explorer", NULL);
	Refresh();
#endif

	// Override the keypress handler
	// add_events(Gdk::KEY_PRESS_MASK);
	handlePreviousWindowTransform();

	projectHandler = new ProjectHandler(dataProcessingInstance, &mainSettings);
}

void MainWindow::onStart() {
	// Now, open the choose project dialog
	// TODO open this in wxApp because it apparently doesn't work in the constructor
	projectHandler->ShowModal();
	if(!projectHandler->wasProjectChosen()) {
		// Generate a temp one
		projectHandler->createTempProjectDir();
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
	if(!event.ControlDown()) {
		dataProcessingInstance->handleKeyboardInput(event.GetUnicodeKey());
	}

	// Always skip the event
	event.Skip();
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
		// Read queues from the network and do things, TODO
		if(networkInstance->isConnected()) {
			/*
			CHECK_QUEUE(networkInstance, IsPaused, {
				if(data.isPaused) {
					// cool
				}
			})
			*/
		}

		// Listen to joystick
		bottomUI->listenToJoystick();
	}
}

void MainWindow::getGlobalSettings(rapidjson::Document* d) {
	std::ifstream settingsFile("../mainSettings.json");
	std::string content((std::istreambuf_iterator<char>(settingsFile)), (std::istreambuf_iterator<char>()));
	// Allow comments in JSON
	d->Parse<rapidjson::kParseCommentsFlag>(content.c_str());
}

void MainWindow::addMenuBar() {
	menuBar = new wxMenuBar();

	wxMenu* fileMenu = new wxMenu();

	selectIPID = NewControlId();
	fileMenu->Append(selectIPID, "&Server");
	// Add joystick submenu
	bottomUI->addJoystickMenu(fileMenu);

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
	if(id >= bottomUI->joystickSubmenuIDBase) {
		// Send straight to bottomUI
		bottomUI->onJoystickSelect(commandEvent);
	} else {
		if(id == selectIPID) {
			// IP needs to be selected
			wxString ipAddress = wxGetTextFromUser("Please enter IP address of Nintendo Switch", "Server connect", wxEmptyString);
			if(!ipAddress.empty()) {
				// IP address entered
				SetStatusText(ipAddress + ":" + std::to_string(SERVER_PORT), 0);
				networkInstance->attemptConnectionToServer(ipAddress.ToStdString());
			}
		}
	}
}

void MainWindow::onClose(wxCloseEvent& event) {
	// Close project dialog and save
	projectHandler->saveProject();
	projectHandler->Destroy();
	Destroy();
}
#define RAPIDJSON_HAS_STDSTRING 1

#include "mainWindow.hpp"

MainWindow::MainWindow()
	: wxFrame(NULL, wxID_ANY, "NX TAS UI", wxDefaultPosition, wxDefaultSize) {
	wxImage::AddHandler(new wxPNGHandler());

	// Get the main settings
	getGlobalSettings(&mainSettings);

	wxIcon mainicon;
	mainicon.LoadFile(HELPERS::resolvePath(mainSettings["programIcon"].GetString()), wxBITMAP_TYPE_PNG);
	SetIcon(mainicon);

	mainSizer = std::make_shared<wxFlexGridSizer>(4, 4, 3, 3);

	// Set button data instance
	buttonData = std::make_shared<ButtonData>();
	// Load button data here
	buttonData->setupButtonMapping(&mainSettings);

	dataProcessingInstance = std::make_shared<DataProcessing>(&mainSettings, buttonData, this);

	// UI instances
	sideUI   = std::make_shared<SideUI>(this, &mainSettings, mainSizer.get(), dataProcessingInstance);
	bottomUI = std::make_shared<BottomUI>(this, buttonData, mainSizer.get(), dataProcessingInstance);

	// Add the top menubar
	addMenuBar();

	SetMinSize(wxSize(500, 400));
	Center();

	SetSizerAndFit(mainSizer.get());

	// Override the keypress handler
	// add_events(Gdk::KEY_PRESS_MASK);
	handlePreviousWindowTransform();
}

// clang-format off
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_CHAR_HOOK(MainWindow::keyDownHandler)
	EVT_SIZE(MainWindow::OnSize)
END_EVENT_TABLE()
// clang-format on

// Override default signal handler:
void MainWindow::keyDownHandler(wxKeyEvent& event) {
	dataProcessingInstance->handleKeyboardInput(event.GetUnicodeKey());
}

void MainWindow::handlePreviousWindowTransform() {
	// Resize and maximize as needed
	// TODO
}

void MainWindow::OnSize(wxSizeEvent& event) {
	// https://wiki.wxwidgets.org/WxSizer#Sizer_Doesn.27t_Work_When_Making_a_Custom_Control.2FWindow_.28no_autolayout.29
	if(GetAutoLayout()) {
		Layout();
	}
}

void MainWindow::getGlobalSettings(rapidjson::Document* d) {
	std::ifstream settingsFile("../mainSettings.json");
	std::string content((std::istreambuf_iterator<char>(settingsFile)), (std::istreambuf_iterator<char>()));
	// Allow comments in JSON
	d->Parse<rapidjson::kParseCommentsFlag>(content.c_str());
}

void MainWindow::addMenuBar() {
	// https://www.lucidarme.me/gtkmm-example-13/
	// Add menubar to layout
	// mainLayout.pack_start(menuBar, Gtk::PACK_SHRINK);
}

MainWindow::~MainWindow() {}
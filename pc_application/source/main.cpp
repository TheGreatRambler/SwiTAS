#include <atomic>
#include <memory>
#include <thread>
#include <wx/wx.h>

#include "dataHandling/interfaceWithSwitch.hpp"
#include "ui/mainWindow.hpp"

class MainApp : public wxApp {
private:
	MainWindow window;

	std::shared_ptr<std::thread> networkThread;

	std::shared_ptr<CommunicateWithSwitch> networkInstance;

	// For the networking code to check if it should end
	std::atomic_bool networkRun;

	void runNetwork() {
		// Simply run network handling
		if(networkRun) {
			// Ask for data
			networkInstance->listenForSwitchCommands();
		}
	}

public:
	// This needs to run before the window is created
	MainApp() {
		// Load PNGs
		wxImage::AddHandler(new wxPNGHandler());
	}

	virtual bool OnInit() {
		window.Show(true);
		SetTopWindow(&window);

		// Create network thread
		networkThread = std::make_shared<std::thread>(&MainApp::runNetwork, this);

		networkInstance = std::make_shared<CommunicateWithSwitch>();

		networkRun = true;

		return true;
	}

	virtual int OnExit() {
		// Wait for the network thread to end
		networkRun = false;
		networkThread->join();
		// No errors, return 0
		return 0;
	}
};

wxIMPLEMENT_APP(MainApp);
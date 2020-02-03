#include <atomic>
#include <cstdio>
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
			// For now, no
			// networkInstance->listenForSwitchCommands();
		}
	}

public:
	virtual bool OnInit() {
		puts("App starting");
		window.Show(true);
		SetTopWindow(&window);
		puts("Window opened");
		// Create network thread
		networkThread = std::make_shared<std::thread>(&MainApp::runNetwork, this);
		puts("Network thread opened");

		networkInstance = std::make_shared<CommunicateWithSwitch>();

		networkRun = true;

		return true;
	}

	int OnExit() {
		// Wait for the network thread to end
		networkRun = false;
		networkThread->join();
		// No errors, return 0
		return 0;
	}
};

IMPLEMENT_APP(MainApp);
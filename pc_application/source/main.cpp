#include <atomic>
#include <cstdio>
#include <memory>
#include <thread>
#include <wx/wx.h>

#include "ui/mainWindow.hpp"

class MainApp : public wxApp {
private:
	MainWindow window;

public:
	virtual bool OnInit() {
		puts("App starting");
		window.Show(true);
		SetTopWindow(&window);
		puts("Window opened");

		return true;
	}

	int OnExit() {
		// Wait for the network thread to end
		window.endNetworking();
		// No errors, return 0
		return 0;
	}

	/*void OnIdle(wxIdleEvent& event) {
		// https://forums.wxwidgets.org/viewtopic.php?t=20610
		window.onIdleLoop();

		if(IsMainLoopRunning()) {
			event.RequestMore(true);
		}
	} */

	// DECLARE_EVENT_TABLE()
};

// BEGIN_EVENT_TABLE(MainApp, wxApp)
// EVT_IDLE(MainApp::OnIdle)
// END_EVENT_TABLE()

IMPLEMENT_APP(MainApp);
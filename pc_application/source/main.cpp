#include <atomic>
#include <cstdio>
#include <memory>
#include <thread>
#include <wx/wx.h>

#include "ui/mainWindow.hpp"

class MainApp : public wxApp {
private:
	MainWindow* window;

public:
	virtual bool OnInit() {
		window = new MainWindow();

		puts("App starting");
		window->Show(true);
		puts("Window opened");
		window->onStart();
		return true;
	}

	int OnExit() {
		// No errors, return 0
		return 0;
	}
};

IMPLEMENT_APP(MainApp);
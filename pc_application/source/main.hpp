#include <glibmm/refptr.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>

#include "mainWindow.hpp"

class MainApplication {
private:
	Glib::RefPtr<Gtk::Application> app;
	Gtk::Window mainWindow;

public:
	MainApplication(int argc, char* argv[]) {
		app = Gtk::Application::create(argc, argv, "org.tgr.nxtas");
	}

	int run() {
		// Run now
		return app->run(mainWindow);
	}
};
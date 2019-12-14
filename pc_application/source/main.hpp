#include <glibmm/refptr.h>
#include <gtkmm.h>

#include "mainWindow.hpp"

class MainApplication {
private:
	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::Window> mainWindow;

public:
	MainApplication (int argc, char* argv[]) {
		app        = Gtk::Application::create (argc, argv, "org.tgr.nxtas");
		mainWindow = new MainWindow ();
	}

	int run () {
		// Run now
		return app->run (mainWindow);
	}
};
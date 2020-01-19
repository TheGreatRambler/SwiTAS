#include "wx/wx.h"

#include "mainWindow.hpp"

// Note to self for later: https://stackoverflow.com/questions/49092784/how-to-distribute-a-gtk-application-on-windows
// https://www.lucidarme.me/gtkmm/
/*
int main(int argc, char* argv[]) {
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.tgr.nxtas");
	MainWindow mainWindow;

	// Finish
	return app->run(mainWindow);
}
*/

IMPLEMENT_APP(MainApp)

class MainApp : public wxApp {
private:
	MainWindow window;

public:
	virtual bool OnInit() {
		window.Show(true);
		SetTopWindow(&window);
		return true;
	}
};
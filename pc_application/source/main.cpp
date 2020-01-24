#include "wx/wx.h"

#include "ui/mainWindow.hpp"

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

wxIMPLEMENT_APP(MainApp)
#include <gtkmm.h>
#include <glibmm/refptr.h>

#include "mainWindow.hpp"

class MainApplication {
    private:
    Glib::RefPtr<Gtk::Application> app;
    Gtk::Window* mainWindow;

    public:
    MainApplication(int argc, char* argv[]) {
        app = Gtk::Application::create(argc, argv, "org.tgr.nxtas");
        mainWindow = new MainWindow();
    }

    int run() {
        // Run now
        return app->run(mainWindow);
    }
}
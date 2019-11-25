#include <gtkmm.h> 

class MainApplication {
    private:
    Gtk::Main app;

    public:
    MainApplication(int argc, char* argv[]) {
        app = Gtk::Main(argc, argv);
    }
}
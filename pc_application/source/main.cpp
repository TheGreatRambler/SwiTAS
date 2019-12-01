#include <gtkmm.h> 

#include "main.hpp"

// Note to self for later: https://stackoverflow.com/questions/49092784/how-to-distribute-a-gtk-application-on-windows
// https://www.lucidarme.me/gtkmm/
int main(int argc, char* argv[]) {
    // Initialize gtkmm
    MainApplication* main = new MainApplication(argc, argv);
    
    // Finish
    return main->run();
}

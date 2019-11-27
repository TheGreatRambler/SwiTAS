#include <gtkmm.h> 

#include "main.hpp"

// https://www.lucidarme.me/gtkmm/
int main(int argc, char* argv[]) {
    // Initialize gtkmm
    MainApplication* main = new MainApplication(argc, argv);
    
    // Finish
    return main->run();
}
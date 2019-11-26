#pragma once

#include <gtkmm.h>

class MainWindow : public Gtk::Window {
    private:
    // Layout that contains all items, including the menubar
    Gtk::VBox mainLayout;
    // Grid where all the layout stuff is dealt with
    Gtk::Grid mainGrid;
    // Grid containing the left hand side widgets
    Gtk::Grid leftGrid;
    // Grid containing the bottom widgets
    Gtk::Grid bottomGrid;
    // Menu bar containing "open file", etc...
    Gtk::MenuBar menuBar;

    // Left UI
    // Button group for play, pause, etc...
    Gtk::ButtonBox playButtons;
    // Buttons themselves
    Gtk::Button playPauseButton;
    Gtk::Button frameAdvanceButton;

    
    public:
    MainWindow() {
        // Add mainLayout to window
        add(mainLayout);
        // Add the top menubar
        addMenuBar();
        // Add left UI
        addLeftUI();
        // Add bottom UI
        addBottomUI();
        // Adding all the grids
        addGrids();
    };
	
    void addGrids() {
        // Add left grid spanning the whole left side
        mainGrid.attach(leftGrid, 0, 0, 1, 2);
        // Add bottom grid spanning the rest of the bottom
        mainGrid.attach(bottomGrid, 1, 1, 1, 1);
        // Shows all the items in this container
        mainGrid.show_all();
        // Add all of them to the current layout
        mainLayout.pack_start(mainGrid, Gtk::PACK_SHRINK);
    }

    void addMenuBar() {
        // https://www.lucidarme.me/gtkmm-example-13/
        // Add menubar to layout
        mainLayout.pack_start(menuBar, Gtk::PACK_SHRINK);
    }

    void addLeftUI() {
        // Some buttons for the left UI
        // https://developer.gnome.org/gtkmm-tutorial/stable/sec-multi-item-containers.html.en#buttonbox-example
        // https://www.lucidarme.me/gtkmm-example-2/
        playButtons.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        playButtons.set_layout(Gtk::BUTTONBOX_SPREAD);

        // Set images for buttons (instead of add_label)
        playPauseButton.set_image_from_icon_name("pauseButton");
        frameAdvanceButton.set_image_from_icon_name("frameAdvanceButton");

        playButtons.add(playPauseButton);
        playButtons.add(frameAdvanceButton);
    }

    void addBottomUI() {

    }

    ~mainwindow() {};
}
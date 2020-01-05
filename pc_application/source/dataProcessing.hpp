#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treepath.h>
#include <gtkmm/treeview.h>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "buttonData.hpp"

class InputColumns : public Gtk::TreeModelColumnRecord {
public:
	Gtk::TreeModelColumn<uint32_t> frameNum;
	// All the buttons are stored inside of buttonMapping
	// https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
	std::map<Btn, Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>*> buttonPixbufs;

	InputColumns();
};

class DataProcessing {
private:
	// Vector storing ALL inputs
	// Shared pointer so everything is nice
	std::vector<std::shared_ptr<ControllerData>> inputsList;
	// Current input
	std::shared_ptr<ControllerData> currentData;
	// Button data
	ButtonData* buttonData;
	// Current frame
	uint32_t currentFrame;
	// Tree data storing the controller stuffs
	Glib::RefPtr<Gtk::ListStore> controllerListStore;
	// Stores the columns for the above list store
	InputColumns inputColumns;
	// Tree view viewed in the UI
	Gtk::TreeView treeView;
	// Scrollable data window
	Gtk::ScrolledWindow scrolledWindow;
	// Current path and current iterator to save on CPU
	Gtk::TreePath currentPath;
	// Using callbacks for inputs
	std::function<void(Btn, bool)> inputCallback;

	void getCurrentIndex();

public:
	DataProcessing(ButtonData* buttons);

	void setInputCallback(std::function<void(Btn, bool)> callback);

	bool getButtonState(Btn button);

	void setButtonState(Btn button, bool state);

	void toggleButtonState(Btn button);

	void setCurrentFrame(uint32_t frameNum);

	void addNewFrame(bool isFirstFrame = false);

	void handleKeyboardInput(guint key);

	Gtk::ScrolledWindow* getWindow();
};

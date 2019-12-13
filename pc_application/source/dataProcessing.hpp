#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
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

#include "bottomUI.hpp"
#include "buttonData.hpp"

class InputColumns : public Gtk::TreeModelColumnRecord {
public:
	Gtk::TreeModelColumn<uint32_t> frameNum;
	// All the buttons are stored inside of buttonMapping
	// https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
	std::map<Btn, Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>*> buttonPixbufs;

	InputColumns () {
		add (frameNum);
		// Loop through the buttons and add them
		for (auto const& button : buttonMapping) {
			// Gets pointer from tuple
			Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* thisIcon = button.second->columnIcon;
			// Add to map for later
			buttonPixbufs[button.first] = thisIcon;
			// Add to the columns themselves (gives value, not pointer)
			add (*thisIcon);
		}
	}
}

class DataProcessing {
private:
	// Vector storing ALL inputs
	// Shared pointer so everything is nice
	std::vector<std::shared_ptr<ControllerData>> inputsList;
	// Current input
	std::shared_ptr<ControllerData> currentData;
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
	// Has to be a pointer, sadly
	Gtk::TreeModel::iterator* currentIterator;
	// The instance of bottomUI, this class has a reference to it
	// This is so it can make bottomUI aware of various changes
	BottomUI* bottomUI;

	void getCurrentIndex () {
		// Delete the first iterator now because it will be replaced
		delete currentIterator;
		// returns a treepath to the current index
		// Also returns an iterator because apparently that's needed
		// Clear the variable so it can be reassigned
		currentPath.clear ();
		// Add the current frame
		currentPath.push_back (currentFrame);
		// Simply returns the iterator
		currentIterator controllerListStore->get_iter (*path);
	}

public:
	DataProcessing () {
		// Add the list store from the columns
		controllerListStore = Gtk::ListStore::create (inputColumns);
		// Set this tree view to this model
		treeView.set_model (controllerListStore);
		// Add all the columns, this somehow wasn't done already
		treeView.append_column ("Frame", inputColumns.frameNum);
		// Loop through buttons and add all of them
		for (auto const& thisButton : buttonMapping) {
			// Append with the string specified by Button Mapping
			// Get value of columnIcon, not pointer
			treeView.append_column (thisButton.second->viewName, *thisButton.second->columnIcon);
		}
		// Once all columns are added, do some stuff on them
		for (auto& column : treeView.get_columns ()) {
			// Set to fixed size mode to speed things up
			column->set_sizing (Gtk::TREE_VIEW_COLUMN_FIXED);
		}
		treeView.set_fixed_height_mode (true);
		// Add the treeview to the scrolled window
		scrolledWindow.add (treeView);
		// Only show the scrollbars when they are necessary:
		scrolledWindow.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		// Add this first frame
		addNewFrame (true);
	}

	void setBottomUI (BottomUI* bUI) {
		bottomUI = bUI;
	}

	bool getButtonState (Btn button) {
		// Get value from the bitset
		return currentData->buttons.test (button);
	}

	void setButtonState (Btn button, bool state) {
		// If state is true, on, else off
		currentData->buttons.set (button, state);
		// Get the index of the treeview
		// The state is being changed, so so does the UI
		// Two pointers have to be deconstructed
		Gtk::TreeModel::Row row = *(*currentIterator);
		// Set the image based on the state
		row[*inputColumns.buttonPixbufs[button]] = state ? buttonMapping[button]->onIcon : buttonMapping[button]->offIcon;
		// Send the update signal
		controllerListStore->row_changed (currentPath, *currentIterator);
		// Focus to this specific row
		treeView.scroll_to_row (currentPath);
		// BottomUI needs to know the state changed, even if it
		// Was the one to make us aware in the first place
		bottomUI->setIconState (button, state);
	}

	void toggleButtonState (Btn button) {
		// Send the `not` of the current state
		setButtonState (button, !getButtonState (button));
	}

	void setCurrentFrame (uint32_t frameNum) {
		// Must be a frame that has already been written, else, raise error
		if (frameNum < inputsList.size () && frameNum > -1) {
			// Set the current frame to this frame
			// Shared pointer so this can be done
			currentData = inputsList[frameNum];
			// Set the current frame to this number
			currentFrame = frameNum;
			// Kinda a misnomer, but this will set the current
			// index for those functions That need it
			getCurrentIndex ();
			// Focus to this specific row now
			treeView.scroll_to_row (currentPath);
		}
	}

	void addNewFrame (bool isFirstFrame = false) {
		if (!isFirstFrame) {
			// On the first frame, it is already set right
			// This will automatically add at the end even when the
			// Current frame is not the most recent
			currentFrame = inputsList.size ();
		}
		// Will overwrite previous frame if need be
		currentData = std::make_shared<ControllerData> ();
		// Add this to the vector
		inputsList.push_back (currentData);
		// Add to the table
		Gtk::TreeModel::Row row    = *(controllerListStore->append ());
		row[inputColumns.frameNum] = currentFrame;
		for (auto const& pixbufData : inputColumns.buttonPixbufs) {
			// Turn all buttons automatically to off
			// Dereference pointer to get to it
			row[*pixbufData.second] = buttonMapping[pixbufData.first].offIcon;
		}
		// Now, set the index to here so that some stuff can be ran
		setCurrentFrame (currentFrame);
	}

	void handleKeyboardInput (guint key) {
		for (auto const& thisButton : buttonMapping) {
			// See if it corresponds to the toggle keybind
			// TODO handle set and clear commands (shift and ctrl)
			if (key == thisButton.second->toggleKeybind) {
				// Toggle this key and end the loop
				toggleButtonState (thisButton.first);
				break;
			}
		}
	}

	Gtk::ScrolledWindow* getWindow () {
		// Get pointer of the scrolled window (not the treeview)
		// The value should be saftely dereferenced once the
		// Class dies, but I dunno
		return &scrolledWindow;
	}

	~DataProcessing () {
		delete currentIterator;
	}
}

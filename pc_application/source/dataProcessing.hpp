#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <gtkmm.h>
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
	// Rows displayed in the treeview
	std::vector<Gtk::TreeModel::Row> treeviewRows;
	// Tree view viewed in the UI
	Gtk::TreeView treeView;

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
			treeView.append_column (thisButton.second->viewName,
				*thisButton.second->columnIcon);
		}
		// Once all columns are added, do some stuff on them
		for (auto& column : treeView.get_columns ()) {
			// Set to fixed size mode to speed things up
			column->set_sizing (Gtk::TREE_VIEW_COLUMN_FIXED);
		}
		treeView.set_fixed_height_mode (true);
		// Add this first frame
		addNewFrame (true);
	}

	bool getButtonState (Btn button) {
		// Get value from the bitset
		return currentData->buttons.test (button);
	}

	void setButtonState (Btn button, bool state) {
		// If state is true, on, else off
		return currentData->buttons.set (button, state);
	}

	void setCurrentFrame (uint32_t frameNum) {
		// Must be a frame that has already been written, else, raise error
		if (frameNum < inputsList.size () && frameNum > -1) {
			// Set the current frame to this frame
			// Shared pointer so this can be done
			currentData = inputsList[frameNum];
			// Set the current frame to this number
			currentFrame = frameNum;
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
		treeviewRows.push_back (row);
	}

	Gtk::TreeView* getTreeview () {
		// Get pointer of the treeview
		// The value should be saftely dereferenced once the
		// Class dies, but I dunno
		return &treeView;
	}

<<<<<<< HEAD
	~DataProcessing () { }
}
=======
	~DataProcessing () { }
}
>>>>>>> 544fbe60f1eb621084e0084903e972c9476b38e3

#include "dataProcessing.hpp"
#include <bits/stdint-uintn.h>

InputColumns::InputColumns() {
	add(frameNum);
}

DataProcessing::DataProcessing(std::shared_ptr<ButtonData> buttons, wxWindow* parent) {
	// Inherit from list control
	wxGenericListCtrl(parent, -1, "", wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES);
	buttonData = buttons;
	// scrolledWindow = std::make_shared<Gtk::ScrolledWindow>();
	// This is cool, so set it
	EnableAlternateRowColours(true);
	imageList.Create(50, 50);
	// Add the list store from the columns
	// controllerListStore = Gtk::ListStore::create(inputColumns);
	// Set this tree view to this model
	SetImageList(&imageList, wxIMAGE_LIST_NORMAL);
	// treeView.set_model(controllerListStore);
	// Add all the columns, this somehow wasn't done already
	// treeView.append_column("Frame", inputColumns.frameNum);
	AppendColumn("Frame");
	// Disable searching because it breaks stuff
	// treeView.set_enable_search(false);
	// Loop through buttons and add all of them
	// Set this now that it is recieved
	// Loop through the buttons and add them
	for(auto const& button : buttonData->buttonMapping) {
		// Gets pointer from tuple
		// This better not be deleted when it goes out of scope
		// Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thisIcon;
		// Add to map for later
		// inputColumns.buttonPixbufs[button.first] = thisIcon;
		// Append now
		AppendColumn(button.second.scriptName);
		// Have to create a bitmap manually
		// Bitmaps are interleaved between on and off
		imageList.Add(button.offBitmapIcon.get());
		imageList.Add(button.onBitmapIcon.get());
		// treeView.append_column(button.second.scriptName, thisIcon);
		// Add to the columns themselves (gives value, not pointer)
		// inputColumns.add(thisIcon);
	}
	// Once all columns are added, do some stuff on them
	// for(auto& column : treeView.get_columns()) {
	// Set to fixed size mode to speed things up
	// column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
	//}
	// treeView.set_fixed_height_mode(true);
	// Add the treeview to the scrolled window
	// scrolledWindow->add(treeView);
	// Only show the scrollbars when they are necessary:
	// scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	// Add this first frame
	// addNewFrame();
}

int OnGetItemColumnImage(long row, long column) const {
	if(column == 0) {
		// This is the frame
		return -1;
	} else {
		// Returns index in the imagelist
		// Need to account for the frame being first
		uint8_t button = column - 1;
		uint8_t on     = inputsList[row]->buttons[button];
		if(on) {
			// Return index of on image
			// Interleaved means it looks like this
			return button * 2 + 1;
		} else {
			return button * 2;
		}
	}
}

wxString OnGetItemText(long row, long column) const {
	// Returns when text is needed
	if(column == 0) {
		// This is the frame, which is just the row number
		return std::to_string(row);
	}
	// This function shouldn't recieve any other column
}

bool DataProcessing::getButtonState(Btn button) {
	// Get value from the bitset
	return currentData->buttons[button];
}

void DataProcessing::setButtonState(Btn button, bool state) {
	// If state is true, on, else off
	currentData->buttons[button] = state;
	// Because of virtual, just trigger an update of the row
	RefreshItem(currentFrame);
	// Get the index of the treeview
	// The state is being changed, so so does the UI
	// Two pointers have to be deconstructed
	// Gtk::TreeModel::Row row = getRowAtIndex(currentFrame);
	// Set the image based on the state
	// row[inputColumns.buttonPixbufs[button]] = state ? buttonData->buttonMapping[button].onIcon : buttonData->buttonMapping[button].offIcon;
	// Send the update signal TODO
	// controllerListStore->row_changed(currentPath, row);
	// Focus to this specific row
	// treeView.scroll_to_row(currentPath);
	// BottomUI needs to know the state changed, even if it
	// Was the one to make us aware in the first place
	// inputCallback(button, state);
}

void DataProcessing::toggleButtonState(Btn button) {
	// Send the `not` of the current state
	setButtonState(button, !getButtonState(button));
}

void DataProcessing::setCurrentFrame(uint32_t frameNum) {
	// Must be a frame that has already been written, else, raise error
	if(frameNum < inputsList.size()) {
		// Set the current frame to this frame
		// Shared pointer so this can be done
		currentData = inputsList[frameNum];
		// Set the current frame to this number
		currentFrame = frameNum;
		// Focus to this specific row now
		// This essentially scrolls to it
		EnsureVisible(frameNum);
	}
}

void DataProcessing::addNewFrame() {
	// On the first frame, it is already set right (because the size is zero)
	// This will automatically add at the end even when the
	// Current frame is not the most recent
	currentFrame = inputsList.size();
	// Will overwrite previous frame if need be
	currentData = std::make_shared<ControllerData>();
	// Add this to the vector
	inputsList.push_back(currentData);
	// Add to the table
	// Gtk::TreeModel::Row row    = *(controllerListStore->append());
	// row[inputColumns.frameNum] = currentFrame;
	// for(auto const& pixbufData : inputColumns.buttonPixbufs) {
	// Turn all buttons automatically to off
	// Dereference pointer to get to it
	// Set all to off
	// setButtonState(pixbufData.first, false);
	//}
	// Because of the usuability of virtual list controls, just update the length
	SetItemCount(inputsList.size());
	// Now, set the index to here so that some stuff can be ran
	setCurrentFrame(currentFrame);
}

void DataProcessing::handleKeyboardInput(wxChar key) {
	for(auto const& thisButton : buttonData->buttonMapping) {
		// See if it corresponds to the toggle keybind
		// TODO handle set and clear commands (shift and ctrl)
		if(key == thisButton.second.toggleKeybind) {
			// Toggle this key and end the loop
			toggleButtonState(thisButton.first);
			break;
		}
	}
}

// std::shared_ptr<wxGenericListCtrl> DataProcessing::getWidget() {
//	return controllerListStore;
//}
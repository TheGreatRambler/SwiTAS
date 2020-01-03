#include "dataProcessing.hpp"

InputColumns::InputColumns() {
	add(frameNum);
	// Loop through the buttons and add them
	for(auto const& button : buttonMapping) {
		// Gets pointer from tuple
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* thisIcon = new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>();
		// Add to map for later
		buttonPixbufs[button.first] = thisIcon;
		// Add to the columns themselves (gives value, not pointer)
		add(*thisIcon);
	}
}

void DataProcessing::getCurrentIndex() {
	// returns a treepath to the current index
	// Also returns an iterator because apparently that's needed
	// Clear the variable so it can be reassigned
	currentPath.clear();
	// Add the current frame
	currentPath.push_back(currentFrame);
}

DataProcessing::DataProcessing() {
	// Add the list store from the columns
	controllerListStore = Gtk::ListStore::create(inputColumns);
	// Set this tree view to this model
	treeView.set_model(controllerListStore);
	// Add all the columns, this somehow wasn't done already
	treeView.append_column("Frame", inputColumns.frameNum);
	// Loop through buttons and add all of them
	for(auto const& thisButton : buttonMapping) {
		// Append with the string specified by Button Mapping
		// Get value of columnIcon, not pointer
		// Default to off
		treeView.append_column(thisButton.second->viewName, *inputColumns.buttonPixbufs[thisButton.first]);
	}
	// Once all columns are added, do some stuff on them
	for(auto& column : treeView.get_columns()) {
		// Set to fixed size mode to speed things up
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
	}
	treeView.set_fixed_height_mode(true);
	// Add the treeview to the scrolled window
	scrolledWindow.add(treeView);
	// Only show the scrollbars when they are necessary:
	scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	// Add this first frame
	addNewFrame(true);
}

void DataProcessing::setInputCallback(std::function<void(Btn, bool)> callback) {
	inputCallback = callback;
}

bool DataProcessing::getButtonState(Btn button) {
	// Get value from the bitset
	return currentData->buttons.test(button);
}

void DataProcessing::setButtonState(Btn button, bool state) {
	// If state is true, on, else off
	currentData->buttons.set(button, state);
	// Get the index of the treeview
	// The state is being changed, so so does the UI
	// Two pointers have to be deconstructed
	Gtk::TreeModel::Row row = *controllerListStore->get_iter(currentPath);
	// Set the image based on the state
	row[*inputColumns.buttonPixbufs[button]] = state ? buttonMapping[button]->onIcon : buttonMapping[button]->offIcon;
	// Send the update signal
	controllerListStore->row_changed(currentPath, row);
	// Focus to this specific row
	treeView.scroll_to_row(currentPath);
	// BottomUI needs to know the state changed, even if it
	// Was the one to make us aware in the first place
	inputCallback(button, state);
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
		// Kinda a misnomer, but this will set the current
		// index for those functions That need it
		getCurrentIndex();
		// Focus to this specific row now
		treeView.scroll_to_row(currentPath);
	}
}

void DataProcessing::addNewFrame(bool isFirstFrame = false) {
	if(!isFirstFrame) {
		// On the first frame, it is already set right
		// This will automatically add at the end even when the
		// Current frame is not the most recent
		currentFrame = inputsList.size();
	}
	// Will overwrite previous frame if need be
	currentData = std::make_shared<ControllerData>();
	// Add this to the vector
	inputsList.push_back(currentData);
	// Add to the table
	Gtk::TreeModel::Row row    = *(controllerListStore->append());
	row[inputColumns.frameNum] = currentFrame;
	for(auto const& pixbufData : inputColumns.buttonPixbufs) {
		// Turn all buttons automatically to off
		// Dereference pointer to get to it
		row[*pixbufData.second] = buttonMapping[pixbufData.first]->offIcon;
	}
	// Now, set the index to here so that some stuff can be ran
	setCurrentFrame(currentFrame);
}

void DataProcessing::handleKeyboardInput(guint key) {
	for(auto const& thisButton : buttonMapping) {
		// See if it corresponds to the toggle keybind
		// TODO handle set and clear commands (shift and ctrl)
		if(key == thisButton.second->toggleKeybind) {
			// Toggle this key and end the loop
			toggleButtonState(thisButton.first);
			break;
		}
	}
}

Gtk::ScrolledWindow* DataProcessing::getWindow() {
	// Get pointer of the scrolled window (not the treeview)
	// The value should be saftely dereferenced once the
	// Class dies, but I dunno
	return &scrolledWindow;
}
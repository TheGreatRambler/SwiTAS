#include "dataProcessing.hpp"

DataProcessing::DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxWindow* parent)
	: wxListCtrl(parent, DataProcessing::LIST_CTRL_ID, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES) {
	// Inherit from list control
	// Use this specific ID in order to do things
	buttonData   = buttons;
	mainSettings = settings;
	// Set the mask color via a css string
	// https://docs.wxwidgets.org/3.0/classwx_colour.html#a08e9f56265647b8b5e1349b76eb728e3
	maskColor.Set((*mainSettings)["iconTransparent"].GetString());
	// scrolledWindow = std::make_shared<Gtk::ScrolledWindow>();
	// This is cool, so set it
	EnableAlternateRowColours(true);
	// Resize images based on the settings file
	int imageIconWidth  = (*mainSettings)["inputsList"]["imageWidth"].GetInt();
	int imageIconHeight = (*mainSettings)["inputsList"]["imageHeight"].GetInt();
	imageList.Create(imageIconWidth, imageIconHeight);
	// Add the list store from the columns
	// controllerListStore = Gtk::ListStore::create(inputColumns);
	// Set this tree view to this model

	// uint8_t i = 0;

	InsertColumn(0, "Frame", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
	// wxListItem frameInfo;
	// frameInfo.SetMask(wxLIST_MASK_TEXT);
	// frameInfo.SetColumn(i);
	// SetItem(frameInfo);
	// i++;
	// Disable searching because it breaks stuff
	// treeView.set_enable_search(false);
	// Loop through buttons and add all of them
	// Set this now that it is recieved
	// Loop through the buttons and add them
	uint8_t i = 1;
	for(auto const& button : buttonData->buttonMapping) {
		// Gets pointer from tuple
		// This better not be deleted when it goes out of scope
		// Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thisIcon;
		// Add to map for later
		// inputColumns.buttonPixbufs[button.first] = thisIcon;
		// Append now
		InsertColumn(i, button.second->normalName, wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
		i++;
		// wxListItem imageInfo;
		// imageInfo.SetMask(wxLIST_MASK_IMAGE);
		// imageInfo.SetColumn(i);
		// SetItem(imageInfo);
		// i++;
		// Have to create a bitmap manually
		// Bitmaps are interleaved between on and off
		// Have to pass raw value, not pointer
		// Have to resize also, no reference needed
		imageList.Add(*button.second->resizedListOnBitmap, maskColor);
		imageList.Add(*button.second->resizedListOffBitmap, maskColor);
		// treeView.append_column(button.second.scriptName, thisIcon);
		// Add to the columns themselves (gives value, not pointer)
		// inputColumns.add(thisIcon);
	}
	SetImageList(&imageList, wxIMAGE_LIST_SMALL);
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
	for(int i = 0; i < 30; i++) {
		addNewFrame();
	}
}

// clang-format off
BEGIN_EVENT_TABLE(DataProcessing, wxListCtrl)
	EVT_LIST_CACHE_HINT(DataProcessing::LIST_CTRL_ID, DataProcessing::onCacheHint)
END_EVENT_TABLE()
// clang-format on

void DataProcessing::setInputCallback(std::function<void(Btn, bool)> callback) {
	inputCallback = callback;
}

int DataProcessing::OnGetItemColumnImage(long row, long column) const {
	if(column == 0) {
		// This is the frame num
		return -1;
	} else {
		// Returns index in the imagelist
		// Need to account for the frame being first
		uint8_t button = column - 1;
		uint8_t on     = inputsList[row]->buttons[button];
		int res;
		if(on) {
			// Return index of on image
			// Interleaved means it looks like this
			res = button * 2;
		} else {
			// I'm trying something, don't return an image if off
			// res = button * 2 + 1;
			res = -1;
		}

		wxBitmap test = imageList.GetBitmap(res);

		return res;
	}
}

wxString DataProcessing::OnGetItemText(long row, long column) const {
	// Returns when text is needed
	if(column == 0) {
		// This is the frame, which is just the row number
		return wxString::Format(wxT("%i"), row);
	} else {
		// Fallback for every other column
		return "";
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
	// Use wxListCtrl::GetSubItemRect and wxWindow::RefreshRect instead to prevent flickering
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
		if(key == thisButton.second->toggleKeybind) {
			// Toggle this key and end the loop
			toggleButtonState(thisButton.first);
			break;
		}
	}
}

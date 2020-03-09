#include "dataProcessing.hpp"

DataProcessing::DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent)
	: wxListCtrl(parent, DataProcessing::LIST_CTRL_ID, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES) {

	// This can't handle it :(
	SetDoubleBuffered(false);
	// Inherit from list control
	// Use this specific ID in order to do things
	buttonData      = buttons;
	mainSettings    = settings;
	networkInstance = communicateWithNetwork;
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
		InsertColumn(i, button.second->normalName, wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);
		// Shouldn't be the exact width, it's a bit too small that way
		SetColumnWidth(i, (int)(imageIconWidth * 1.5f));
		buttonToColumn[button.first]               = i;
		charToButton[button.second->toggleKeybind] = button.first;

		imageList.Add(*const_cast<wxBitmap*>(button.second->resizedListOnBitmap), maskColor);
		imageList.Add(*const_cast<wxBitmap*>(button.second->resizedListOffBitmap), maskColor);

		i++;
	}
	SetImageList(&imageList, wxIMAGE_LIST_SMALL);
	// Set item attributes for nice colors
	setItemAttributes();
	// Add one frame
	for(uint8_t i = 0; i < 30; i++) {
		addNewFrame();
	}
	// Set other frames manually, without a function
	currentRunFrame   = 0;
	currentImageFrame = 0;
	// Set the current frame to the first
	setCurrentFrame(0);
}

// clang-format off
BEGIN_EVENT_TABLE(DataProcessing, wxListCtrl)
	EVT_LIST_CACHE_HINT(DataProcessing::LIST_CTRL_ID, DataProcessing::onCacheHint)
	EVT_LIST_ITEM_SELECTED(DataProcessing::LIST_CTRL_ID, DataProcessing::onSelect)
	// This is activated via double click
	EVT_LIST_ITEM_ACTIVATED(DataProcessing::LIST_CTRL_ID, DataProcessing::onActivate)
	EVT_ERASE_BACKGROUND(DataProcessing::OnEraseBackground)
END_EVENT_TABLE()
// clang-format on

void DataProcessing::setInputCallback(std::function<void(Btn, bool)> callback) {
	inputCallback = callback;
}

void DataProcessing::setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback) {
	viewableInputsCallback = callback;
}

void DataProcessing::setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback) {
	changingSelectedFrameCallback = callback;
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

wxItemAttr* DataProcessing::OnGetItemAttr(long item) const {
	uint8_t state = inputsList[item]->frameState;
	return itemAttributes.at(state);
}

void DataProcessing::setItemAttributes() {
	// Reuse both variables
	uint8_t state = 0;
	wxItemAttr* itemAttribute;

	// Default is nothing
	SET_BIT(state, false, FrameState::RAN);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["notRan"].GetString()));
	itemAttributes[state] = itemAttribute;

	SET_BIT(state, true, FrameState::RAN);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["ran"].GetString()));
	itemAttributes[state] = itemAttribute;
	SET_BIT(state, false, FrameState::RAN);

	SET_BIT(state, true, FrameState::SAVESTATE_HOOK);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["savestateHook"].GetString()));
	itemAttributes[state] = itemAttribute;
	// SavestateHook takes precedence in the case where both are present
	SET_BIT(state, true, FrameState::RAN);
	itemAttributes[state] = itemAttribute;
}

void DataProcessing::OnEraseBackground(wxEraseEvent& event) {
	// to prevent flickering, erase only content *outside* of the
	// actual list items stuff

	if(GetItemCount() > 0) {
		wxDC* dc = event.GetDC();
		assert(dc);

		// get some info
		wxCoord width = 0, height = 0;
		GetClientSize(&width, &height);

		wxCoord x, y, w, h;
		dc->SetClippingRegion(0, 0, width, height);
		dc->GetClippingBox(&x, &y, &w, &h);

		long top_item    = GetTopItem();
		long bottom_item = top_item + GetCountPerPage();
		if(bottom_item >= GetItemCount()) {
			bottom_item = GetItemCount() - 1;
		}

		// trick: we want to exclude a couple pixels
		// on the left side thus use wxLIST_RECT_LABEL
		// for the top rect and wxLIST_RECT_BOUNDS for bottom
		// rect
		wxRect top_rect, bottom_rect;
		GetItemRect(top_item, top_rect, wxLIST_RECT_LABEL);
		GetItemRect(bottom_item, bottom_rect, wxLIST_RECT_BOUNDS);

		// set the new clipping region and do erasing
		wxRect items_rect(top_rect.GetLeftTop(), bottom_rect.GetBottomRight());
		wxRegion reg(wxRegion(x, y, w, h));
		reg.Subtract(items_rect);
		dc->DestroyClippingRegion();
		dc->SetClippingRegion(reg);

		// do erasing
		dc->SetBackground(wxBrush(GetBackgroundColour(), wxSOLID));
		dc->Clear();

		// restore old clipping region
		dc->DestroyClippingRegion();
		dc->SetClippingRegion(wxRegion(x, y, w, h));
	} else {
		event.Skip();
	}
}

void DataProcessing::onSelect(wxListEvent& event) {
	// The current frame has changed
	setCurrentFrame(event.GetIndex());
}

void DataProcessing::onActivate(wxListEvent& event) {
	// Select the current image frame
	currentImageFrame = event.GetIndex();
	setCurrentFrame(event.GetIndex());
}

bool DataProcessing::getButtonState(Btn button) {
	// Get value from the bitflags
	uint8_t group;
	uint8_t position;
	if(button < 8) {
		group    = currentData->buttons[0];
		position = button;
	} else if(button < 16) {
		group    = currentData->buttons[1];
		position = button - 8;
	} else {
		// All other bits go here
		group    = currentData->buttons[2];
		position = button - 16;
	}
	return GET_BIT(group, position);
}

void DataProcessing::setButtonState(Btn button, bool state) {
	currentData->buttons[button] = state;

	if(button < 8) {
		SET_BIT(currentData->buttons[0], state, button);
	} else if(button < 16) {
		SET_BIT(currentData->buttons[1], state, button - 8);
	} else {
		// All other bits go here
		SET_BIT(currentData->buttons[2], state, button - 16);
	}

	// Because of run colors (run is invalidated on modify), need to do some trickery

	// If this frame is not run, use the fast method
	if(!GET_BIT(currentData->frameState, FrameState::RAN)) {
		Freeze();
		wxRect itemRect;
		GetSubItemRect(currentFrame, buttonToColumn[button], itemRect);
		RefreshRect(itemRect);
		Thaw();
	} else {
		// If it's run, have to do a slow way, kinda
		FrameNum frame = currentFrame + 1;
		while(true) {
			if(!GET_BIT(inputsList[frame]->frameState, FrameState::RAN)) {
				// Refresh all these items
				// I don't care if it's way off the page, I think wxWidgets handles for this
				RefreshItems(currentFrame, frame - 1);
				break;
			}
			// Set bit
			SET_BIT(inputsList[frame]->frameState, false, FrameState::RAN);
			frame++;
		}
	}

	// Make the grid aware
	if(inputCallback) {
		inputCallback(button, state);
	}
}

void DataProcessing::toggleButtonState(Btn button) {
	// Send the `not` of the current state
	setButtonState(button, !getButtonState(button));
}

void DataProcessing::setCurrentFrame(FrameNum frameNum) {
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

		triggerCurrentFrameChanges();
	}
}

void DataProcessing::addNewFrame() {
	std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();
	// Set some defaults now
	SET_BIT(newControllerData->frameState, false, FrameState::RAN);
	SET_BIT(newControllerData->frameState, false, FrameState::SAVESTATE_HOOK);
	// Add this to the vector
	inputsList.push_back(newControllerData);
	// Because of the usability of virtual list controls, just update the length
	SetItemCount(inputsList.size());
	// Dont change the current frame (for now)
}

void DataProcessing::createSavestateHookHere() {
	// Add one savestate hook at this frame
	savestateHooks[currentFrame] = std::make_shared<SavestateHook>();
	// Set the style of this frame
	SET_BIT(currentData->frameState, true, FrameState::SAVESTATE_HOOK);
	// Refresh the item for it to take effect
	RefreshItem(currentFrame);
}

void DataProcessing::runFrame() {
	if(currentRunFrame != inputsList.size()) {
		std::shared_ptr<ControllerData> data = inputsList[currentRunFrame];

		// Change the state and get colors
		SET_BIT(data->frameState, 1, FrameState::RAN);
		RefreshItem(currentRunFrame);

		// If possible, make current frame this frame
		if(currentRunFrame < inputsList.size()) {
			// Set to this frame
			setCurrentFrame(currentRunFrame + 1);
		}

		// Increment run frame
		currentRunFrame++;
		// Set image frame to this too
		currentImageFrame = currentRunFrame;
		triggerCurrentFrameChanges();

		// Send to switch to run
		Protocol::Struct_SendRunFrame sendFrame;
		sendFrame.controllerData = *data;
		ADD_TO_QUEUE(SendRunFrame, sendFrame, networkInstance)
	}
}

void DataProcessing::handleKeyboardInput(wxChar key) {
	if(charToButton.count(key)) {
		toggleButtonState(charToButton[key]);
	}
}

DataProcessing::~DataProcessing() {
	for(auto& itemAttribute : itemAttributes) {
		// Free the sucker
		// free(itemAttribute.second);
	}
}
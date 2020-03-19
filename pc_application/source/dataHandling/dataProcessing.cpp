#include "dataProcessing.hpp"
#include "buttonData.hpp"

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

	InsertColumn(0, "Frame", wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

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

	// Set other frames manually, without a function
	currentRunFrame   = 0;
	currentImageFrame = 0;
	// Set the current frame to the first
	// One frame needs to be added at the very beginning
	addNewFrame();
	setCurrentFrame(0);

	// Set item attributes for nice colors
	setItemAttributes();
	// Add a few frame
	for(uint8_t i = 0; i < 30; i++) {
		addNewFrame();
	}

	// Create keyboard handlers
	// Each menu item is added here
	wxAcceleratorEntry entries[8];

	pasteInsertID   = wxNewId();
	pastePlaceID    = wxNewId();
	addFrameID      = wxNewId();
	frameAdvanceID  = wxNewId();
	savestateHookID = wxNewId();

	insertPaste = false;
	placePaste  = false;

	entries[0].Set(wxACCEL_CTRL, (int)'C', wxID_COPY, editMenu.Append(wxID_COPY, wxT("&Copy\tCtrl+C")));
	entries[1].Set(wxACCEL_CTRL, (int)'X', wxID_CUT, editMenu.Append(wxID_CUT, wxT("&Cut\tCtrl+X")));
	entries[2].Set(wxACCEL_CTRL, (int)'V', wxID_PASTE, editMenu.Append(wxID_PASTE, wxT("&Paste\tCtrl+V")));
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'V', pasteInsertID, editMenu.Append(pasteInsertID, wxT("&Paste Insert\tCtrl+Shift+V")));
	entries[4].Set(wxACCEL_CTRL | wxACCEL_ALT, (int)'V', pastePlaceID, editMenu.Append(pasteInsertID, wxT("&Paste Place\tCtrl+Alt+V")));

	entries[5].Set(wxACCEL_CTRL, (int)'=', addFrameID, editMenu.Append(addFrameID, wxT("&Add Frame\tCtrl+Plus")));
	entries[6].Set(wxACCEL_CTRL, WXK_RIGHT, frameAdvanceID, editMenu.Append(frameAdvanceID, wxT("&Frame Advance\tCtrl+Right")));
	entries[7].Set(wxACCEL_CTRL, (int)'H', savestateHookID, editMenu.Append(savestateHookID, wxT("&Add Savestate Hook\tCtrl+H")));

	wxAcceleratorTable accel(8, entries);
	SetAcceleratorTable(accel);

	// Bind each to a handler, both menu and button events
	Bind(wxEVT_MENU, &DataProcessing::onCopy, this, wxID_COPY);
	Bind(wxEVT_MENU, &DataProcessing::onCut, this, wxID_CUT);
	Bind(wxEVT_MENU, &DataProcessing::onPaste, this, wxID_PASTE);
	Bind(wxEVT_MENU, &DataProcessing::onInsertPaste, this, pasteInsertID);
	Bind(wxEVT_MENU, &DataProcessing::onPlacePaste, this, pastePlaceID);
	Bind(wxEVT_MENU, &DataProcessing::onAddFrame, this, addFrameID);
	Bind(wxEVT_MENU, &DataProcessing::onFrameAdvance, this, frameAdvanceID);
	Bind(wxEVT_MENU, &DataProcessing::onAddSavestateHook, this, savestateHookID);
}

// clang-format off
BEGIN_EVENT_TABLE(DataProcessing, wxListCtrl)
	EVT_LIST_CACHE_HINT(DataProcessing::LIST_CTRL_ID, DataProcessing::onCacheHint)
	EVT_LIST_ITEM_SELECTED(DataProcessing::LIST_CTRL_ID, DataProcessing::onSelect)
	// This is activated via double click
	EVT_LIST_ITEM_ACTIVATED(DataProcessing::LIST_CTRL_ID, DataProcessing::onActivate)
	EVT_CONTEXT_MENU(DataProcessing::onRightClick)
	EVT_ERASE_BACKGROUND(DataProcessing::OnEraseBackground)
END_EVENT_TABLE()
// clang-format on

void DataProcessing::setInputCallback(std::function<void()> callback) {
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
		bool on        = GET_BIT(inputsList[row]->buttons, button);
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

void DataProcessing::onRightClick(wxContextMenuEvent& event) {
	// Get item at location
	const wxPoint mousePosition = ScreenToClient(event.GetPosition());
	int flags                   = wxLIST_HITTEST_ONITEM;
	const FrameNum item         = HitTest(mousePosition, flags);
	if(item != wxNOT_FOUND) {
		setCurrentFrame(item);
	}

	PopupMenu(&editMenu, mousePosition);
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

void DataProcessing::onCopy(wxCommandEvent& event) {
	// First, try opening the clipboard
	// https://docs.wxwidgets.org/3.0/classwx_clipboard.html#a6c56dbf02b1807ce61cac8134a534336
	if(wxTheClipboard->Open()) {
		long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if(firstSelectedItem != wxNOT_FOUND) {
			long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
			// There is a selected item
			if(currentFrame >= firstSelectedItem && currentFrame <= lastSelectedItem) {
				// Add these items to the clipboard
				wxTheClipboard->SetData(new wxTextDataObject(buttonData->framesToText(inputsList, firstSelectedItem, lastSelectedItem)));
			} else {
				// Deselect the others
				for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
					SetItemState(i, 0, wxLIST_STATE_SELECTED);
				}
				// Select just the one
				SetItemState(currentFrame, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

				wxTheClipboard->SetData(new wxTextDataObject(buttonData->framesToText(inputsList, currentFrame, currentFrame)));

				// See the new selection
				RefreshItem(currentFrame);
			}
		}

		wxTheClipboard->Close();
	}
}

void DataProcessing::onCut(wxCommandEvent& event) {
	// Copy the elements, then delete
	onCopy(event);
	auto beginning = inputsList.begin();
	// Erase them
	inputsList.erase(beginning + currentFrame, beginning + currentFrame + GetSelectedItemCount());
	// Refresh to see things again
	SetItemCount(inputsList.size());
	Refresh();
	// Make the grid aware
	if(inputCallback) {
		// Doesn't matter what arguments
		inputCallback();
	}
}

void DataProcessing::onPaste(wxCommandEvent& event) {

	if(wxTheClipboard->Open()) {
		if(wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			std::string clipboardText = data.GetText().ToStdString();

			// Get to the meaty stuff, this is it
			buttonData->textToFrames(inputsList, clipboardText, currentFrame, insertPaste, placePaste);

			SetItemCount(inputsList.size());
			Refresh();
			// Make the grid aware
			if(inputCallback) {
				// Doesn't matter what arguments
				inputCallback();
			}
		}
		wxTheClipboard->Close();
	}
}

void DataProcessing::onInsertPaste(wxCommandEvent& event) {
	insertPaste = true;
	onPaste(event);
	insertPaste = false;
}

void DataProcessing::onPlacePaste(wxCommandEvent& event) {
	placePaste = true;
	onPaste(event);
	placePaste = false;
}

void DataProcessing::onAddFrame(wxCommandEvent& event) {
	// These are 1-to-1 from sideUI.cpp
	addNewFrame();
}

void DataProcessing::onFrameAdvance(wxCommandEvent& event) {
	runFrame();
}

void DataProcessing::onAddSavestateHook(wxCommandEvent& event) {
	createSavestateHookHere();
}

bool DataProcessing::getButtonState(Btn button) {
	// Get value from the bitflags
	return GET_BIT(currentData->buttons, button);
}

void DataProcessing::setButtonState(Btn button, bool state) {
	SET_BIT(currentData->buttons, state, button);

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
		inputCallback();
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

		// Make the grid aware
		if(inputCallback) {
			// Doesn't matter what arguments
			inputCallback();
		}

		triggerCurrentFrameChanges();

		Refresh();
	}
}

void DataProcessing::addNewFrame() {
	std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();
	// Set some defaults now
	SET_BIT(newControllerData->frameState, false, FrameState::RAN);
	SET_BIT(newControllerData->frameState, false, FrameState::SAVESTATE_HOOK);
	// Add this to the vector right after the selected frame
	if(inputsList.size() == 0) {
		inputsList.push_back(newControllerData);
	} else {
		inputsList.insert(inputsList.begin() + currentFrame + 1, newControllerData);
	}
	// Because of the usability of virtual list controls, just update the length
	SetItemCount(inputsList.size());
	// Dont change the current frame (for now)
	Refresh();
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
		std::shared_ptr<ControllerData> controllerData = inputsList[currentRunFrame];

		// Change the state and get colors
		SET_BIT(controllerData->frameState, 1, FrameState::RAN);
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

		Refresh();

		// Send to switch to run
		// clang-format off
		ADD_TO_QUEUE(SendRunFrame, networkInstance, {
			data.controllerData = *controllerData;
		})
		// clang-format on
	}
}

void DataProcessing::handleButtonInput(Btn button) {
	// Just a generic method to handle multiselect and others
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		// Now, apply the button
		// Usually, just set to the opposite of the currently selected element
		uint8_t state = !getButtonState(button);
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			// This is so hacky, but oh well
			setCurrentFrame(i);
			setButtonState(button, state);
		}
		Refresh();
	}
}

bool DataProcessing::handleKeyboardInput(wxChar key) {
	if(charToButton.count(key)) {
		handleButtonInput(charToButton[key]);
		return true;
	}
	return false;
}

DataProcessing::~DataProcessing() {
	for(auto& itemAttribute : itemAttributes) {
		// Free the sucker
		// free(itemAttribute.second);
	}
}
#include "dataProcessing.hpp"
#include "buttonData.hpp"

DataProcessing::DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES) {

	// All savestate hook blocks
	// Start with default, will get cleared later
	addNewPlayer();
	addNewSavestateHook("", HELPERS::getDefaultSavestateScreenshot());

	// This can't handle it :(
	SetDoubleBuffered(false);
	DragAcceptFiles(true);
	// Inherit from list control
	// Use this specific ID in order to do things
	buttonData      = buttons;
	mainSettings    = settings;
	networkInstance = communicateWithNetwork;
	// Set the mask color via a css string
	// https://docs.wxwidgets.org/3.0/classwx_colour.html#a08e9f56265647b8b5e1349b76eb728e3
	maskColor.Set((*mainSettings)["iconTransparent"].GetString());
	// This is cool, so set it
	// EnableAlternateRowColours(true);
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

	// Set item attributes for nice colors
	setItemAttributes();

	// Create keyboard handlers
	// Each menu item is added here
	wxAcceleratorEntry entries[11];

	pasteInsertID         = wxNewId();
	pastePlaceID          = wxNewId();
	addFrameID            = wxNewId();
	add10FramesID         = wxNewId();
	removeFrameID         = wxNewId();
	frameAdvanceID        = wxNewId();
	savestateID           = wxNewId();
	mergeIntoMainBranchID = wxNewId();

	insertPaste = false;
	placePaste  = false;

	entries[0].Set(wxACCEL_CTRL, (int)'C', wxID_COPY, editMenu.Append(wxID_COPY, wxT("Copy\tCtrl+C")));
	entries[1].Set(wxACCEL_CTRL, (int)'X', wxID_CUT, editMenu.Append(wxID_CUT, wxT("Cut\tCtrl+X")));
	entries[2].Set(wxACCEL_CTRL, (int)'V', wxID_PASTE, editMenu.Append(wxID_PASTE, wxT("Paste\tCtrl+V")));
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'V', pasteInsertID, editMenu.Append(pasteInsertID, wxT("Paste Insert\tCtrl+Shift+V")));
	entries[4].Set(wxACCEL_CTRL | wxACCEL_ALT, (int)'V', pastePlaceID, editMenu.Append(pastePlaceID, wxT("Paste Place\tCtrl+Alt+V")));

	entries[5].Set(wxACCEL_CTRL, (int)'=', addFrameID, editMenu.Append(addFrameID, wxT("Add Frame\tCtrl+Plus")));
	entries[6].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'=', add10FramesID, editMenu.Append(add10FramesID, wxT("Add 10 Frames\tCtrl+Plus")));
	entries[7].Set(0, WXK_DELETE, removeFrameID, editMenu.Append(removeFrameID, wxT("Remove Frame\tDelete")));
	entries[8].Set(wxACCEL_CTRL, WXK_RIGHT, frameAdvanceID, editMenu.Append(frameAdvanceID, wxT("Frame Advance\tCtrl+Right")));
	entries[9].Set(wxACCEL_CTRL, (int)'H', savestateID, editMenu.Append(savestateID, wxT("Add Savestate\tCtrl+H")));
	entries[10].Set(wxACCEL_CTRL, (int)'M', mergeIntoMainBranchID, editMenu.Append(mergeIntoMainBranchID, wxT("Merge Frames into Main Branch\tCtrl+M")));

	wxAcceleratorTable accel(11, entries);
	SetAcceleratorTable(accel);

	// Bind each to a handler, both menu and button events
	Bind(wxEVT_MENU, &DataProcessing::onCopy, this, wxID_COPY);
	Bind(wxEVT_MENU, &DataProcessing::onCut, this, wxID_CUT);
	Bind(wxEVT_MENU, &DataProcessing::onPaste, this, wxID_PASTE);
	Bind(wxEVT_MENU, &DataProcessing::onInsertPaste, this, pasteInsertID);
	Bind(wxEVT_MENU, &DataProcessing::onPlacePaste, this, pastePlaceID);
	Bind(wxEVT_MENU, &DataProcessing::onAddFrame, this, addFrameID);
	Bind(wxEVT_MENU, &DataProcessing::onAdd10Frames, this, add10FramesID);
	Bind(wxEVT_MENU, &DataProcessing::onRemoveFrame, this, removeFrameID);
	Bind(wxEVT_MENU, &DataProcessing::onFrameAdvance, this, frameAdvanceID);
	Bind(wxEVT_MENU, &DataProcessing::onAddSavestate, this, savestateID);
	Bind(wxEVT_MENU, &DataProcessing::onMergeIntoMainBranch, this, mergeIntoMainBranchID);

	Bind(wxEVT_LIST_CACHE_HINT, &DataProcessing::onCacheHint, this);
	Bind(wxEVT_LIST_ITEM_SELECTED, &DataProcessing::onSelect, this);
	Bind(wxEVT_LIST_ITEM_ACTIVATED, &DataProcessing::onActivate, this);
}

// clang-format off
BEGIN_EVENT_TABLE(DataProcessing, wxListCtrl)
	EVT_CONTEXT_MENU(DataProcessing::onRightClick)
	EVT_ERASE_BACKGROUND(DataProcessing::OnEraseBackground)
	EVT_DROP_FILES(DataProcessing::onDropFiles)
END_EVENT_TABLE()
// clang-format on

void DataProcessing::setInputCallback(std::function<void(uint8_t)> callback) {
	inputCallback = callback;
}

void DataProcessing::setSelectedFrameCallbackVideoViewer(std::function<void(int)> callback) {
	selectedFrameCallbackVideoViewer = callback;
}

void DataProcessing::setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback) {
	viewableInputsCallback = callback;
}

void DataProcessing::setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback) {
	changingSelectedFrameCallback = callback;
}

void DataProcessing::setPlayerInfoCallback(std::function<void(uint8_t, uint8_t, bool)> callback) {
	playerInfoCallback = callback;
}

void DataProcessing::setBranchInfoCallback(std::function<void(uint16_t, uint16_t, bool)> callback) {
	branchInfoCallback = callback;
}

void DataProcessing::triggerCurrentFrameChanges() {
	if(changingSelectedFrameCallback) {
		changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
	}
}

void DataProcessing::sendAutoAdvance(uint8_t includeFramebuffer, TasValueToRecord valueToRecord, uint8_t incrementFrame) {
	std::shared_ptr<TouchAndKeyboardData> extraDatas = getControllerDataExtra(currentSavestateHook, viewingBranchIndex, currentRunFrame);

	for(uint8_t playerIndex = 0; playerIndex < allPlayers.size(); playerIndex++) {
		// Set inputs of all other players correctly but not the current one
		// Unless the controller is not being recorded, in that case add it
		if(playerIndex != viewingPlayerIndex || valueToRecord != TasValueToRecord::CONTROLLER) {
			std::shared_ptr<ControllerData> controllerDatas = getControllerData(playerIndex, currentSavestateHook, viewingBranchIndex, currentRunFrame);

			ADD_TO_QUEUE(SendFrameData, networkInstance, {
				data.controllerData = *controllerDatas;
				data.incrementFrame = false;
				data.typeToRecord   = TasValueToRecord::NONE;
				data.valueIncluded  = TasValueToRecord::CONTROLLER;
			})
		}
	}

	if(valueToRecord != TasValueToRecord::KEYBOARD_MOUSE) {
		std::shared_ptr<TouchAndKeyboardData> sendData = std::make_shared<TouchAndKeyboardData>();
		buttonData->transferOnlyKeyboard(*extraDatas, sendData);

		ADD_TO_QUEUE(SendFrameData, networkInstance, {
			data.extraData      = *sendData;
			data.incrementFrame = false;
			data.typeToRecord   = TasValueToRecord::NONE;
			data.valueIncluded  = TasValueToRecord::KEYBOARD_MOUSE;
		})
	}

	if(valueToRecord != TasValueToRecord::TOUCHSCREEN) {
		std::shared_ptr<TouchAndKeyboardData> sendData = std::make_shared<TouchAndKeyboardData>();
		buttonData->transferOnlyTouch(*extraDatas, sendData);

		ADD_TO_QUEUE(SendFrameData, networkInstance, {
			data.extraData      = *sendData;
			data.incrementFrame = false;
			data.typeToRecord   = TasValueToRecord::NONE;
			data.valueIncluded  = TasValueToRecord::TOUCHSCREEN;
		})
	}

	// Actually trigger the increment
	ADD_TO_QUEUE(SendFrameData, networkInstance, {
		data.frame              = currentRunFrame + 1;
		data.savestateHookNum   = currentSavestateHook;
		data.branchIndex        = viewingBranchIndex;
		data.playerIndex        = viewingPlayerIndex;
		data.incrementFrame     = incrementFrame;
		data.includeFramebuffer = includeFramebuffer;
		data.typeToRecord       = valueToRecord;
	})
}

std::string DataProcessing::getExportedCurrentPlayer() {
	/*
	wxFile file(exportTarget.GetFullPath(), wxFile::write);

	if(file.IsOpened()) {
		std::string exported = buttonData->framesToText(this, 0, 0, viewingPlayerIndex);
		file.Write(wxString::FromUTF8(exported));
		file.Close();
	}
	*/

	// This blocks for a long time
	// Always export the main branch, MAY CHANGE
	return buttonData->framesToText(this, 0, 0, viewingPlayerIndex, 0);
}

void DataProcessing::importFromFile(wxFileName importTarget) {
	wxFile file(importTarget.GetFullPath(), wxFile::read);

	if(file.IsOpened()) {
		wxString fileContents;
		bool successful = file.ReadAll(&fileContents);
		file.Close();

		if(successful) {
			Freeze();
			FrameNum lastFrame = buttonData->textToFrames(this, fileContents.ToStdString(), 0, false, false);
			// Remove all frames after the data
			removeFrames(lastFrame + 1, allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->size() - 1);
			Thaw();
			setCurrentFrame(0);
			Refresh();
		}
	}
}

void DataProcessing::onDropFiles(wxDropFilesEvent& event) {
	if(event.GetNumberOfFiles() == 1) {
		wxString dropped = event.GetFiles()[0];
		wxFileName file(dropped);

		if(file.FileExists()) {
			importFromFile(file);
		}
	}
}

int DataProcessing::OnGetItemColumnImage(long row, long column) const {
	if(column == 0) {
		// This is the frame num
		return -1;
	} else {
		// Returns index in the imagelist
		// Need to account for the frame being first
		Btn button = (Btn)(column - 1);
		bool on    = getButton(row, button);
		int res;
		if(on) {
			// Return index of on image
			// Interleaved means it looks like this
			res = (uint8_t)button * 2;
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
		return wxString::Format(wxT("%ld"), row);
	} else {
		// Fallback for every other column
		return "";
	}
}

wxItemAttr* DataProcessing::OnGetItemAttr(long item) const {
	return itemAttributes.at(getFramestateInfo(item));
}

void DataProcessing::setItemAttributes() {
	// Reuse both variables
	uint8_t state = 0;
	wxItemAttr* itemAttribute;

	// Default is nothing
	SET_BIT(state, false, (uint8_t)FrameState::RAN);
	itemAttribute = new wxItemAttr();
	// itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["notRan"].GetString()));
	itemAttribute->SetBackgroundColour(GetBackgroundColour());
	itemAttributes[state] = itemAttribute;

	SET_BIT(state, true, (uint8_t)FrameState::RAN);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["ran"].GetString()));
	itemAttributes[state] = itemAttribute;
	SET_BIT(state, false, (uint8_t)FrameState::RAN);

	SET_BIT(state, true, (uint8_t)FrameState::SAVESTATE);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["savestate"].GetString()));
	itemAttributes[state] = itemAttribute;
	// SavestateHook takes precedence in the case where both are present
	SET_BIT(state, true, (uint8_t)FrameState::RAN);
	itemAttributes[state] = itemAttribute;
}

void DataProcessing::OnEraseBackground(wxEraseEvent& event) {
	// to prevent flickering, erase only content *outside* of the
	// actual list items stuff

	if(GetItemCount() != 0) {
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
				wxTheClipboard->SetData(new wxTextDataObject(buttonData->framesToText(this, firstSelectedItem, lastSelectedItem, -1, viewingBranchIndex)));
			} else {
				// Deselect the others
				for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
					SetItemState(i, 0, wxLIST_STATE_SELECTED);
				}
				// Select just the one
				SetItemState(currentFrame, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

				wxTheClipboard->SetData(new wxTextDataObject(buttonData->framesToText(this, currentFrame, currentFrame, -1, viewingBranchIndex)));

				// See the new selection
				RefreshItem(currentFrame);
			}

			wxTheClipboard->Close();
		}
	}
}

void DataProcessing::onCut(wxCommandEvent& event) {
	// Copy the elements, then delete
	onCopy(event);
	// Erase the frames
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		removeFrames(firstSelectedItem, lastSelectedItem);
	}
}

void DataProcessing::onPaste(wxCommandEvent& event) {

	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;

		if(wxTheClipboard->Open()) {
			if(wxTheClipboard->IsSupported(wxDF_TEXT)) {
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				wxTheClipboard->Close();

				std::string clipboardText = data.GetText().ToStdString();

				Freeze();
				FrameNum lastItem    = buttonData->textToFrames(this, clipboardText, firstSelectedItem, insertPaste, placePaste);
				FrameNum sizeOfPaste = lastItem - firstSelectedItem + 1;
				if(!insertPaste) {
					for(long i = firstSelectedItem; i <= lastSelectedItem; i += sizeOfPaste) {
						buttonData->textToFrames(this, clipboardText, i, insertPaste, placePaste);
					}
				}
				setCurrentFrame(firstSelectedItem + sizeOfPaste - 1);
				Thaw();
				Refresh();
			}
		}
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
	addFrame(currentFrame);
}

void DataProcessing::onAdd10Frames(wxCommandEvent& event) {
	for(uint8_t i = 0; i < 10; i++) {
		addFrame(currentFrame);
	}
}

void DataProcessing::onRemoveFrame(wxCommandEvent& event) {
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		removeFrames(firstSelectedItem, lastSelectedItem);
	}
}

void DataProcessing::onFrameAdvance(wxCommandEvent& event) {
	if(tethered) {
		runFrame(false, false, true);
	}
}

void DataProcessing::onAddSavestate(wxCommandEvent& event) {
	// NEEDS WORK
	createSavestateHere();
}

void DataProcessing::onMergeIntoMainBranch(wxCommandEvent& event) {
	// Don't just convert into text format, merge by moving over frames
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			// Transfer directly to first branch
			buttonData->transferControllerData(*getInputsList()->at(i), allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[0]->at(i), false);
			// Include keyboard and touch
			buttonData->transferExtraData(*getInputsExtraList()->at(i), allExtraFrameData[currentSavestateHook]->at(0)->at(i), false);
		}
	}
	// It's up to the user to remove the frames in the other branch if they want
}

void DataProcessing::setCurrentFrame(FrameNum frameNum) {
	// Must be a frame that has already been written, else, raise error
	if(frameNum < getFramesSize()) {
		// Set the current frame to this frame
		// Shared pointer so this can be done
		currentData = allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(frameNum);
		// Set the current frame to this number
		// Focus to this specific row now
		// This essentially scrolls to it

		if(GetItemCount() != 1) {
			EnsureVisible(frameNum);
		}

		currentFrame = frameNum;

		// If not selected by wxWidgets, do so
		if(!GetItemState(frameNum, wxLIST_STATE_FOCUSED)) {
			// This will trigger another setFrame, but that's fine
			SetItemState(0, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
		}

		modifyCurrentFrameViews(frameNum);
		if(selectedFrameCallbackVideoViewer) {
			selectedFrameCallbackVideoViewer((int)frameNum - currentFrame);
		}

		Refresh();
	}
}

// THIS NEEDS TO CHANGE COMPLETELY
void DataProcessing::createSavestateHere() {
	// NEEDS WORK UNUSED RIGHT NOW
	// Add one savestate hook at this frame
	savestates[currentFrame] = std::make_shared<Savestate>();
	// Set the style of this frame
	SET_BIT(currentData->frameState, true, (uint8_t)FrameState::SAVESTATE);
	// Refresh the item for it to take effect
	RefreshItem(currentFrame);
}

void DataProcessing::runFrame(uint8_t forAutoFrame, uint8_t updateFramebuffer, uint8_t includeFramebuffer) {
	if(currentRunFrame < allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->size() - 1) {
		// Technically, should handle for entering next savetstate hook block, but TODO
		std::shared_ptr<ControllerData> controllerData = allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(currentRunFrame);

		setFramestateInfo(currentRunFrame, FrameState::RAN, true);

		uint8_t withinFrames = currentRunFrame < allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->size();

		// If possible, make current frame this frame
		if(withinFrames) {
			// Set to this frame
			setCurrentFrame(currentRunFrame + 1);
		}

		// Increment run frame
		currentRunFrame++;
		// Set image frame to this too
		currentImageFrame = currentRunFrame;

		modifyCurrentFrameViews(currentFrame);

		// Refresh the grid
		if(changingSelectedFrameCallback) {
			changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
		}

		if(inputCallback) {
			// Doesn't matter what arguments
			inputCallback(updateFramebuffer);
		}

		Refresh();

		if(currentRunFrame < getFramesSize()) {
			if(!forAutoFrame) {
				sendAutoAdvance(includeFramebuffer, TasValueToRecord::NONE, true);
			}
		}
	}
}

bool DataProcessing::handleKeyboardInput(wxChar key) {
	if(charToButton.count(key)) {
		triggerButton(charToButton[key]);
		return true;
	}
	return false;
}

BranchData DataProcessing::getInputsList() const {
	return currentBranchData;
}

ExtraBranchData DataProcessing::getInputsExtraList() const {
	return currentExtraData;
}

wxRect DataProcessing::getFirstItemRect() {
	wxRect itemRect;
	long topItem = GetTopItem();
	GetItemRect(topItem, itemRect);
	return itemRect;
}

void DataProcessing::onCacheHint(wxListEvent& event) {
	if(viewableInputsCallback) {
		long numOfRowsVisible = GetCountPerPage();
		if(numOfRowsVisible != 0) {
			// Don't use the event values, they are wrong
			long first = GetTopItem();
			long last  = first + numOfRowsVisible;

			if(viewableInputsCallback) {
				viewableInputsCallback(first, last);
			}
		}
	}
}

void DataProcessing::addNewSavestateHook(std::string dHash, wxBitmap* screenshot) {
	// Has to be done for every controller
	for(uint8_t i = 0; i < allPlayers.size(); i++) {
		std::shared_ptr<SavestateHook> savestateHook = std::make_shared<SavestateHook>();
		savestateHook->dHash                         = dHash;
		savestateHook->screenshot                    = screenshot;
		savestateHook->runFinalTasDelayFrames        = 0;
		// Add a single branch for default
		savestateHook->inputs.push_back(std::make_shared<std::vector<FrameData>>());
		allPlayers[i]->push_back(savestateHook);
		// Add one controller data to the first branch
		allPlayers[i]->at(allPlayers.size() - 1)->inputs[0]->push_back(std::make_shared<ControllerData>());
	}

	std::shared_ptr<std::vector<std::shared_ptr<std::vector<ExtraFrameData>>>> thisHookExtraData = std::make_shared<std::vector<std::shared_ptr<std::vector<ExtraFrameData>>>>();
	thisHookExtraData->push_back(std::make_shared<std::vector<ExtraFrameData>>());
	thisHookExtraData->at(0)->push_back(std::make_shared<TouchAndKeyboardData>());

	allExtraFrameData.push_back(thisHookExtraData);

	viewingBranchIndex = 0;
	// NOTE: There must be at least one block with one input when this is loaded
	// Automatically, the first block is always at index 0
	setSavestateHook(allPlayers[0]->size() - 1);
	setBranch(0);
}

void DataProcessing::setSavestateHook(SavestateBlockNum index) {
	currentSavestateHook = index;

	// Just in case, refresh branch here
	setBranch(viewingBranchIndex);
	SetItemCount(getInputsList()->size());
	setCurrentFrame(0);
	currentRunFrame   = 0;
	currentImageFrame = 0;

	modifyCurrentFrameViews(currentFrame);

	if(changingSelectedFrameCallback) {
		changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
	}

	Refresh();
}

void DataProcessing::removeSavestateHook(SavestateBlockNum index) {
	if(allPlayers[viewingPlayerIndex]->size() > 1) {
		allPlayers[viewingPlayerIndex]->erase(allPlayers[viewingPlayerIndex]->begin() + index);
		allExtraFrameData.erase(allExtraFrameData.begin() + index);
		setSavestateHook(0);

		// Move over all the framebuffer names
		wxRemoveFile(getFramebufferPathForSavestateHook(index).GetFullPath());
		HELPERS::popOffDirs(getFramebufferPath(0, index, 0, 0), 1).Rmdir(wxPATH_RMDIR_RECURSIVE);

		// Rename all images following this hook
		SavestateBlockNum temp1 = index;
		while(true) {
			temp1++;
			wxFileName savestateHookFile = getFramebufferPathForSavestateHook(temp1);
			if(savestateHookFile.FileExists()) {
				wxRenameFile(savestateHookFile.GetPath(), getFramebufferPathForSavestateHook(temp1).GetPath());
			} else {
				// Have encountered last savestate hook, break loop
				break;
			}
		}

		// Rename all folders following this hook
		SavestateBlockNum temp2 = index;
		while(true) {
			temp2++;
			wxFileName savestateHookDir = HELPERS::popOffDirs(getFramebufferPath(0, temp2, 0, 0), 1);
			if(savestateHookDir.DirExists()) {
				wxRenameFile(savestateHookDir.GetPath(), HELPERS::popOffDirs(getFramebufferPath(0, temp2 - 1, 0, 0), 1).GetPath());
			} else {
				// Have encountered last savestate hook, break loop
				break;
			}
		}
	}
}

void DataProcessing::addNewPlayer() {
	if(allPlayers.size() < 4) {
		std::shared_ptr<std::vector<std::shared_ptr<SavestateHook>>> player = std::make_shared<std::vector<std::shared_ptr<SavestateHook>>>();

		if(allPlayers.size() != 0) {
			sendPlayerNum();
			// Match this player to the number of savestate hooks as the first player
			for(auto const& hook : *allPlayers[0]) {
				std::shared_ptr<SavestateHook> newSavestateHook = std::make_shared<SavestateHook>();

				// Has the same number of branches
				for(FrameNum i = 0; i < hook->inputs.size(); i++) {
					newSavestateHook->inputs.push_back(std::make_shared<std::vector<std::shared_ptr<ControllerData>>>());
					// Each of those branches have the same number of inputs
					for(FrameNum j = 0; j < hook->inputs[i]->size(); j++) {
						// Create empty frames to add
						newSavestateHook->inputs[i]->push_back(std::make_shared<ControllerData>());
					}
				}

				newSavestateHook->dHash                  = "";
				newSavestateHook->screenshot             = HELPERS::getDefaultSavestateScreenshot();
				newSavestateHook->runFinalTasDelayFrames = 0;
				player->push_back(newSavestateHook);
			}
		}

		allPlayers.push_back(player);

		setPlayer(allPlayers.size() - 1);
	}
}

void DataProcessing::setPlayer(uint8_t playerIndex) {
	viewingPlayerIndex = playerIndex;
	// Make sure there are savestate hooks
	if(allPlayers[viewingPlayerIndex]->size() != 0) {
		FrameNum curFrame = currentFrame;
		setSavestateHook(currentSavestateHook);
		setCurrentFrame(curFrame);
	}
	if(playerInfoCallback) {
		playerInfoCallback(allPlayers.size(), viewingPlayerIndex, false);
	}
	if(branchInfoCallback) {
		branchInfoCallback(getNumBranches(), viewingBranchIndex, true);
	}
}

void DataProcessing::removePlayer(uint8_t playerIndex) {
	if(allPlayers.size() > 1) {
		allPlayers.erase(allPlayers.begin() + playerIndex);
		setPlayer(allPlayers.size() - 1);
	}
	sendPlayerNum();

	// No framebuffer folders to remove because players dont have specific framebuffers
}

void DataProcessing::removeThisPlayer() {
	removePlayer(viewingPlayerIndex);
}

void DataProcessing::sendPlayerNum() {
	uint8_t size = allPlayers.size();
	// Also sends to SideUI
	if(playerInfoCallback) {
		playerInfoCallback(size, viewingPlayerIndex, false);
	}
}

std::shared_ptr<ControllerData> DataProcessing::getFrame(FrameNum frame) const {
	return getInputsList()->at(frame);
}

std::shared_ptr<TouchAndKeyboardData> DataProcessing::getFrameExtra(FrameNum frame) const {
	return getInputsExtraList()->at(frame);
}

void DataProcessing::addNewBranch() {
	// Only add to this player
	for(auto& player : allPlayers) {
		auto& list = player->at(currentSavestateHook)->inputs;
		list.push_back(std::make_shared<std::vector<std::shared_ptr<ControllerData>>>());
		uint16_t lastElement = list.size() - 1;
		// Add number of frames as the first branch has
		for(FrameNum j = 0; j < list[0]->size(); j++) {
			list[lastElement]->push_back(std::make_shared<ControllerData>());
		}
	}
	auto& extraFrameDataBranch = allExtraFrameData[currentSavestateHook];
	extraFrameDataBranch->push_back(std::make_shared<std::vector<ExtraFrameData>>());
	uint16_t lastElement = extraFrameDataBranch->size() - 1;
	for(FrameNum j = 0; j < extraFrameDataBranch->at(0)->size(); j++) {
		extraFrameDataBranch->at(lastElement)->push_back(std::make_shared<TouchAndKeyboardData>());
	}
	setBranch(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size() - 1);
}

void DataProcessing::setBranch(uint16_t branchIndex) {
	viewingBranchIndex = branchIndex;
	currentBranchData  = allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex];
	currentExtraData   = allExtraFrameData[currentSavestateHook]->at(viewingBranchIndex);
	if(branchInfoCallback) {
		branchInfoCallback(getNumBranches(), branchIndex, true);
	}
	modifyCurrentFrameViews(currentFrame);
	Refresh();
}

void DataProcessing::removeBranch(uint8_t branchIndex) {
	if(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size() > 1) {
		allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.erase(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.begin() + branchIndex);
		allExtraFrameData[currentSavestateHook]->erase(allExtraFrameData[currentSavestateHook]->begin() + branchIndex);
		setBranch(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size() - 1);

		getFramebufferPath(0, currentSavestateHook, branchIndex, 0).Rmdir(wxPATH_RMDIR_RECURSIVE);

		// Rename all images in this branch
		while(true) {
			branchIndex++;
			wxFileName branchFolder = getFramebufferPath(0, currentSavestateHook, branchIndex, 0);
			if(branchFolder.DirExists()) {
				wxRenameFile(branchFolder.GetPath(), getFramebufferPath(0, currentSavestateHook, branchIndex - 1, 0).GetPath());
			} else {
				// Have encountered last savestate hook, break loop
				break;
			}
		}
	}
}

void DataProcessing::removeThisBranch() {
	removeBranch(viewingBranchIndex);
}

void DataProcessing::scrollToSpecific(uint8_t player, SavestateBlockNum savestateHookNum, BranchNum branch, FrameNum frame) {
	setPlayer(player);
	setSavestateHook(savestateHookNum);
	setCurrentFrame(frame);
	setBranch(branch);
}

void DataProcessing::triggerButton(Btn button) {
	// Trigger button, can occur over range
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		// Now, apply the button
		// Usually, just set to the opposite of the currently selected element
		uint8_t state = !getButton(currentFrame, button);
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			modifyButton(i, button, state);
		}
	}
}

// This includes joysticks, accel, gyro, etc...
void DataProcessing::triggerNumberValuesJoystick(ControllerNumberValues joystickId, int16_t value) {
	// Trigger joystick, can occur over range
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setNumberValuesJoystick(i, joystickId, value);
			// No refresh for now, as the joystick is not visible in the allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs
		}
	}
}

// New FANCY methods
void DataProcessing::modifyButton(FrameNum frame, Btn button, uint8_t isPressed) {
	SET_BIT(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(frame)->buttons, isPressed, (uint8_t)button);

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

void DataProcessing::toggleButton(FrameNum frame, Btn button) {
	modifyButton(frame, button, !getButton(frame, button));
}

void DataProcessing::clearAllButtons(FrameNum frame) {
	// I think this works
	getInputsList()->at(frame)->buttons = 0;

	invalidateRun(frame);
	modifyCurrentFrameViews(frame);
	RefreshItem(frame);
}

void DataProcessing::setNumberValuesJoystick(FrameNum frame, ControllerNumberValues joystickId, int16_t value) {
	auto& frameData = getInputsList()->at(frame);
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		frameData->LS_X = value;
		break;
	case ControllerNumberValues::LEFT_Y:
		frameData->LS_Y = value;
		break;
	case ControllerNumberValues::RIGHT_X:
		frameData->RS_X = value;
		break;
	case ControllerNumberValues::RIGHT_Y:
		frameData->RS_Y = value;
		break;
	}

	modifyCurrentFrameViews(frame);
	invalidateRun(frame);
}

void DataProcessing::setNumberValuesMotion(FrameNum frame, ControllerNumberValues joystickId, float value) {
	auto& frameData = getInputsList()->at(frame);
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		frameData->ACCEL_X_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		frameData->ACCEL_Y_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		frameData->ACCEL_Z_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		frameData->GYRO_X_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		frameData->GYRO_Y_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		frameData->GYRO_Z_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		frameData->ANGLE_X_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		frameData->ANGLE_Y_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		frameData->ANGLE_Z_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		frameData->ACCEL_X_RIGHT = value;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		frameData->ACCEL_Y_RIGHT = value;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		frameData->ACCEL_Z_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		frameData->GYRO_X_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		frameData->GYRO_Y_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		frameData->GYRO_Z_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		frameData->ANGLE_X_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		frameData->ANGLE_Y_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		frameData->ANGLE_Z_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_XX_LEFT:
		frameData->DIRECTION_XX_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_XY_LEFT:
		frameData->DIRECTION_XY_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_XZ_LEFT:
		frameData->DIRECTION_XZ_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_YX_LEFT:
		frameData->DIRECTION_YX_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_YY_LEFT:
		frameData->DIRECTION_YY_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_YZ_LEFT:
		frameData->DIRECTION_YZ_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZX_LEFT:
		frameData->DIRECTION_ZX_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZY_LEFT:
		frameData->DIRECTION_ZY_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_LEFT:
		frameData->DIRECTION_ZZ_LEFT = value;
		break;
	case ControllerNumberValues::DIRECTION_XX_RIGHT:
		frameData->DIRECTION_XX_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_XY_RIGHT:
		frameData->DIRECTION_XY_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_XZ_RIGHT:
		frameData->DIRECTION_XZ_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_YX_RIGHT:
		frameData->DIRECTION_YX_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_YY_RIGHT:
		frameData->DIRECTION_YY_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_YZ_RIGHT:
		frameData->DIRECTION_YZ_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZX_RIGHT:
		frameData->DIRECTION_ZX_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZY_RIGHT:
		frameData->DIRECTION_ZY_RIGHT = value;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_RIGHT:
		frameData->DIRECTION_ZZ_RIGHT = value;
		break;
	}

	modifyCurrentFrameViews(frame);
	invalidateRun(frame);
}

int16_t DataProcessing::getNumberValuesJoystick(FrameNum frame, ControllerNumberValues joystickId) const {
	auto& frameData = getInputsList()->at(frame);
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		return frameData->LS_X;
		break;
	case ControllerNumberValues::LEFT_Y:
		return frameData->LS_Y;
		break;
	case ControllerNumberValues::RIGHT_X:
		return frameData->RS_X;
		break;
	case ControllerNumberValues::RIGHT_Y:
		return frameData->RS_Y;
		break;
	}
	return 0;
}

float DataProcessing::getNumberValuesMotion(FrameNum frame, ControllerNumberValues joystickId) const {
	auto& frameData = getInputsList()->at(frame);
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		return frameData->ACCEL_X_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		return frameData->ACCEL_Y_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		return frameData->ACCEL_Z_LEFT;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		return frameData->GYRO_X_LEFT;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		return frameData->GYRO_Y_LEFT;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		return frameData->GYRO_Z_LEFT;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		return frameData->ANGLE_X_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		return frameData->ANGLE_Y_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		return frameData->ANGLE_Z_LEFT;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		return frameData->ACCEL_X_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		return frameData->ACCEL_Y_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		return frameData->ACCEL_Z_RIGHT;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		return frameData->GYRO_X_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		return frameData->GYRO_Y_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		return frameData->GYRO_Z_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		return frameData->ANGLE_X_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		return frameData->ANGLE_Y_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		return frameData->ANGLE_Z_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XX_LEFT:
		return frameData->DIRECTION_XX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XY_LEFT:
		return frameData->DIRECTION_XY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XZ_LEFT:
		return frameData->DIRECTION_XZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YX_LEFT:
		return frameData->DIRECTION_YX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YY_LEFT:
		return frameData->DIRECTION_YY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YZ_LEFT:
		return frameData->DIRECTION_YZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZX_LEFT:
		return frameData->DIRECTION_ZX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZY_LEFT:
		return frameData->DIRECTION_ZY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_LEFT:
		return frameData->DIRECTION_ZZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XX_RIGHT:
		return frameData->DIRECTION_XX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XY_RIGHT:
		return frameData->DIRECTION_XY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XZ_RIGHT:
		return frameData->DIRECTION_XZ_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YX_RIGHT:
		return frameData->DIRECTION_YX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YY_RIGHT:
		return frameData->DIRECTION_YY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YZ_RIGHT:
		return frameData->DIRECTION_YZ_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZX_RIGHT:
		return frameData->DIRECTION_ZX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZY_RIGHT:
		return frameData->DIRECTION_ZY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_RIGHT:
		return frameData->DIRECTION_ZZ_RIGHT;
		break;
	}
	return 0;
}

int16_t DataProcessing::getNumberValuesSpecificJoystick(FrameNum frame, ControllerNumberValues joystickId, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	auto const& frameData = getControllerData(player, savestateHookNum, branch, frame);
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		return frameData->LS_X;
		break;
	case ControllerNumberValues::LEFT_Y:
		return frameData->LS_Y;
		break;
	case ControllerNumberValues::RIGHT_X:
		return frameData->RS_X;
		break;
	case ControllerNumberValues::RIGHT_Y:
		return frameData->RS_Y;
		break;
	}
	return 0;
}

float DataProcessing::getNumberValuesSpecificMotion(FrameNum frame, ControllerNumberValues joystickId, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	auto const& frameData = getControllerData(player, savestateHookNum, branch, frame);
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		return frameData->ACCEL_X_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		return frameData->ACCEL_Y_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		return frameData->ACCEL_Z_LEFT;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		return frameData->GYRO_X_LEFT;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		return frameData->GYRO_Y_LEFT;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		return frameData->GYRO_Z_LEFT;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		return frameData->ANGLE_X_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		return frameData->ANGLE_Y_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		return frameData->ANGLE_Z_LEFT;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		return frameData->ACCEL_X_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		return frameData->ACCEL_Y_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		return frameData->ACCEL_Z_RIGHT;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		return frameData->GYRO_X_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		return frameData->GYRO_Y_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		return frameData->GYRO_Z_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		return frameData->ANGLE_X_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		return frameData->ANGLE_Y_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		return frameData->ANGLE_Z_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XX_LEFT:
		return frameData->DIRECTION_XX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XY_LEFT:
		return frameData->DIRECTION_XY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XZ_LEFT:
		return frameData->DIRECTION_XZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YX_LEFT:
		return frameData->DIRECTION_YX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YY_LEFT:
		return frameData->DIRECTION_YY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_YZ_LEFT:
		return frameData->DIRECTION_YZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZX_LEFT:
		return frameData->DIRECTION_ZX_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZY_LEFT:
		return frameData->DIRECTION_ZY_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_LEFT:
		return frameData->DIRECTION_ZZ_LEFT;
		break;
	case ControllerNumberValues::DIRECTION_XX_RIGHT:
		return frameData->DIRECTION_XX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XY_RIGHT:
		return frameData->DIRECTION_XY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_XZ_RIGHT:
		return frameData->DIRECTION_XZ_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YX_RIGHT:
		return frameData->DIRECTION_YX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YY_RIGHT:
		return frameData->DIRECTION_YY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_YZ_RIGHT:
		return frameData->DIRECTION_YZ_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZX_RIGHT:
		return frameData->DIRECTION_ZX_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZY_RIGHT:
		return frameData->DIRECTION_ZY_RIGHT;
		break;
	case ControllerNumberValues::DIRECTION_ZZ_RIGHT:
		return frameData->DIRECTION_ZZ_RIGHT;
		break;
	}
}

// This includes joysticks, accel, gyro, etc...
void DataProcessing::triggerNumberValuesMotion(ControllerNumberValues id, float value) {
	// Trigger joystick, can occur over range
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setNumberValuesMotion(i, id, value);
			// No refresh for now, as the joystick is not visible in the allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs
		}
	}
}

void DataProcessing::triggerNumberOfTouches(uint8_t value) {
	// Trigger button, can occur over range
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setNumberOfTouches(i, value);
		}
	}
}

void DataProcessing::setNumberOfTouches(FrameNum frame, uint8_t value) {
	getInputsExtraList()->at(frame)->numberOfTouches = value;

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

uint8_t DataProcessing::getNumberOfTouches(FrameNum frame) const {
	return getInputsExtraList()->at(frame)->numberOfTouches;
}

uint8_t DataProcessing::getNumberOfTouchesSpecific(FrameNum frame, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return getControllerDataExtra(savestateHookNum, branch, frame)->numberOfTouches;
}

uint8_t DataProcessing::getNumberOfTouchesCurrent() const {
	return getNumberOfTouches(currentFrame);
}

void DataProcessing::triggerExtraValue(ExtraValues extraValue, int32_t value) {
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setExtraValue(i, extraValue, value);
		}
	}
}

void DataProcessing::setExtraValue(FrameNum frame, ExtraValues extraValue, int32_t value) {
	auto& frameData = getInputsExtraList()->at(frame);
	switch(extraValue) {
	case TOUCH_X_1:
		frameData->touchX1 = value;
		break;
	case TOUCH_Y_1:
		frameData->touchY1 = value;
		break;
	case TOUCH_X_2:
		frameData->touchX2 = value;
		break;
	case TOUCH_Y_2:
		frameData->touchY2 = value;
		break;
	case MOUSE_X:
		frameData->mouseX = value;
		break;
	case MOUSE_Y:
		frameData->mouseY = value;
		break;
	case MOUSE_VELOCITY_X:
		frameData->mouseVelocityX = value;
		break;
	case MOUSE_VELOCITY_Y:
		frameData->mouseVelocityY = value;
		break;
	case SCROLL_VELOCITY_X:
		frameData->scrollVelocityX = value;
		break;
	case SCROLL_VELOCITY_Y:
		frameData->scrollVelocityY = value;
		break;
	}

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

int32_t DataProcessing::getExtraValue(FrameNum frame, ExtraValues extraValue) const {
	auto const& frameData = getInputsExtraList()->at(frame);
	switch(extraValue) {
	case TOUCH_X_1:
		return frameData->touchX1;
		break;
	case TOUCH_Y_1:
		return frameData->touchY1;
		break;
	case TOUCH_X_2:
		return frameData->touchX2;
		break;
	case TOUCH_Y_2:
		return frameData->touchY2;
		break;
	case MOUSE_X:
		return frameData->mouseX;
		break;
	case MOUSE_Y:
		return frameData->mouseY;
		break;
	case MOUSE_VELOCITY_X:
		return frameData->mouseVelocityX;
		break;
	case MOUSE_VELOCITY_Y:
		return frameData->mouseVelocityY;
		break;
	case SCROLL_VELOCITY_X:
		return frameData->scrollVelocityX;
		break;
	case SCROLL_VELOCITY_Y:
		return frameData->scrollVelocityY;
		break;
	}
	return 0;
}

int32_t DataProcessing::getExtraValueSpecific(FrameNum frame, ExtraValues extraValue, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	switch(extraValue) {
	case TOUCH_X_1:
		return getControllerDataExtra(savestateHookNum, branch, frame)->touchX1;
		break;
	case TOUCH_Y_1:
		return getControllerDataExtra(savestateHookNum, branch, frame)->touchY1;
		break;
	case TOUCH_X_2:
		return getControllerDataExtra(savestateHookNum, branch, frame)->touchX2;
		break;
	case TOUCH_Y_2:
		return getControllerDataExtra(savestateHookNum, branch, frame)->touchY2;
		break;
	case MOUSE_X:
		return getControllerDataExtra(savestateHookNum, branch, frame)->mouseX;
		break;
	case MOUSE_Y:
		return getControllerDataExtra(savestateHookNum, branch, frame)->mouseY;
		break;
	case MOUSE_VELOCITY_X:
		return getControllerDataExtra(savestateHookNum, branch, frame)->mouseVelocityX;
		break;
	case MOUSE_VELOCITY_Y:
		return getControllerDataExtra(savestateHookNum, branch, frame)->mouseVelocityY;
		break;
	case SCROLL_VELOCITY_X:
		return getControllerDataExtra(savestateHookNum, branch, frame)->scrollVelocityX;
		break;
	case SCROLL_VELOCITY_Y:
		return getControllerDataExtra(savestateHookNum, branch, frame)->scrollVelocityY;
		break;
	}
	return 0;
}

int32_t DataProcessing::getExtraValueCurrent(ExtraValues extraValue) const {
	return getExtraValue(currentFrame, extraValue);
}

void DataProcessing::triggerKeyboardButton(nn::hid::KeyboardKey key) {
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		uint8_t state         = !getKeyboardButton(currentFrame, key);
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setKeyboardButton(i, key, state);
		}
	}
}

void DataProcessing::setKeyboardButton(FrameNum frame, nn::hid::KeyboardKey key, uint8_t state) {
	SET_KEYBOARD_HELD(getInputsExtraList()->at(frame)->keyboardKeys, (int32_t)key, state);

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

int32_t DataProcessing::getKeyboardButton(FrameNum frame, nn::hid::KeyboardKey key) const {
	auto const& keys = getInputsExtraList()->at(frame)->keyboardKeys;
	int32_t isHeld   = IS_KEYBOARD_HELD(keys, (int32_t)key);
	return isHeld;
}

int32_t DataProcessing::getKeyboardButtonSpecific(FrameNum frame, nn::hid::KeyboardKey key, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return IS_KEYBOARD_HELD(getControllerDataExtra(savestateHookNum, branch, frame)->keyboardKeys, (int32_t)key);
}

int32_t DataProcessing::getKeyboardButtonCurrent(nn::hid::KeyboardKey key) const {
	return getKeyboardButton(currentFrame, key);
}

void DataProcessing::clearAllKeyboardButtons(FrameNum frame) {
	auto& keyboardKeys = getInputsExtraList()->at(frame)->keyboardKeys;
	memset(keyboardKeys, 0, sizeof(keyboardKeys));

	invalidateRun(frame);
	modifyCurrentFrameViews(frame);
	RefreshItem(frame);
}

void DataProcessing::triggerKeyboardModifier(nn::hid::KeyboardModifier key) {
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		uint8_t state         = !getKeyboardModifier(currentFrame, key);
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setKeyboardModifier(i, key, state);
		}
	}
}

void DataProcessing::setKeyboardModifier(FrameNum frame, nn::hid::KeyboardModifier key, uint8_t state) {
	auto& frameData = getInputsExtraList()->at(frame);
	if(state) {
		frameData->keyboardModifiers |= (int32_t)key;
	} else {
		frameData->keyboardModifiers &= ~((int32_t)key);
	}

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

int32_t DataProcessing::getKeyboardModifier(FrameNum frame, nn::hid::KeyboardModifier key) const {
	return getInputsExtraList()->at(frame)->keyboardModifiers & (int32_t)key;
}
int32_t DataProcessing::getKeyboardModifierSpecific(FrameNum frame, nn::hid::KeyboardModifier key, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return getControllerDataExtra(savestateHookNum, branch, frame)->keyboardModifiers & (int32_t)key;
}

int32_t DataProcessing::getKeyboardModifierCurrent(nn::hid::KeyboardModifier key) const {
	return getKeyboardModifier(currentFrame, key);
}

void DataProcessing::clearAllKeyboardModifiers(FrameNum frame) {
	getInputsExtraList()->at(frame)->keyboardModifiers = 0;

	invalidateRun(frame);
	modifyCurrentFrameViews(frame);
	RefreshItem(frame);
}

void DataProcessing::triggerMouseButton(nn::hid::MouseButton key) {
	long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(firstSelectedItem != wxNOT_FOUND) {
		long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
		uint8_t state         = !getMouseButton(currentFrame, key);
		for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
			setMouseButton(i, key, state);
		}
	}
}

void DataProcessing::setMouseButton(FrameNum frame, nn::hid::MouseButton key, uint8_t state) {
	auto& frameData = getInputsExtraList()->at(frame);
	if(state) {
		frameData->mouseButtons |= (int32_t)key;
	} else {
		frameData->mouseButtons &= ~((int32_t)key);
	}

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

int32_t DataProcessing::getMouseButton(FrameNum frame, nn::hid::MouseButton key) const {
	return getInputsExtraList()->at(frame)->mouseButtons & (int32_t)key;
}

int32_t DataProcessing::getMouseButtonSpecific(FrameNum frame, nn::hid::MouseButton key, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return getControllerDataExtra(savestateHookNum, branch, frame)->mouseButtons & (int32_t)key;
}

int32_t DataProcessing::getMouseButtonCurrent(nn::hid::MouseButton key) const {
	return getMouseButton(currentFrame, key);
}

void DataProcessing::clearAllMouseButtons(FrameNum frame) {
	getInputsExtraList()->at(frame)->mouseButtons = 0;

	invalidateRun(frame);
	modifyCurrentFrameViews(frame);
	RefreshItem(frame);
}

uint8_t DataProcessing::getButton(FrameNum frame, Btn button) const {
	return GET_BIT(getInputsList()->at(frame)->buttons, (uint8_t)button);
}

uint8_t DataProcessing::getButtonSpecific(FrameNum frame, Btn button, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return GET_BIT(getControllerData(player, savestateHookNum, branch, frame)->buttons, (uint8_t)button);
}

uint8_t DataProcessing::getButtonCurrent(Btn button) const {
	return getButton(currentFrame, button);
}

void DataProcessing::setControllerDataForAutoRun(ControllerData controllerData) {
	buttonData->transferControllerData(controllerData, getInputsList()->at(currentFrame), false);
	modifyCurrentFrameViews(currentFrame);
}

void DataProcessing::setExtraDataKeyboardForAutoRun(TouchAndKeyboardData extraData) {
	buttonData->transferOnlyKeyboard(extraData, getInputsExtraList()->at(currentFrame));
	modifyCurrentFrameViews(currentFrame);
}

void DataProcessing::setExtraDataTouchForAutoRun(TouchAndKeyboardData extraData) {
	buttonData->transferOnlyTouch(extraData, getInputsExtraList()->at(currentFrame));
	modifyCurrentFrameViews(currentFrame);
}

int16_t DataProcessing::getNumberValueCurrentJoystick(ControllerNumberValues joystickId) const {
	return getNumberValuesJoystick(currentFrame, joystickId);
}

float DataProcessing::getNumberValueCurrentMotion(ControllerNumberValues joystickId) const {
	return getNumberValuesMotion(currentFrame, joystickId);
}

// Updates how the current frame looks on the UI
// Also called when modifying anything of importance, like currentFrame
void DataProcessing::modifyCurrentFrameViews(FrameNum frame) {
	// Only update if it is the current frame, as this one has the focus
	if(frame == currentFrame) {
		// Refresh this item
		RefreshItem(currentFrame);
		// Refresh the grid
		if(changingSelectedFrameCallback) {
			changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
		}
		if(inputCallback) {
			inputCallback(true);
		}
	}
}

void DataProcessing::setFramestateInfo(FrameNum frame, FrameState id, uint8_t state) {
	SET_BIT(getInputsList()->at(frame)->frameState, state, (uint8_t)id);

	if(IsVisible(frame)) {
		RefreshItem(frame);
	}

	modifyCurrentFrameViews(frame);
}

void DataProcessing::setFramestateInfoSpecific(FrameNum frame, FrameState id, uint8_t state, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) {
	if(savestateHookNum == currentSavestateHook && player == viewingPlayerIndex) {
		setFramestateInfo(frame, id, state);
	} else {
		SET_BIT(getControllerData(player, savestateHookNum, branch, frame)->frameState, state, (uint8_t)id);
	}
}

uint8_t DataProcessing::getFramestateInfo(FrameNum frame, FrameState id) const {
	return GET_BIT(getInputsList()->at(frame)->frameState, (uint8_t)id);
}

uint8_t DataProcessing::getFramestateInfoSpecific(FrameNum frame, FrameState id, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return GET_BIT(getControllerData(player, savestateHookNum, branch, frame)->frameState, (uint8_t)id);
}

// Without the id, just return the whole hog
uint8_t DataProcessing::getFramestateInfo(FrameNum frame) const {
	return getInputsList()->at(frame)->frameState;
}

void DataProcessing::invalidateRun(FrameNum frame) {
	FrameNum size = getInputsList()->size();
	while(true) {
		if(frame == size || !getFramestateInfo(frame, FrameState::RAN)) {
			// Refresh all these items
			// I don't care if it's way off the page, I think wxWidgets handles for this
			Refresh();
			break;
		}
		// Set bit
		setFramestateInfo(frame, FrameState::RAN, false);
		// Also delete framebuffer from filesystem if neccessary
		wxFileName framebufferFileName = getFramebufferPath(viewingPlayerIndex, currentSavestateHook, viewingBranchIndex, frame);
		if(framebufferFileName.FileExists()) {
			// Delete file from filesystem
			wxRemoveFile(framebufferFileName.GetFullPath());
		}
		frame++;
	}
}

void DataProcessing::invalidateRunSpecific(FrameNum frame, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) {
	auto& list    = allPlayers[player]->at(savestateHookNum)->inputs[branch];
	FrameNum size = list->size();
	// TODO make this work with extra data too
	while(true) {
		if(frame == size || !getFramestateInfoSpecific(frame, FrameState::RAN, savestateHookNum, branch, player)) {
			// Refresh all these items
			// I don't care if it's way off the page, I think wxWidgets handles for this
			Refresh();
			break;
		}
		// Set bit
		setFramestateInfoSpecific(frame, FrameState::RAN, false, savestateHookNum, branch, player);
		// setFramestateInfoSpecific(frame, FrameState::SAVESTATE, false, savestateHookNum, branch, player);
		// Also delete framebuffer from filesystem if neccessary
		wxFileName framebufferFileName = getFramebufferPath(player, savestateHookNum, branch, frame);
		if(framebufferFileName.FileExists()) {
			// Delete file from filesystem
			wxRemoveFile(framebufferFileName.GetFullPath());
		}
		frame++;
	}
}

void DataProcessing::addFrame(FrameNum afterFrame) {
	// Add this to the vector right after the selected frame
	uint8_t playerIndex = 0;
	for(auto& player : allPlayers) {
		BranchNum branchIndex = 0;

		for(auto& branch : player->at(currentSavestateHook)->inputs) {
			std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();

			if(branch->size() == 0) {
				branch->push_back(newControllerData);
			} else {
				auto begin = branch->begin();
				branch->insert(begin + afterFrame + 1, newControllerData);
			}

			// Invalidate run for the data immidiently after this frame
			invalidateRunSpecific(afterFrame + 1, currentSavestateHook, branchIndex, playerIndex);

			branchIndex++;
		}

		playerIndex++;
	}

	BranchNum branchIndex = 0;

	for(auto& branch : *allExtraFrameData[currentSavestateHook]) {
		std::shared_ptr<TouchAndKeyboardData> newExtraData = std::make_shared<TouchAndKeyboardData>();

		if(branch->size() == 0) {
			branch->push_back(newExtraData);
		} else {
			auto begin = branch->begin();
			branch->insert(begin + afterFrame + 1, newExtraData);
		}

		// Invalidate run for the data immidiently after this frame
		// invalidateRunSpecific(afterFrame + 1, currentSavestateHook, branchIndex, playerIndex);

		branchIndex++;
	}

	// Because of the usability of virtual list controls, just update the length
	SetItemCount(getInputsList()->size());

	modifyCurrentFrameViews(afterFrame + 1);

	// Be very careful about refreshing, serious lag can happen if it's done wrong
	if(IsVisible(afterFrame + 1)) {
		Refresh();
	}
}

void DataProcessing::addFrameHere() {
	addFrame(currentFrame);
}

void DataProcessing::removeFrames(FrameNum start, FrameNum end) {
	// Since only selected frames will ever be selected, this just makes sure
	// One frame is left over
	if(end - start != (getFramesSize() - 1)) {
		uint8_t playerIndex = 0;
		for(auto& player : allPlayers) {
			BranchNum branchIndex = 0;
			for(auto& branch : player->at(currentSavestateHook)->inputs) {
				auto beginning = branch->begin();
				branch->erase(beginning + start, beginning + end + 1);

				// Invalidate run for the data immidiently after this frame
				invalidateRunSpecific(start, currentSavestateHook, branchIndex, playerIndex);

				branchIndex++;
			}

			playerIndex++;
		}

		BranchNum branchIndex = 0;
		for(auto& branch : *allExtraFrameData[currentSavestateHook]) {
			auto beginning = branch->begin();
			branch->erase(beginning + start, beginning + end + 1);

			// Invalidate run for the data immidiently after this frame
			// invalidateRunSpecific(start, currentSavestateHook, branchIndex, playerIndex);

			branchIndex++;
		}

		// Because of the usability of virtual list controls, just update the length
		SetItemCount(getInputsList()->size());

		if(currentFrame > (getFramesSize() - 1)) {
			setCurrentFrame(getFramesSize() - 1);
		} else {
			modifyCurrentFrameViews(currentFrame);
			Refresh();
		}
	}
}

std::size_t DataProcessing::getFramesSize() const {
	return getInputsList()->size();
}

DataProcessing::~DataProcessing() {
	// for(auto& itemAttribute : itemAttributes) {
	// Free the sucker
	// free(itemAttribute.second);
	//}
}
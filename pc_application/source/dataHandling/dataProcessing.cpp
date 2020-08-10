#include "dataProcessing.hpp"
#include "buttonData.hpp"

DataProcessing::DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent)
	: wxListCtrl(parent, DataProcessing::LIST_CTRL_ID, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES) {

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
	entries[7].Set(wxACCEL_CTRL, (int)'-', removeFrameID, editMenu.Append(removeFrameID, wxT("Remove Frame\tCtrl+Minus")));
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
}

// clang-format off
BEGIN_EVENT_TABLE(DataProcessing, wxListCtrl)
	EVT_LIST_CACHE_HINT(DataProcessing::LIST_CTRL_ID, DataProcessing::onCacheHint)
	EVT_LIST_ITEM_SELECTED(DataProcessing::LIST_CTRL_ID, DataProcessing::onSelect)
	// This is activated via double click
	EVT_LIST_ITEM_ACTIVATED(DataProcessing::LIST_CTRL_ID, DataProcessing::onActivate)
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

void DataProcessing::sendAutoAdvance(uint8_t includeFramebuffer) {
	for(uint8_t playerIndex = 0; playerIndex < allPlayers.size(); playerIndex++) {
		// Set inputs of all other players correctly but not the current one
		if(playerIndex != viewingPlayerIndex) {
			std::shared_ptr<ControllerData> controllerDatas = getControllerData(playerIndex, currentSavestateHook, viewingBranchIndex, currentRunFrame);

			ADD_TO_QUEUE(SendFrameData, networkInstance, {
				data.controllerData     = *controllerDatas;
				data.frame              = currentRunFrame;
				data.savestateHookNum   = currentSavestateHook;
				data.branchIndex        = viewingBranchIndex;
				data.playerIndex        = playerIndex;
				data.incrementFrame     = false;
				data.includeFramebuffer = includeFramebuffer;
				data.isAutoRun          = false;
			})
		}
	}

	ADD_TO_QUEUE(SendFrameData, networkInstance, {
		data.frame              = currentFrame + 1;
		data.savestateHookNum   = currentSavestateHook;
		data.branchIndex        = viewingBranchIndex;
		data.playerIndex        = viewingPlayerIndex;
		data.incrementFrame     = false;
		data.includeFramebuffer = includeFramebuffer;
		data.isAutoRun          = true;
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
		return wxString::Format(wxT("%ld"), row);
	} else {
		// Fallback for every other column
		return "";
	}
	// This function shouldn't receive any other column
}

// EXCUSE ME, WUT TODO
// Why can't I call a const method from a const method hmmm
wxItemAttr* DataProcessing::OnGetItemAttr(long item) const {
	return itemAttributes.at(getFramestateInfo(item));
}

void DataProcessing::setItemAttributes() {
	// Reuse both variables
	uint8_t state = 0;
	wxItemAttr* itemAttribute;

	// Default is nothing
	SET_BIT(state, false, FrameState::RAN);
	itemAttribute = new wxItemAttr();
	// itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["notRan"].GetString()));
	itemAttribute->SetBackgroundColour(GetBackgroundColour());
	itemAttributes[state] = itemAttribute;

	SET_BIT(state, true, FrameState::RAN);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["ran"].GetString()));
	itemAttributes[state] = itemAttribute;
	SET_BIT(state, false, FrameState::RAN);

	SET_BIT(state, true, FrameState::SAVESTATE);
	itemAttribute = new wxItemAttr();
	itemAttribute->SetBackgroundColour(wxColor((*mainSettings)["ui"]["frameViewerColors"]["savestate"].GetString()));
	itemAttributes[state] = itemAttribute;
	// SavestateHook takes precedence in the case where both are present
	SET_BIT(state, true, FrameState::RAN);
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
			// Transfer directly
			buttonData->transferControllerData(*(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(i)), allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[0]->at(i), false);
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
	SET_BIT(currentData->frameState, true, FrameState::SAVESTATE);
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

		if(currentRunFrame < allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->size()) {
			if(!forAutoFrame) {
				// Send to switch to run for each player
				for(uint8_t playerIndex = 0; playerIndex < allPlayers.size(); playerIndex++) {
					std::shared_ptr<ControllerData> controllerDatas = getControllerData(playerIndex, currentSavestateHook, viewingBranchIndex, currentRunFrame);
					ADD_TO_QUEUE(SendFrameData, networkInstance, {
						data.controllerData     = *controllerDatas;
						data.frame              = currentRunFrame;
						data.savestateHookNum   = currentSavestateHook;
						data.branchIndex        = viewingBranchIndex;
						data.playerIndex        = playerIndex;
						data.incrementFrame     = false;
						data.includeFramebuffer = includeFramebuffer;
						data.isAutoRun          = false;
					})
				}
				ADD_TO_QUEUE(SendFrameData, networkInstance, {
					data.frame              = currentRunFrame;
					data.savestateHookNum   = currentSavestateHook;
					data.branchIndex        = viewingBranchIndex;
					data.playerIndex        = viewingPlayerIndex;
					data.incrementFrame     = true;
					data.includeFramebuffer = includeFramebuffer;
					data.isAutoRun          = false;
				})
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
		allPlayers[i]->at(0)->inputs[0]->push_back(std::make_shared<ControllerData>());
		viewingBranchIndex = 0;
		// NOTE: There must be at least one block with one input when this is loaded
		// Automatically, the first block is always at index 0
		setSavestateHook(allPlayers[i]->size() - 1);
		setBranch(0);
	}
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
	return allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(frame);
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
	setBranch(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size() - 1);
}

void DataProcessing::setBranch(uint16_t branchIndex) {
	viewingBranchIndex = branchIndex;
	currentBranchData  = allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex];
	if(branchInfoCallback) {
		branchInfoCallback(getNumBranches(), branchIndex, true);
	}
	modifyCurrentFrameViews(currentFrame);
	Refresh();
}

void DataProcessing::removeBranch(uint8_t branchIndex) {
	if(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size() > 1) {
		allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.erase(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.begin() + branchIndex);
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
	SET_BIT(allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(frame)->buttons, isPressed, button);

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
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		getInputsList()->at(frame)->LS_X = value;
		break;
	case ControllerNumberValues::LEFT_Y:
		getInputsList()->at(frame)->LS_Y = value;
		break;
	case ControllerNumberValues::RIGHT_X:
		getInputsList()->at(frame)->RS_X = value;
		break;
	case ControllerNumberValues::RIGHT_Y:
		getInputsList()->at(frame)->RS_Y = value;
		break;
	case ControllerNumberValues::TOUCH_X_1:
		getInputsList()->at(frame)->TOUCH_X_1 = value;
		break;
	case ControllerNumberValues::TOUCH_Y_1:
		getInputsList()->at(frame)->TOUCH_Y_1 = value;
		break;
	case ControllerNumberValues::TOUCH_X_2:
		getInputsList()->at(frame)->TOUCH_X_2 = value;
		break;
	case ControllerNumberValues::TOUCH_Y_2:
		getInputsList()->at(frame)->TOUCH_Y_2 = value;
		break;
	case ControllerNumberValues::ACCEL_X_LEFT:
	case ControllerNumberValues::ACCEL_Y_LEFT:
	case ControllerNumberValues::ACCEL_Z_LEFT:
	case ControllerNumberValues::GYRO_X_LEFT:
	case ControllerNumberValues::GYRO_Y_LEFT:
	case ControllerNumberValues::GYRO_Z_LEFT:
	case ControllerNumberValues::ANGLE_X_LEFT:
	case ControllerNumberValues::ANGLE_Y_LEFT:
	case ControllerNumberValues::ANGLE_Z_LEFT:
	case ControllerNumberValues::ACCEL_X_RIGHT:
	case ControllerNumberValues::ACCEL_Y_RIGHT:
	case ControllerNumberValues::ACCEL_Z_RIGHT:
	case ControllerNumberValues::GYRO_X_RIGHT:
	case ControllerNumberValues::GYRO_Y_RIGHT:
	case ControllerNumberValues::GYRO_Z_RIGHT:
	case ControllerNumberValues::ANGLE_X_RIGHT:
	case ControllerNumberValues::ANGLE_Y_RIGHT:
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		break;
	}

	modifyCurrentFrameViews(frame);
	invalidateRun(frame);
}

void DataProcessing::setNumberValuesMotion(FrameNum frame, ControllerNumberValues joystickId, float value) {
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		getInputsList()->at(frame)->ACCEL_X_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		getInputsList()->at(frame)->ACCEL_Y_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		getInputsList()->at(frame)->ACCEL_Z_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		getInputsList()->at(frame)->GYRO_X_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		getInputsList()->at(frame)->GYRO_Y_LEFT = value;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		getInputsList()->at(frame)->GYRO_Z_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		getInputsList()->at(frame)->ANGLE_X_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		getInputsList()->at(frame)->ANGLE_Y_LEFT = value;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		getInputsList()->at(frame)->ANGLE_Z_LEFT = value;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		getInputsList()->at(frame)->ACCEL_X_RIGHT = value;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		getInputsList()->at(frame)->ACCEL_Y_RIGHT = value;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		getInputsList()->at(frame)->ACCEL_Z_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		getInputsList()->at(frame)->GYRO_X_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		getInputsList()->at(frame)->GYRO_Y_RIGHT = value;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		getInputsList()->at(frame)->GYRO_Z_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		getInputsList()->at(frame)->ANGLE_X_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		getInputsList()->at(frame)->ANGLE_Y_RIGHT = value;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		getInputsList()->at(frame)->ANGLE_Z_RIGHT = value;
		break;
	case ControllerNumberValues::LEFT_X:
	case ControllerNumberValues::LEFT_Y:
	case ControllerNumberValues::RIGHT_X:
	case ControllerNumberValues::RIGHT_Y:
	case ControllerNumberValues::TOUCH_X_1:
	case ControllerNumberValues::TOUCH_Y_1:
	case ControllerNumberValues::TOUCH_X_2:
	case ControllerNumberValues::TOUCH_Y_2:
		break;
	}

	modifyCurrentFrameViews(frame);
	invalidateRun(frame);
}

int16_t DataProcessing::getNumberValuesJoystick(FrameNum frame, ControllerNumberValues joystickId) const {
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		return getInputsList()->at(frame)->LS_X;
		break;
	case ControllerNumberValues::LEFT_Y:
		return getInputsList()->at(frame)->LS_Y;
		break;
	case ControllerNumberValues::RIGHT_X:
		return getInputsList()->at(frame)->RS_X;
		break;
	case ControllerNumberValues::RIGHT_Y:
		return getInputsList()->at(frame)->RS_Y;
		break;
	case ControllerNumberValues::TOUCH_X_1:
		return getInputsList()->at(frame)->TOUCH_X_1;
		break;
	case ControllerNumberValues::TOUCH_Y_1:
		return getInputsList()->at(frame)->TOUCH_Y_1;
		break;
	case ControllerNumberValues::TOUCH_X_2:
		return getInputsList()->at(frame)->TOUCH_X_2;
		break;
	case ControllerNumberValues::TOUCH_Y_2:
		return getInputsList()->at(frame)->TOUCH_Y_2;
		break;
	case ControllerNumberValues::ACCEL_X_LEFT:
	case ControllerNumberValues::ACCEL_Y_LEFT:
	case ControllerNumberValues::ACCEL_Z_LEFT:
	case ControllerNumberValues::GYRO_X_LEFT:
	case ControllerNumberValues::GYRO_Y_LEFT:
	case ControllerNumberValues::GYRO_Z_LEFT:
	case ControllerNumberValues::ANGLE_X_LEFT:
	case ControllerNumberValues::ANGLE_Y_LEFT:
	case ControllerNumberValues::ANGLE_Z_LEFT:
	case ControllerNumberValues::ACCEL_X_RIGHT:
	case ControllerNumberValues::ACCEL_Y_RIGHT:
	case ControllerNumberValues::ACCEL_Z_RIGHT:
	case ControllerNumberValues::GYRO_X_RIGHT:
	case ControllerNumberValues::GYRO_Y_RIGHT:
	case ControllerNumberValues::GYRO_Z_RIGHT:
	case ControllerNumberValues::ANGLE_X_RIGHT:
	case ControllerNumberValues::ANGLE_Y_RIGHT:
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		break;
	}
}

float DataProcessing::getNumberValuesMotion(FrameNum frame, ControllerNumberValues joystickId) const {
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		return getInputsList()->at(frame)->ACCEL_X_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		return getInputsList()->at(frame)->ACCEL_Y_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		return getInputsList()->at(frame)->ACCEL_Z_LEFT;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		return getInputsList()->at(frame)->GYRO_X_LEFT;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		return getInputsList()->at(frame)->GYRO_Y_LEFT;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		return getInputsList()->at(frame)->GYRO_Z_LEFT;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		return getInputsList()->at(frame)->ANGLE_X_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		return getInputsList()->at(frame)->ANGLE_Y_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		return getInputsList()->at(frame)->ANGLE_Z_LEFT;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		return getInputsList()->at(frame)->ACCEL_X_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		return getInputsList()->at(frame)->ACCEL_Y_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		return getInputsList()->at(frame)->ACCEL_Z_RIGHT;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		return getInputsList()->at(frame)->GYRO_X_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		return getInputsList()->at(frame)->GYRO_Y_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		return getInputsList()->at(frame)->GYRO_Z_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		return getInputsList()->at(frame)->ANGLE_X_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		return getInputsList()->at(frame)->ANGLE_Y_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		return getInputsList()->at(frame)->ANGLE_Z_RIGHT;
		break;
	case ControllerNumberValues::LEFT_X:
	case ControllerNumberValues::LEFT_Y:
	case ControllerNumberValues::RIGHT_X:
	case ControllerNumberValues::RIGHT_Y:
	case ControllerNumberValues::TOUCH_X_1:
	case ControllerNumberValues::TOUCH_Y_1:
	case ControllerNumberValues::TOUCH_X_2:
	case ControllerNumberValues::TOUCH_Y_2:
		break;
	}
}

int16_t DataProcessing::getNumberValuesSpecificJoystick(FrameNum frame, ControllerNumberValues joystickId, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	switch(joystickId) {
	case ControllerNumberValues::LEFT_X:
		return getControllerData(player, savestateHookNum, branch, frame)->LS_X;
		break;
	case ControllerNumberValues::LEFT_Y:
		return getControllerData(player, savestateHookNum, branch, frame)->LS_Y;
		break;
	case ControllerNumberValues::RIGHT_X:
		return getControllerData(player, savestateHookNum, branch, frame)->RS_X;
		break;
	case ControllerNumberValues::RIGHT_Y:
		return getControllerData(player, savestateHookNum, branch, frame)->RS_Y;
		break;
	case ControllerNumberValues::TOUCH_X_1:
		return getControllerData(player, savestateHookNum, branch, frame)->TOUCH_X_1;
		break;
	case ControllerNumberValues::TOUCH_Y_1:
		return getControllerData(player, savestateHookNum, branch, frame)->TOUCH_Y_1;
		break;
	case ControllerNumberValues::TOUCH_X_2:
		return getControllerData(player, savestateHookNum, branch, frame)->TOUCH_X_2;
		break;
	case ControllerNumberValues::TOUCH_Y_2:
		return getControllerData(player, savestateHookNum, branch, frame)->TOUCH_Y_2;
		break;
	case ControllerNumberValues::ACCEL_X_LEFT:
	case ControllerNumberValues::ACCEL_Y_LEFT:
	case ControllerNumberValues::ACCEL_Z_LEFT:
	case ControllerNumberValues::GYRO_X_LEFT:
	case ControllerNumberValues::GYRO_Y_LEFT:
	case ControllerNumberValues::GYRO_Z_LEFT:
	case ControllerNumberValues::ANGLE_X_LEFT:
	case ControllerNumberValues::ANGLE_Y_LEFT:
	case ControllerNumberValues::ANGLE_Z_LEFT:
	case ControllerNumberValues::ACCEL_X_RIGHT:
	case ControllerNumberValues::ACCEL_Y_RIGHT:
	case ControllerNumberValues::ACCEL_Z_RIGHT:
	case ControllerNumberValues::GYRO_X_RIGHT:
	case ControllerNumberValues::GYRO_Y_RIGHT:
	case ControllerNumberValues::GYRO_Z_RIGHT:
	case ControllerNumberValues::ANGLE_X_RIGHT:
	case ControllerNumberValues::ANGLE_Y_RIGHT:
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		break;
	}
}

float DataProcessing::getNumberValuesSpecificMotion(FrameNum frame, ControllerNumberValues joystickId, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	switch(joystickId) {
	case ControllerNumberValues::ACCEL_X_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_X_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Y_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_Y_LEFT;
		break;
	case ControllerNumberValues::ACCEL_Z_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_Z_LEFT;
		break;
	case ControllerNumberValues::GYRO_X_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_X_LEFT;
		break;
	case ControllerNumberValues::GYRO_Y_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_Y_LEFT;
		break;
	case ControllerNumberValues::GYRO_Z_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_Z_LEFT;
		break;
	case ControllerNumberValues::ANGLE_X_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_X_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Y_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_Y_LEFT;
		break;
	case ControllerNumberValues::ANGLE_Z_LEFT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_Z_LEFT;
		break;
	case ControllerNumberValues::ACCEL_X_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_X_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Y_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_Y_RIGHT;
		break;
	case ControllerNumberValues::ACCEL_Z_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ACCEL_Z_RIGHT;
		break;
	case ControllerNumberValues::GYRO_X_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_X_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Y_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_Y_RIGHT;
		break;
	case ControllerNumberValues::GYRO_Z_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->GYRO_Z_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_X_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_X_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Y_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_Y_RIGHT;
		break;
	case ControllerNumberValues::ANGLE_Z_RIGHT:
		return getControllerData(player, savestateHookNum, branch, frame)->ANGLE_Z_RIGHT;
		break;
	case ControllerNumberValues::LEFT_X:
	case ControllerNumberValues::LEFT_Y:
	case ControllerNumberValues::RIGHT_X:
	case ControllerNumberValues::RIGHT_Y:
	case ControllerNumberValues::TOUCH_X_1:
	case ControllerNumberValues::TOUCH_Y_1:
	case ControllerNumberValues::TOUCH_X_2:
	case ControllerNumberValues::TOUCH_Y_2:
		break;
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
	allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs[viewingBranchIndex]->at(frame)->numberOfTouches = value;

	invalidateRun(frame);

	RefreshItem(frame);
	modifyCurrentFrameViews(frame);
}

uint8_t DataProcessing::getNumberOfTouches(FrameNum frame) const {
	return getInputsList()->at(frame)->numberOfTouches;
}

uint8_t DataProcessing::getNumberOfTouchesSpecific(FrameNum frame, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return getControllerData(player, savestateHookNum, branch, frame)->numberOfTouches;
}

uint8_t DataProcessing::getNumberOfTouchesCurrent() const {
	return getNumberOfTouches(currentFrame);
}

uint8_t DataProcessing::getButton(FrameNum frame, Btn button) const {
	return GET_BIT(getInputsList()->at(frame)->buttons, button);
}

uint8_t DataProcessing::getButtonSpecific(FrameNum frame, Btn button, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return GET_BIT(getControllerData(player, savestateHookNum, branch, frame)->buttons, button);
}

uint8_t DataProcessing::getButtonCurrent(Btn button) const {
	return getButton(currentFrame, button);
}

void DataProcessing::setControllerDataForAutoRun(ControllerData controllerData) {
	// Set controller data manually
	std::shared_ptr<ControllerData> newData = std::make_shared<ControllerData>();
	buttonData->transferControllerData(controllerData, newData, false);
	getInputsList()->at(currentFrame) = newData;
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
	SET_BIT(getInputsList()->at(frame)->frameState, state, id);

	if(IsVisible(frame)) {
		RefreshItem(frame);
	}

	modifyCurrentFrameViews(frame);
}

void DataProcessing::setFramestateInfoSpecific(FrameNum frame, FrameState id, uint8_t state, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) {
	if(savestateHookNum == currentSavestateHook && player == viewingPlayerIndex) {
		setFramestateInfo(frame, id, state);
	} else {
		SET_BIT(getControllerData(player, savestateHookNum, branch, frame)->frameState, state, id);
	}
}

uint8_t DataProcessing::getFramestateInfo(FrameNum frame, FrameState id) const {
	return GET_BIT(getInputsList()->at(frame)->frameState, id);
}

uint8_t DataProcessing::getFramestateInfoSpecific(FrameNum frame, FrameState id, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const {
	return GET_BIT(getControllerData(player, savestateHookNum, branch, frame)->frameState, id);
}

// Without the id, just return the whole hog
uint8_t DataProcessing::getFramestateInfo(FrameNum frame) const {
	return getInputsList()->at(frame)->frameState;
}

void DataProcessing::invalidateRun(FrameNum frame) {
	while(true) {
		if(frame == getInputsList()->size() || !getFramestateInfo(frame, FrameState::RAN)) {
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
	auto& list       = allPlayers[player]->at(savestateHookNum)->inputs[branch];
	std::size_t size = list->size();
	while(true) {
		if(frame == size || !getFramestateInfoSpecific(frame, FrameState::RAN, savestateHookNum, branch, player)) {
			// Refresh all these items
			// I don't care if it's way off the page, I think wxWidgets handles for this
			Refresh();
			break;
		} else {
			// Set bit
			setFramestateInfoSpecific(frame, FrameState::RAN, false, savestateHookNum, branch, player);
			setFramestateInfoSpecific(frame, FrameState::SAVESTATE, false, savestateHookNum, branch, player);
			frame++;
		}
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
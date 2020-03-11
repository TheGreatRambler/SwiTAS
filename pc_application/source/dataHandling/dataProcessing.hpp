#pragma once

#include <Windows.h>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <string>
#include <tuple>
#include <utility>
#include <uxtheme.h>
#include <vector>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/wx.h>

#include "../networking/networkInterface.hpp"
#include "buttonData.hpp"

class DataProcessing : public wxListCtrl {
	// clang-format on
private:
	// Vector storing ALL inputs
	// Shared pointer so everything is nice
	std::vector<std::shared_ptr<ControllerData>> inputsList;
	// Current input
	std::shared_ptr<ControllerData> currentData;
	// Button data
	std::shared_ptr<ButtonData> buttonData;

	// Current frames
	// What you can edit
	FrameNum currentFrame;
	// What is the state of the switch
	FrameNum currentRunFrame;
	// What is the image you can see
	FrameNum currentImageFrame;

	wxImageList imageList;

	// Using callbacks for inputs
	std::function<void(Btn, bool)> inputCallback;
	std::function<void(FrameNum, FrameNum)> viewableInputsCallback;
	std::function<void(FrameNum, FrameNum, FrameNum)> changingSelectedFrameCallback;

	// Network instance for sending to switch
	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	// Main settings
	rapidjson::Document* mainSettings;

	// Mask color for transparency
	wxColour maskColor;

	// Map to get column
	std::unordered_map<Btn, uint8_t> buttonToColumn;
	std::unordered_map<wxChar, Btn> charToButton;

	// Probably not smart, but the current savestate hooks
	std::unordered_map<FrameNum, std::shared_ptr<SavestateHook>> savestateHooks;
	// Universal item attributes for certain attributes
	std::unordered_map<uint8_t, wxItemAttr*> itemAttributes;

	virtual int OnGetItemColumnImage(long item, long column) const override;
	virtual wxString OnGetItemText(long item, long column) const override;
	virtual wxItemAttr* OnGetItemAttr(long item) const override;

	void setItemAttributes();

	void OnEraseBackground(wxEraseEvent& event);

	std::vector<std::shared_ptr<ControllerData>> framesCopied;
	int pasteInsertID;
	int pastePlaceID;
	bool placePaste;

	// Menu popup
	wxMenu editMenu;

	void onRightClick(wxContextMenuEvent& event);
	void onSelect(wxListEvent& event);
	void onActivate(wxListEvent& event);
	void onCopy(wxCommandEvent& event);
	void onCut(wxCommandEvent& event);
	void onPaste(wxCommandEvent& event);
	void onInsertPaste(wxCommandEvent& event);
	void onPlacePaste(wxCommandEvent& event);

public:
	static const int LIST_CTRL_ID = 1000;

	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent);

	void setInputCallback(std::function<void(Btn, bool)> callback);

	void setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback);
	void setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback);
	void triggerCurrentFrameChanges() {
		if(changingSelectedFrameCallback) {
			changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
		}
	}

	bool getButtonState(Btn button);

	void setButtonState(Btn button, bool state);

	void toggleButtonState(Btn button);

	void setCurrentFrame(FrameNum frameNum);

	void addNewFrame();
	void createSavestateHookHere();
	void runFrame();

	wxRect getFirstItemRect() {
		wxRect itemRect;
		long topItem = GetTopItem();
		GetItemRect(topItem, itemRect);
		return itemRect;
	}

	bool handleKeyboardInput(wxChar key);
	// The class itself is the list control
	// std::shared_ptr<wxGenericListCtrl> getWidget();

	void onCacheHint(wxListEvent& event) {
		if(viewableInputsCallback) {
			long numOfRowsVisible = GetCountPerPage();
			if(numOfRowsVisible != 0) {
				// Don't use the event values, they are wrong
				long first = GetTopItem();
				long last  = first + numOfRowsVisible;

				viewableInputsCallback(first, last);
			}
		}
	}

	~DataProcessing();

	DECLARE_EVENT_TABLE();
};
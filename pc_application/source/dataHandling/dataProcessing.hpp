#pragma once

#DEFINE SET_BIT(number, bit, loc) number ^= (-(unsigned long)bit ^ number) & (1UL << loc);

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
	// Current frame
	uint32_t currentFrame;
	// Tree data storing the controller stuffs
	// Use generic because it reports scroll events
	wxImageList imageList;
	// Using callbacks for inputs
	std::function<void(Btn, bool)> inputCallback;
	std::function<void(uint32_t, uint32_t)> viewableInputsCallback;
	// Network instance for sending to switch
	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	// Main settings
	rapidjson::Document* mainSettings;
	// Mask color for transparency
	wxColour maskColor;
	// Map to get column
	std::unordered_map<Btn, uint8_t> buttonToColumn;
	std::unordered_map<wxChar, Btn> charToButton;
	virtual int OnGetItemColumnImage(long item, long column) const override;
	virtual wxString OnGetItemText(long item, long column) const override;

public:
	static const int LIST_CTRL_ID = 1000;

	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent);

	void setInputCallback(std::function<void(Btn, bool)> callback);

	void setViewableInputsCallback(std::function<void(uint32_t, uint32_t)> callback);

	bool getButtonState(Btn button);

	void setButtonState(Btn button, bool state);

	void toggleButtonState(Btn button);

	void setCurrentFrame(uint32_t frameNum);

	void addNewFrame();

	wxRect getFirstItemRect() {
		wxRect itemRect;
		GetItemRect(GetTopItem() - 1, itemRect);
		return itemRect;
	}

	void handleKeyboardInput(wxChar key);
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

	void OnEraseBackground(wxEraseEvent& event);

	DECLARE_EVENT_TABLE();
};
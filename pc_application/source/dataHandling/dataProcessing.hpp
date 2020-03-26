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
#include <wx/clipbrd.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../ui/savestateSelection.hpp"
#include "buttonData.hpp"

typedef std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>> SavestateHookBlock;
typedef std::shared_ptr<ControllerData> FrameData;
typedef std::vector<std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>>> AllSavestateHookBlocks;

class DataProcessing : public wxListCtrl {
	// clang-format on
private:
	// Vector storing inputs for current savestate hook
	SavestateHookBlock inputsList;
	// Current input
	FrameData currentData;
	// Button data instance (never changes)
	std::shared_ptr<ButtonData> buttonData;
	// Vector holding the savestate hook blocks
	AllSavestateHookBlocks savestateHookBlocks;

	// Current frames (all relative to the start of the savestate hook block)
	// What you can edit
	FrameNum currentFrame;
	// What is the state of the switch
	FrameNum currentRunFrame;
	// What is the image you can see
	FrameNum currentImageFrame;

	wxImageList imageList;

	// First savestate hook creation, then savestate loader
	SavestateSelection* savestateHookCreator;
	SavestateSelection* savestateHookLoader;

	// Using callbacks for inputs
	std::function<void()> inputCallback;
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

	// Custom accelerator IDs
	int pasteInsertID;
	int pastePlaceID;
	int addFrameID;
	int frameAdvanceID;
	int savestateHookID;

	int insertPaste;
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

	void onAddFrame(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onAddSavestateHook(wxCommandEvent& event);

public:
	static const int LIST_CTRL_ID = 1000;

	// All blocks loaded in by projectManager
	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent, AllSavestateHookBlocks allBlocks);

	void setInputCallback(std::function<void()> callback);

	void setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback);
	void setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback);
	void triggerCurrentFrameChanges() {
		if(changingSelectedFrameCallback) {
			changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
		}
	}

	void setCurrentFrame(FrameNum frameNum);

	void addNewFrame();
	void createSavestateHookHere();
	void runFrame();

	std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>> getInputsList() {
		return inputsList;
	}

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

	const std::shared_ptr<ControllerData> getFrame(FrameNum frame) {
		return inputsList->at(frame);
	}

	void triggerButton(Btn button) {
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
	void triggerNumberValues(ControllerNumberValues joystickId, int32_t value) {
		// Trigger joystick, can occur over range
		long firstSelectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if(firstSelectedItem != wxNOT_FOUND) {
			long lastSelectedItem = firstSelectedItem + GetSelectedItemCount() - 1;
			for(FrameNum i = firstSelectedItem; i <= lastSelectedItem; i++) {
				setNumberValues(i, joystickId, value);
				// No refresh for now, as the joystick is not visible in the inputsList
			}
		}
	}

	// New FANCY methods
	void modifyButton(FrameNum frame, Btn button, uint8_t isPressed) {
		SET_BIT(inputsList->at(frame)->buttons, isPressed, button);

		invalidateRun(frame);
		modifyCurrentFrameViews(frame);
		RefreshItem(frame);
	}

	void toggleButton(FrameNum frame, Btn button) {
		modifyButton(frame, button, !getButton(frame, button));
	}

	void clearAllButtons(FrameNum frame) {
		// I think this works
		inputsList->at(frame)->buttons = 0;

		invalidateRun(frame);
		modifyCurrentFrameViews(frame);
		RefreshItem(frame);
	}

	void setNumberValues(FrameNum frame, ControllerNumberValues joystickId, int32_t value) {
		switch(joystickId) {
		case ControllerNumberValues::LEFT_X:
			inputsList->at(frame)->LS_X = value;
			break;
		case ControllerNumberValues::LEFT_Y:
			inputsList->at(frame)->LS_Y = value;
			break;
		case ControllerNumberValues::RIGHT_X:
			inputsList->at(frame)->RS_X = value;
			break;
		case ControllerNumberValues::RIGHT_Y:
			inputsList->at(frame)->RS_Y = value;
			break;
		case ControllerNumberValues::ACCEL_X:
			inputsList->at(frame)->ACCEL_X = value;
			break;
		case ControllerNumberValues::ACCEL_Y:
			inputsList->at(frame)->ACCEL_Y = value;
			break;
		case ControllerNumberValues::ACCEL_Z:
			inputsList->at(frame)->ACCEL_Z = value;
			break;
		case ControllerNumberValues::GYRO_1:
			inputsList->at(frame)->GYRO_1 = value;
			break;
		case ControllerNumberValues::GYRO_2:
			inputsList->at(frame)->GYRO_2 = value;
			break;
		case ControllerNumberValues::GYRO_3:
			inputsList->at(frame)->GYRO_3 = value;
			break;
		}

		modifyCurrentFrameViews(frame);

		invalidateRun(frame);
	}

	const int32_t getNumberValues(FrameNum frame, ControllerNumberValues joystickId) {
		switch(joystickId) {
		case ControllerNumberValues::LEFT_X:
			return inputsList->at(frame)->LS_X;
			break;
		case ControllerNumberValues::LEFT_Y:
			return inputsList->at(frame)->LS_Y;
			break;
		case ControllerNumberValues::RIGHT_X:
			return inputsList->at(frame)->RS_X;
			break;
		case ControllerNumberValues::RIGHT_Y:
			return inputsList->at(frame)->RS_Y;
			break;
		case ControllerNumberValues::ACCEL_X:
			return inputsList->at(frame)->ACCEL_X;
			break;
		case ControllerNumberValues::ACCEL_Y:
			return inputsList->at(frame)->ACCEL_Y;
			break;
		case ControllerNumberValues::ACCEL_Z:
			return inputsList->at(frame)->ACCEL_Z;
			break;
		case ControllerNumberValues::GYRO_1:
			return inputsList->at(frame)->GYRO_1;
			break;
		case ControllerNumberValues::GYRO_2:
			return inputsList->at(frame)->GYRO_2;
			break;
		case ControllerNumberValues::GYRO_3:
			return inputsList->at(frame)->GYRO_3;
			break;
		}
	}

	const uint8_t getButton(FrameNum frame, Btn button) {
		return GET_BIT(inputsList->at(frame)->buttons, button);
	}

	const uint8_t getButtonCurrent(Btn button) {
		return getButton(currentFrame, button);
	}

	const int32_t getNumberValueCurrent(ControllerNumberValues joystickId) {
		return getNumberValues(currentFrame, joystickId);
	}

	// Updates how the current frame looks on the UI
	// Also called when modifying anything of importance, like currentFrame
	void modifyCurrentFrameViews(FrameNum frame) {
		// Only update if it is the current frame, as this one has the focus
		if(frame == currentFrame) {
			// Refresh this item
			RefreshItem(currentFrame);
			// Refresh the grid
			if(changingSelectedFrameCallback) {
				changingSelectedFrameCallback(currentFrame, currentRunFrame, currentImageFrame);
			}
			if(inputCallback) {
				// Doesn't matter what arguments
				inputCallback();
			}
		}
	}

	void setFramestateInfo(FrameNum frame, FrameState id, uint8_t state) {
		SET_BIT(inputsList->at(frame)->frameState, state, id);

		RefreshItem(frame);

		modifyCurrentFrameViews(frame);
	}

	const uint8_t getFramestateInfo(FrameNum frame, FrameState id) {
		return GET_BIT(inputsList->at(frame)->frameState, id);
	}

	// Without the id, just return the whole hog
	const uint8_t getFramestateInfo(FrameNum frame) {
		return inputsList->at(frame)->frameState;
	}

	void invalidateRun(FrameNum frame) {
		while(true) {
			if(frame == inputsList->size() || !getFramestateInfo(frame, FrameState::RAN)) {
				// Refresh all these items
				// I don't care if it's way off the page, I think wxWidgets handles for this
				Refresh();
				break;
			}
			// Set bit
			setFramestateInfo(frame, FrameState::RAN, false);
			frame++;
		}
	}

	void addFrame(FrameNum afterFrame) {
		std::shared_ptr<ControllerData> newControllerData = std::make_shared<ControllerData>();

		// Add this to the vector right after the selected frame
		if(inputsList->size() == 0) {
			inputsList->push_back(newControllerData);
		} else {
			inputsList->insert(inputsList->begin() + afterFrame + 1, newControllerData);
		}

		setFramestateInfo(afterFrame + 1, FrameState::RAN, false);
		setFramestateInfo(afterFrame + 1, FrameState::SAVESTATE_HOOK, false);

		// Because of the usability of virtual list controls, just update the length
		SetItemCount(inputsList->size());

		// Invalidate run for the data immidiently after this frame
		invalidateRun(afterFrame + 2);

		modifyCurrentFrameViews(afterFrame + 1);

		// Be very careful about refreshing, serious lag can happen if it's done wrong
		if(IsVisible(afterFrame + 1)) {
			Refresh();
		}
	}

	void removeFrames(FrameNum start, FrameNum end) {
		auto beginning = inputsList->begin();
		inputsList->erase(beginning + start, beginning + end + 1);

		// Because of the usability of virtual list controls, just update the length
		SetItemCount(inputsList->size());

		// Invalidate run for the data immidiently after this frame
		invalidateRun(start);

		for(FrameNum s = start; s <= end; s++) {
			modifyCurrentFrameViews(s);
		}

		Refresh();
	}

	const std::size_t getFramesSize() {
		return inputsList->size();
	}

	~DataProcessing();

	DECLARE_EVENT_TABLE();
};
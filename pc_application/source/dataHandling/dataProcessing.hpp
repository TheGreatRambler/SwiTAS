#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <uxtheme.h>
#endif
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
#include <vector>
#include <wx/clipbrd.h>
#include <wx/grid.h>
#include <wx/imaglist.h>
#include <wx/itemattr.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/wx.h>

#include "../../sharedNetworkCode/networkInterface.hpp"
#include "../ui/savestateSelection.hpp"
#include "buttonConstants.hpp"
#include "buttonData.hpp"

typedef std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>> SavestateHookBlock;
typedef std::shared_ptr<ControllerData> FrameData;
typedef std::vector<std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>>> AllSavestateHookBlocks;

class ButtonData;

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

	// Using callbacks for inputs
	std::function<void()> inputCallback;
	std::function<void(FrameNum)> selectedFrameCallbackVideoViewer;
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
	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent);

	void setInputCallback(std::function<void()> callback);
	void setSelectedFrameCallbackVideoViewer(std::function<void(FrameNum)> callback);
	void setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback);
	void setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback);
	void triggerCurrentFrameChanges();

	AllSavestateHookBlocks& getAllSavestateHookBlocks() {
		return savestateHookBlocks;
	}

	void setAllSavestateHookBlocks(AllSavestateHookBlocks blocks) {
		// Called by projectHandler when loading
		// It has to have at least one block with one input
		savestateHookBlocks = blocks;
		setSavestateHook(0);
	}

	void modifySavestate() {}

	void setCurrentFrame(FrameNum frameNum);

	void createSavestateHookHere();
	void runFrame();

	std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>> getInputsList();

	wxRect getFirstItemRect();

	bool handleKeyboardInput(wxChar key);
	// The class itself is the list control
	// std::shared_ptr<wxGenericListCtrl> getWidget();

	void onCacheHint(wxListEvent& event);

	void addNewSavestateHook();

	void setSavestateHook(SavestateBlockNum index);

	std::shared_ptr<ControllerData> getFrame(FrameNum frame) const;

	void triggerButton(Btn button);

	// This includes joysticks, accel, gyro, etc...
	void triggerNumberValues(ControllerNumberValues joystickId, int32_t value);

	// New FANCY methods
	void modifyButton(FrameNum frame, Btn button, uint8_t isPressed);

	void toggleButton(FrameNum frame, Btn button);

	void clearAllButtons(FrameNum frame);

	void setNumberValues(FrameNum frame, ControllerNumberValues joystickId, int32_t value);

	int32_t getNumberValues(FrameNum frame, ControllerNumberValues joystickId) const;

	uint8_t getButton(FrameNum frame, Btn button) const;

	uint8_t getButtonCurrent(Btn button) const;

	int32_t getNumberValueCurrent(ControllerNumberValues joystickId) const;

	// Updates how the current frame looks on the UI
	// Also called when modifying anything of importance, like currentFrame
	void modifyCurrentFrameViews(FrameNum frame);

	void setFramestateInfo(FrameNum frame, FrameState id, uint8_t state);

	uint8_t getFramestateInfo(FrameNum frame, FrameState id) const;

	// Without the id, just return the whole hog
	uint8_t getFramestateInfo(FrameNum frame) const;

	void invalidateRun(FrameNum frame);

	void addFrame(FrameNum afterFrame);

	void addFrameHere();

	void removeFrames(FrameNum start, FrameNum end);

	std::size_t getFramesSize() const;

	~DataProcessing();

	DECLARE_EVENT_TABLE();
};
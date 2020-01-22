#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <wx/listctrl.h>
#include <wx/wx.h>

#include "buttonData.hpp"

class DataProcessing : public wxListCtrl {
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
	// Tree view viewed in the UI
	// Gtk::TreeView treeView;
	// Scrollable data window
	// std::shared_ptr<Gtk::ScrolledWindow> scrolledWindow;
	// Using callbacks for inputs
	// std::function<void(Btn, bool)> inputCallback;
	virtual int OnGetItemColumnImage(long item, long column) const;
	virtual wxString OnGetItemText(long item, long column) const;

public:
	DataProcessing(std::shared_ptr<ButtonData> buttons);

	void setInputCallback(std::function<void(Btn, bool)> callback);

	bool getButtonState(Btn button);

	void setButtonState(Btn button, bool state);

	void toggleButtonState(Btn button);

	void setCurrentFrame(uint32_t frameNum);

	void addNewFrame();

	void handleKeyboardInput(wxChar key);
	// The class itself is the list control
	// std::shared_ptr<wxGenericListCtrl> getWidget();
};

// wxBEGIN_EVENT_TABLE(DataProcessing, wxGenericListCtrl) EVT_MENU(wxID_EXIT, MyFrame::OnExit) EVT_MENU(DO_TEST, MyFrame::DoTest) EVT_SIZE(MyFrame::OnSize) EVT_BUTTON(BUTTON1, MyFrame::OnButton1) wxEND_EVENT_TABLE()
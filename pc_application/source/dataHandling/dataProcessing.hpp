#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <wx/glcanvas.h>
#include <wx/grid.h>
#include <wx/listctrl.h>

#include <wx/wx.h>
#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "buttonData.hpp"

class FrameCanvas : public wxGLCanvas {
private:
	wxGLContext* co;
	bool init;

	uint64_t currentPixelOffset;
	uint32_t currentFirstItem;
	uint32_t currentLastItem;

public:
	// https://wiki.wxwidgets.org/WxGLCanvas#Multiple_Canvases
	FrameCanvas(wxFrame* parent);

	void SetupGL();

	void Render(wxIdleEvent& event);
	void Resize(wxSizeEvent& event);

	void SetupViewport();

	// I expect the number of pixels scrolled to be incredibly large
	// First item is the first input listed in the view, the last item is the same
	//   but is the last item
	// The math to achieve this is somewhat tricky, but should work
	// Will use `GetTopItem` for first item and that plus `GetCountPerPage` for the last
	// Will have to use trickery with `GetItemRect` for actual scroll position, with wxLIST_RECT_BOUNDS
	//   should actually return pixels scrolled *within* the list control, http://wxpython-users.1045709.n5.nabble.com/listCtrl-virtual-how-to-possition-item-in-middle-or-top-of-screen-tp2307363p2307366.html
	void setPixelsScrolled(uint64_t pixelOffset, uint32_t firstItem, uint32_t lastItem);

	wxDECLARE_EVENT_TABLE();
};

// clang-format off
wxBEGIN_EVENT_TABLE(FrameCanvas, wxGLCanvas)
	EVT_IDLE(FrameCanvas::Render)
	//EVT_SIZE(FrameCanvas::Resize)
wxEND_EVENT_TABLE()

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
	// Main settings
	rapidjson::Document* mainSettings;
	// Mask color for transparency
	wxColour maskColor;
	// Reference to the main settings
	// Tree view viewed in the UI
	// Gtk::TreeView treeView;
	// Scrollable data window
	// std::shared_ptr<Gtk::ScrolledWindow> scrolledWindow;
	// Using callbacks for inputs
	// std::function<void(Btn, bool)> inputCallback;
	virtual int OnGetItemColumnImage(long item, long column) const;
	virtual wxString OnGetItemText(long item, long column) const;

public:
	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxWindow* parent);

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
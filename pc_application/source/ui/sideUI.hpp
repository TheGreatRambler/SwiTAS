#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>
#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "../dataHandling/dataProcessing.hpp"

class FrameCanvas : public wxGLCanvas {
private:
	wxGLContext* co;
	bool init;

	uint64_t currentPixelOffset;
	uint32_t currentFirstItem;
	uint32_t currentLastItem;

public:
	// https://wiki.wxwidgets.org/WxGLCanvas#Multiple_Canvases
	// Used to be wxFrame*
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

class SideUI {
private:
	rapidjson::Document* mainSettings;

	std::shared_ptr<wxBoxSizer> verticalBoxSizer;

	std::shared_ptr<wxBitmap> playBitmap;
	std::shared_ptr<wxBitmap> frameAdvanceBitmap;

	std::shared_ptr<wxBitmapButton> playButton;
	std::shared_ptr<wxBitmapButton> frameAdvanceButton;

    // Sizer holding the inputs and the fancy viewer next to them
    std::shared_ptr<wxBoxSizer> inputsViewSizer;

    std::shared_ptr<FrameCanvas> frameCanvas;

	std::shared_ptr<wxFrame> frameCanvasFrame;

    std::shared_ptr<DataProcessing> inputData;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

public:
	SideUI(rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input);
};
#pragma once

#include <memory>
#include <rapidjson/document.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>

#include "../dataHandling/dataProcessing.hpp"
<<<<<<< HEAD
#include "drawingCanvas.hpp"

class FrameCanvas : public DrawingCanvas {
	void draw(wxDC* dc);
}
=======
#include "../helpers.hpp"

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

	DECLARE_EVENT_TABLE();
};
>>>>>>> bf27387cf34d6d321956bfe23f51b7cccf5ad259

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
<<<<<<< HEAD
=======

	std::shared_ptr<FrameCanvas> frameCanvas;
>>>>>>> bf27387cf34d6d321956bfe23f51b7cccf5ad259

	std::shared_ptr<FrameCanvas> frameDrawer;

	std::shared_ptr<DataProcessing> inputData;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

<<<<<<< HEAD
	void onPlayPressed(wxMouseEvent& event);
	void onFrameAdvancePressed(wxMouseEvent& event);
=======
	void onPlayPressed(wxCommandEvent& event);
	void onFrameAdvancePressed(wxCommandEvent& event);
>>>>>>> bf27387cf34d6d321956bfe23f51b7cccf5ad259

public:
	SideUI(wxFrame* parentFrame, rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input);
};
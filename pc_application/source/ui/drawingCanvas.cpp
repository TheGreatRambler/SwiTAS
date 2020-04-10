#include "drawingCanvas.hpp"

DrawingCanvas::DrawingCanvas(wxWindow* parent, wxSize size)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE) {
	// By default it's black
	backgroundColor = *wxBLACK;
}

void DrawingCanvas::draw(wxDC& dc) {}

void DrawingCanvas::OnPaint(wxPaintEvent& event) {
	wxBufferedPaintDC dc(this);

	// Paint the background
	PaintBackground(dc);

	// Paint the graphic
	draw(dc);
}

void DrawingCanvas::PaintBackground(wxDC& dc) {
	dc.SetBackground(wxBrush(backgroundColor));
	dc.Clear();
}

// Empty implementation, to prevent flicker
void DrawingCanvas::OnEraseBackground(wxEraseEvent& event) {}

void DrawingCanvas::setBackgroundColor(wxColor color) {
	backgroundColor = color;
}

// clang-format off
BEGIN_EVENT_TABLE(DrawingCanvas, wxWindow)
    EVT_PAINT(DrawingCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(DrawingCanvas::OnEraseBackground)
END_EVENT_TABLE()
// clang-format on

DrawingCanvasBitmap::DrawingCanvasBitmap(wxWindow* parent, wxSize size)
	: DrawingCanvas(parent, size) {
	// Just create an empty bitmap of the right size
	bitmap = new wxBitmap(size);
}

void DrawingCanvasBitmap::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);
	// Set scaling for the image to render without wxImage
	dc.SetUserScale((double)width / bitmap->GetWidth(), (double)height / bitmap->GetHeight());
	// Render the default image, that's it
	dc.DrawBitmap(*bitmap, 0, 0, false);
	// VERY IMPORTANT for later work drawing semi-transparent wxBitmap, use wxAlphaPixelData
	// https://docs.wxwidgets.org/3.0/classwx_pixel_data.html
}

void DrawingCanvasBitmap::setBitmap(wxBitmap* theBitmap) {
	// Delete the earlier one
	delete bitmap;
	// Select the new one
	bitmap = theBitmap;
	Refresh();
}
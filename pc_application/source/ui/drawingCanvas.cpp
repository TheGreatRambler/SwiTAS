#include "drawingCanvas.hpp"

DrawingCanvas::DrawingCanvas(wxFrame* parent, wxSize size)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, size) {}

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
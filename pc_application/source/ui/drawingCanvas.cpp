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
	dc.SetBrush(wxBrush(backgroundColor));
	dc.SetPen(wxPen(backgroundColor, 1));

	wxRect windowRect(wxPoint(0, 0), GetClientSize());

	dc.DrawRectangle(windowRect);
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
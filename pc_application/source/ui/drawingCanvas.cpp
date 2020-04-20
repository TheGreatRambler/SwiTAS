#include "drawingCanvas.hpp"

DrawingCanvas::DrawingCanvas(wxWindow* parent, wxSize size)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE) {
	// By default it's black
	backgroundColor = *wxBLACK;
	zoomPoint       = wxDefaultPosition;
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

void DrawingCanvas::OnMousewheel(wxMouseEvent& event) {
	int ticks = event.GetWheelRotation() / event.GetWheelDelta();
	zoomScale += (double)ticks / 20;
	if(zoomScale < 0.01) {
		zoomScale = 0.01;
	}
	zoomPoint = event.GetPosition();
	Refresh();
}

// clang-format off
BEGIN_EVENT_TABLE(DrawingCanvas, wxWindow)
    EVT_PAINT(DrawingCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(DrawingCanvas::OnEraseBackground)
	EVT_MOUSEWHEEL(DrawingCanvas::OnMousewheel)
END_EVENT_TABLE()
// clang-format on

DrawingCanvasBitmap::DrawingCanvasBitmap(wxWindow* parent, wxSize size)
	: DrawingCanvas(parent, size) {
	// Just create an empty bitmap of the right size
	bitmap = new wxBitmap(size);
}

void DrawingCanvasBitmap::draw(wxDC& dc) {
	if(bitmap != NULL) {
		int width;
		int height;
		GetSize(&width, &height);

		// Scale for width and height are the same
		double scale = (double)width / bitmap->GetWidth();

		// https://forums.wxwidgets.org/viewtopic.php?t=21080
		scale *= zoomScale;
		dc.SetUserScale(scale, scale);
		if(zoomPoint != wxDefaultPosition) {
			wxPoint adjustedZoomPoint;
			// TODO fix this crap
			adjustedZoomPoint.x = dc.DeviceToLogicalX(zoomPoint.x);
			adjustedZoomPoint.y = dc.DeviceToLogicalY(zoomPoint.y);
			wxPoint middlePoint = zoomPoint - adjustedZoomPoint;
			dc.SetDeviceOrigin(middlePoint.x, middlePoint.y);
		}

		dc.DrawBitmap(*bitmap, 0, 0, false);
	}
}

void DrawingCanvasBitmap::setBitmap(wxBitmap* theBitmap) {
	// Delete the earlier one
	if(bitmap != NULL) {
		delete bitmap;
	}
	// Select the new one
	bitmap = theBitmap;
	Refresh();
}
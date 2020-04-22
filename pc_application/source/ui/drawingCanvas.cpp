#include "drawingCanvas.hpp"

DrawingCanvas::DrawingCanvas(wxWindow* parent, wxSize size)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE) {
	// By default it's black
	backgroundColor  = *wxBLACK;
	panningOffset    = wxPoint(0, 0);
	panningBeginning = wxPoint(0, 0);
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
	if(zoomScale < 0) {
		zoomScale = 0;
	}
	zoomPoint = event.GetPosition();
	Refresh();
}

void DrawingCanvas::OnMouseMove(wxMouseEvent& event) {
	if(event.Dragging()) {
		if(leftUp) {
			// Beginning of the drag
			leftUp           = false;
			panningBeginning = event.GetPosition();
			initialOffset    = panningOffset;
		} else {
			panningOffset = event.GetPosition() - panningBeginning + initialOffset;
			Refresh();
		}
	}
}

void DrawingCanvas::OnMouseUp(wxMouseEvent& event) {
	if(!leftUp) {
		leftUp = true;
	}
}

// clang-format off
BEGIN_EVENT_TABLE(DrawingCanvas, wxWindow)
    EVT_PAINT(DrawingCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(DrawingCanvas::OnEraseBackground)
	EVT_MOUSEWHEEL(DrawingCanvas::OnMousewheel)
	EVT_MOTION(DrawingCanvas::OnMouseMove)
	EVT_LEFT_UP(DrawingCanvas::OnMouseUp)
END_EVENT_TABLE()
// clang-format on

DrawingCanvasBitmap::DrawingCanvasBitmap(wxWindow* parent, wxSize size)
	: DrawingCanvas(parent, size) {
	// Just create an empty bitmap of the right size
	bitmap = new wxBitmap(size);
}

void DrawingCanvasBitmap::draw(wxDC& dc) {
	if(bitmap != NULL) {
		wxSize windowSize = GetSize();

		// Scale for width and height are the same
		double scale = (double)windowSize.GetWidth() / bitmap->GetWidth();

		// https://forums.wxwidgets.org/viewtopic.php?f=1&t=46751&p=196414#p196414
		double currentScale = scale * std::exp(zoomScale);
		// double lastScale    = std::exp(scale * lastZoomScale);

		dc.SetUserScale(currentScale, currentScale);
		if(zoomPoint != wxDefaultPosition) {
			wxPoint zoomOffsetPoint = (zoomPoint - dc.GetDeviceOrigin());

			wxPoint imageCorner = zoomPoint - zoomOffsetPoint * currentScale;
			wxPoint origin      = imageCorner + panningOffset;

			dc.SetDeviceOrigin(origin.x, origin.y);
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

wxBitmap* DrawingCanvasBitmap::getBitmap() {
	return bitmap;
}
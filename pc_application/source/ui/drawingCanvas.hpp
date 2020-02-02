#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

class DrawingCanvas : public wxWindow {
private:
	wxColor backgroundColor;

public:
	// To be overriden
	virtual void draw(wxDC* dc);

	// http://www.informit.com/articles/article.aspx?p=405047

	void OnPaint(wxPaintEvent& event) {
		wxBufferedPaintDC dc(this);

		// Paint the background
		PaintBackground(dc);

		// Paint the graphic
		draw(&dc);
	}

	void PaintBackground(wxDC* dc) {
		dc->SetBrush(wxBrush(backgroundColor));
		dc->SetPen(wxPen(backgroundColor, 1));

		wxRect windowRect(wxPoint(0, 0), GetClientSize());

		dc->DrawRectangle(windowRect);
	}

	// Empty implementation, to prevent flicker
	void OnEraseBackground(wxEraseEvent& event) { }

	void setBackgroundColor(wxColor color) {
		backgroundColor = color;
	}

	wxDECLARE_EVENT_TABLE();
};

// clang-format off
wxBEGIN_EVENT_TABLE(DrawingCanvas, wxWindow)
    EVT_PAINT(DrawingCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(DrawingCanvas::OnEraseBackground)
wxEND_EVENT_TABLE()
	// clang-format on
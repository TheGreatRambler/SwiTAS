#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

class DrawingCanvas : public wxWindow {
private:
	wxColor backgroundColor;

public:
	DrawingCanvas(wxFrame* parent, wxSize size);

	// To be overriden
	virtual void draw(wxDC& dc) = 0;

	// http://www.informit.com/articles/article.aspx?p=405047

	void OnPaint(wxPaintEvent& event);

	void PaintBackground(wxDC& dc);

	// Empty implementation, to prevent flicker
	void OnEraseBackground(wxEraseEvent& event);

	void setBackgroundColor(wxColor color);

	DECLARE_EVENT_TABLE();
};

// A version of DrawingCanvas that just renders an image
class DrawingCanvasBitmap : public DrawingCanvas {
private:
	wxBitmap* bitmap;

public:
	DrawingCanvasBitmap(wxFrame* parent, wxSize size);

    void draw(wxDC& dc) override;

	void setBitmap(wxBitmap* theBitmap);
}
#pragma once

#include <cmath>
#include <wx/dcbuffer.h>
#include <wx/wx.h>

class DrawingCanvas : public wxWindow {
private:
	wxColor backgroundColor;

protected:
	double zoomScale = 0;
	wxPoint zoomPoint;
	wxPoint panningOffset;

	wxPoint initialOffset;
	wxPoint panningBeginning;

	bool leftUp = true;

public:
	DrawingCanvas(wxWindow* parent, wxSize size);

	// To be overriden
	virtual void draw(wxDC& dc) = 0;

	// http://www.informit.com/articles/article.aspx?p=405047

	void OnPaint(wxPaintEvent& event);
	void OnMousewheel(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseUp(wxMouseEvent& event);

	void PaintBackground(wxDC& dc);

	// Empty implementation, to prevent flicker
	void OnEraseBackground(wxEraseEvent& event);

	void setBackgroundColor(wxColor color);

	DECLARE_EVENT_TABLE();
};

// A version of DrawingCanvas that just renders an image
class DrawingCanvasBitmap : public DrawingCanvas {
private:
	wxBitmap* bitmap = NULL;

public:
	DrawingCanvasBitmap(wxWindow* parent, wxSize size);

	void draw(wxDC& dc) override;

	void setBitmap(wxBitmap* theBitmap);
	wxBitmap* getBitmap();
};
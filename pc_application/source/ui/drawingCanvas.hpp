#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

class DrawingCanvas : public wxWindow {
private:
	wxColor backgroundColor;
	wxSize preferredWindowSize;

public:
	DrawingCanvas(wxFrame* parent);

	// To be overriden
	virtual void draw(wxDC& dc) = 0;

	// http://www.informit.com/articles/article.aspx?p=405047

	void OnPaint(wxPaintEvent& event);

	void PaintBackground(wxDC& dc);

	wxSize DoGetBestClientSize() const;

	void setPreferredSize(wxSize preferredSize);

	// Empty implementation, to prevent flicker
	void OnEraseBackground(wxEraseEvent& event);

	void setBackgroundColor(wxColor color);

	DECLARE_EVENT_TABLE();
};
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

	void OnPaint(wxPaintEvent& event);

	void PaintBackground(wxDC* dc);

	// Empty implementation, to prevent flicker
	void OnEraseBackground(wxEraseEvent& event);

	void setBackgroundColor(wxColor color);

	DECLARE_EVENT_TABLE();
};
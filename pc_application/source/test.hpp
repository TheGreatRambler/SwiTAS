// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

class PanAndZoomCanvas : public wxWindow {
public:
	PanAndZoomCanvas(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = "PanAndZoomCanvas");
	wxRect2DDouble GetUntransformedRect() const;

protected:
	void DoDrawCanvas(wxGraphicsContext*);

private:
	void OnPaint(wxPaintEvent&);

	void OnMouseWheel(wxMouseEvent&);

	void OnLeftDown(wxMouseEvent&);
	void OnMotion(wxMouseEvent&);
	void OnLeftUp(wxMouseEvent&);
	void OnCaptureLost(wxMouseCaptureLostEvent&);

	void ProcessPan(const wxPoint&, bool);
	void FinishPan(bool);

	int m_zoomFactor;

	wxPoint2DDouble m_panVector;
	wxPoint2DDouble m_inProgressPanVector;
	wxPoint m_inProgressPanStartPoint;
	bool m_panInProgress;
};

PanAndZoomCanvas::PanAndZoomCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name) {
	Bind(wxEVT_PAINT, &PanAndZoomCanvas::OnPaint, this);
	Bind(wxEVT_MOUSEWHEEL, &PanAndZoomCanvas::OnMouseWheel, this);
	Bind(wxEVT_LEFT_DOWN, &PanAndZoomCanvas::OnLeftDown, this);

	SetBackgroundStyle(wxBG_STYLE_PAINT);

	m_zoomFactor = 100;

	m_panVector               = wxPoint2DDouble(0, 0);
	m_inProgressPanStartPoint = wxPoint(0, 0);
	m_inProgressPanVector     = wxPoint2DDouble(0, 0);
	m_panInProgress           = false;
}

void PanAndZoomCanvas::DoDrawCanvas(wxGraphicsContext* gc) {
	gc->SetPen(*wxBLACK_PEN);

	wxGraphicsPath path = gc->CreatePath();
	path.MoveToPoint(100, 100);
	path.AddLineToPoint(300, 100);
	path.AddLineToPoint(300, 300);
	path.CloseSubpath();
	gc->StrokePath(path);
}

void PanAndZoomCanvas::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxAutoBufferedPaintDC dc(this);
	dc.Clear();

	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

	if(gc) {
		double a                 = m_zoomFactor / 100.0;
		wxPoint2DDouble totalPan = m_panVector + m_inProgressPanVector;

		gc->Translate(-totalPan.m_x, -totalPan.m_y);
		gc->Scale(a, a);

		DoDrawCanvas(gc);

		delete gc;
	}
}

void PanAndZoomCanvas::OnMouseWheel(wxMouseEvent& event) {
	if(m_panInProgress) {
		FinishPan(false);
	}

	int rot   = event.GetWheelRotation();
	int delta = event.GetWheelDelta();

	int oldZoom = m_zoomFactor;
	m_zoomFactor += 10 * (rot / delta);

	if(m_zoomFactor < 10) {
		m_zoomFactor = 10;
	}

	if(m_zoomFactor > 800) {
		m_zoomFactor = 800;
	}

	double a = oldZoom / 100.0;
	double b = m_zoomFactor / 100.0;

	// Set the panVector so that the point below the cursor in the new
	// scaled/panned cooresponds to the same point that is currently below it.
	wxPoint2DDouble uvPoint    = event.GetPosition();
	wxPoint2DDouble stPoint    = uvPoint + m_panVector;
	wxPoint2DDouble xypoint    = stPoint / a;
	wxPoint2DDouble newSTPoint = b * xypoint;
	m_panVector                = newSTPoint - uvPoint;

	Refresh();
}

void PanAndZoomCanvas::ProcessPan(const wxPoint& pt, bool refresh) {
	m_inProgressPanVector = m_inProgressPanStartPoint - pt;

	if(refresh) {
		Refresh();
	}
}

void PanAndZoomCanvas::FinishPan(bool refresh) {
	if(m_panInProgress) {
		SetCursor(wxNullCursor);

		if(HasCapture()) {
			ReleaseMouse();
		}

		Unbind(wxEVT_LEFT_UP, &PanAndZoomCanvas::OnLeftUp, this);
		Unbind(wxEVT_MOTION, &PanAndZoomCanvas::OnMotion, this);
		Unbind(wxEVT_MOUSE_CAPTURE_LOST, &PanAndZoomCanvas::OnCaptureLost, this);

		m_panVector += m_inProgressPanVector;
		m_inProgressPanVector = wxPoint2DDouble(0, 0);
		m_panInProgress       = false;

		if(refresh) {
			Refresh();
		}
	}
}

wxRect2DDouble PanAndZoomCanvas::GetUntransformedRect() const {
	double a = m_zoomFactor / 100.0;

	wxSize sz            = GetSize();
	wxPoint2DDouble zero = m_panVector / a;

	return wxRect2DDouble(zero.m_x, zero.m_y, sz.GetWidth() / a, sz.GetHeight() / a);
}

void PanAndZoomCanvas::OnLeftDown(wxMouseEvent& event) {
	wxCursor cursor(wxCURSOR_HAND);
	SetCursor(cursor);

	m_inProgressPanStartPoint = event.GetPosition();
	m_inProgressPanVector     = wxPoint2DDouble(0, 0);
	m_panInProgress           = true;

	Bind(wxEVT_LEFT_UP, &PanAndZoomCanvas::OnLeftUp, this);
	Bind(wxEVT_MOTION, &PanAndZoomCanvas::OnMotion, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &PanAndZoomCanvas::OnCaptureLost, this);

	CaptureMouse();
}

void PanAndZoomCanvas::OnMotion(wxMouseEvent& event) {
	ProcessPan(event.GetPosition(), true);
}

void PanAndZoomCanvas::OnLeftUp(wxMouseEvent& event) {
	ProcessPan(event.GetPosition(), false);
	FinishPan(true);
}

void PanAndZoomCanvas::OnCaptureLost(wxMouseCaptureLostEvent&) {
	FinishPan(true);
}

class MyFrame : public wxFrame {
public:
	MyFrame(wxWindow* parent, int id = wxID_ANY, wxString title = "Demo", wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_FRAME_STYLE);
};

MyFrame::MyFrame(wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style)
	: wxFrame(parent, id, title, pos, size, style) {
	PanAndZoomCanvas* canvas = new PanAndZoomCanvas(this);
}

class myApp : public wxApp {
public:
	virtual bool OnInit() {
		MyFrame* frame = new MyFrame(NULL);
		frame->Show();
		return true;
	}
};

wxIMPLEMENT_APP(myApp);
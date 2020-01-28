#include "sideUI.hpp"
#include <memory>

FrameCanvas::FrameCanvas(wxFrame* parent) {
	// Initialize base class
	const int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16 };
	wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, 0, "GLCanvas");
	co   = new wxGLContext((wxGLCanvas*)this);
	init = false;
}

void FrameCanvas::SetupGL() {
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void FrameCanvas::Render(wxIdleEvent& event) {
	SetCurrent(*co);

	if(!init) {
		SetupGL();
		SetupViewport();
		init = true;
	}
	// Draw

	// Use nanovg to draw a circle
	// SetCurrent sets the GL context
	// Now can use nanovg
	SetCurrent(*co);
	/*
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLint)200, (GLint)200);
	glColor3f(1.0, c_, c_);

	glBegin(GL_POLYGON);
	glVertex3f(-0.5, -0.5, 5 * cos(rotate_));
	glVertex3f(-0.5, 0.5, 5 * cos(rotate_));
	glVertex3f(0.5, 0.5, -5 * cos(rotate_));
	glVertex3f(0.5, -0.5, -5 * cos(rotate_));
	glEnd();
	*/
	// Render
	SwapBuffers();

	// Dunno what this does
	event.RequestMore();
}

void FrameCanvas::Resize(wxSizeEvent& event) {
	SetCurrent(*co);
	SetupViewport();
	wxGLCanvas::OnSize(event);
	Refresh();
}

void FrameCanvas::SetupViewport() {
	int x, y;
	GetSize(&x, &y);
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)x / y, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void FrameCanvas::setPixelsScrolled(uint64_t pixelOffset, uint32_t firstItem, uint32_t lastItem) {
	currentFirstItem   = firstItem;
	currentLastItem    = lastItem;
	currentPixelOffset = pixelOffset;
}

SideUI::SideUI(rapidjson::Document* settings, wxFlexGridSizer* sizer, std::shared_ptr<DataProcessing> input) {
	mainSettings = settings;
	inputData    = input;

	verticalBoxSizer = std::make_shared<wxBoxSizer>(wxVERTICAL);

	// Holds input stuff
	inputsViewSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);

	frameCanvasFrame = std::make_shared<wxFrame>(inputsViewSizer.get(), wxID_ANY, "InputCanvas");
	frameCanvas      = std::make_shared<FrameCanvas>(frameCanvasFrame.get());
	frameCanvasFrame->AddChild(frameCanvas.get());

	wxBitmap play((*mainSettings)["ui"]["playButton"].GetString(), wxBITMAP_TYPE_PNG);
	wxBitmap frameAdvance((*mainSettings)["ui"]["frameAdvanceButton"].GetString(), wxBITMAP_TYPE_PNG);

	playButton         = std::make_shared<wxBitmapButton>(verticalBoxSizer.get(), -1, play, wxDefaultPosition, wxDefaultSize, 0);
	frameAdvanceButton = std::make_shared<wxBitmapButton>(verticalBoxSizer.get(), -1, frameAdvance, wxDefaultPosition, wxDefaultSize, 0);

	verticalBoxSizer->Add(playButton.get(), wxEXPAND | wxALL);
	verticalBoxSizer->Add(frameAdvanceButton.get(), wxEXPAND | wxALL);

	inputsViewSizer->Add(frameCanvasFrame.get(), wxEXPAND | wxALL);
	inputsViewSizer->Add(inputData.get(), wxEXPAND | wxALL);

	verticalBoxSizer->Add(inputsViewSizer.get(), wxEXPAND | wxALL);

	// DataProcessing is itself a list control, so add it
	verticalBoxSizer->Add(input.get(), wxEXPAND | wxALL);

	// Just add it
	sizer->Add(verticalBoxSizer.get(), wxEXPAND | wxALL);
}
#include "bottomUI.hpp"

ButtonGrid::ButtonGrid(wxFrame* parent, wxSize requiredSize, std::shared_ptr<ButtonData> data, DataProcessing* inputs)
	: DrawingCanvas(parent, requiredSize) {
	buttonData             = data;
	totalCombinedImageSize = requiredSize;
	inputInstance          = inputs;

	setBackgroundColor(*wxWHITE);

	// Handle grid clicking
	Bind(wxEVT_LEFT_DOWN, &ButtonGrid::onGridClick, this);

	// Callback stuff
	inputInstance->setInputCallback(std::bind(&ButtonGrid::setIconState, this, std::placeholders::_1, std::placeholders::_2));
}

void ButtonGrid::setIconState(Btn button, bool state) {
	// Just trigger a refresh (I'm too lazy)
	Refresh();
}

void ButtonGrid::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);

	// Set scaling for the image to render without wxImage
	double scaleWidth  = (double)width / totalCombinedImageSize.GetWidth();
	double scaleHeight = (double)height / totalCombinedImageSize.GetHeight();
	dc.SetUserScale(scaleWidth, scaleHeight);

	// TODO not every image is a square :)
	for(auto const& button : buttonData->buttonMapping) {
		wxBitmap* bitmap;
		if(inputInstance->getButtonState(button.first)) {
			bitmap = button.second->resizedGridOnBitmap;
		} else {
			bitmap = button.second->resizedGridOffBitmap;
		}
		wxPoint location;
		location.x = button.second->gridX * bitmap->GetWidth();
		location.y = button.second->gridY * bitmap->GetHeight();

		dc.DrawBitmap(*bitmap, location, true);
	}
}

void ButtonGrid::onGridClick(wxMouseEvent& event) {
	wxPoint windowCoord = event.GetPosition();
	// Divide to handle scaling
	int width;
	int height;
	GetSize(&width, &height);
	float X = ((float)windowCoord.x / width) * buttonData->KeyWidth;
	float Y = ((float)windowCoord.y / height) * buttonData->KeyHeight;
	// Have to loop, no way around it
	for(auto const& button : buttonData->buttonMapping) {
		uint8_t locX = button.second->gridX;
		uint8_t locY = button.second->gridY;
		if((X > locX && X < locX + 1) && (Y > locY && Y < locY + 1)) {
			// This is the button
			inputInstance->toggleButtonState(button.first);
			break;
		}
	}
	// If the loop can't find anything, nothing happens
}

FrameViewerCanvas::FrameViewerCanvas(wxFrame* parent, wxBitmap* defaultImage)
	: DrawingCanvas(parent, wxSize(1280, 720)) {
	// Needs to be able to fit the frames
	hasFrameToRender  = false;
	defaultBackground = defaultImage;
}

void FrameViewerCanvas::draw(wxDC& dc) {
	int width;
	int height;
	GetSize(&width, &height);
	if(!hasFrameToRender) {
		// Set scaling for the image to render without wxImage
		dc.SetUserScale((double)width / defaultBackground->GetWidth(), (double)height / defaultBackground->GetHeight());
		// Render the default image, that's it
		dc.DrawBitmap(*defaultBackground, 0, 0, false);
	} else {
		// Do thing
	}
}

JoystickCanvas::JoystickCanvas(wxFrame* parent)
	: DrawingCanvas(parent, wxSize(150, 150)) {
	// Should be a decent size
}

void JoystickCanvas::draw(wxDC& dc) {
	// Do thing
	int width;
	int height;
	GetSize(&width, &height);

	wxPoint approximateMiddle((float)width / 2, (float)height / 2);

	dc.SetPen(*wxGREEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);

	dc.DrawCircle(approximateMiddle, approximateMiddle.x - 3);
}

BottomUI::BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input) {
	// TODO set up joysticks
	buttonData   = buttons;
	mainSettings = settings;

	inputInstance = input;

	// Game frame viewer

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	horizontalBoxSizer = new wxBoxSizer(wxHORIZONTAL);

	leftJoystickDrawer = new JoystickCanvas(parentFrame);
	leftJoystickDrawer->setBackgroundColor(*wxWHITE);
	rightJoystickDrawer = new JoystickCanvas(parentFrame);
	rightJoystickDrawer->setBackgroundColor(*wxWHITE);

	wxSize gridSize;
	// Just to get a rough estimate
	int w = buttonData->KeyWidth * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetWidth();
	int h = buttonData->KeyHeight * buttonData->buttonMapping[Btn::A]->resizedGridOffBitmap->GetHeight();
	gridSize.SetWidth(w);
	gridSize.SetHeight(h);

	buttonGrid = new ButtonGrid(parentFrame, gridSize, buttonData, inputInstance);

	frameViewerCanvas = new FrameViewerCanvas(parentFrame, new wxBitmap(HELPERS::resolvePath((*mainSettings)["videoViewerDefaultImage"].GetString()), wxBITMAP_TYPE_JPEG));

	// These take up much less space than the grid
	horizontalBoxSizer->Add(leftJoystickDrawer, 0, wxSHAPED | wxEXPAND);
	horizontalBoxSizer->Add(rightJoystickDrawer, 0, wxSHAPED | wxEXPAND);

	horizontalBoxSizer->Add(buttonGrid, 0, wxSHAPED | wxEXPAND);

	// Proportion HAS to be zero here, it's a requirment
	mainSizer->Add(frameViewerCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(horizontalBoxSizer, 1, wxEXPAND | wxALL);

	theGrid->Add(mainSizer, 3, wxEXPAND | wxALL);
}
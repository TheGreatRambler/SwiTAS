#include "bottomUI.hpp"

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

renderImageInGrid::renderImageInGrid(wxBitmap* bitmap, Btn btn, uint8_t width, uint8_t height, wxGrid* gridParent) {
	theBitmap  = bitmap;
	button     = btn;
	gridWidth  = width;
	gridHeight = height;
	parent     = gridParent;
}

void renderImageInGrid::setBitmap(wxBitmap* bitmap) {
	theBitmap = bitmap;
}

void renderImageInGrid::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	int parentWidth;
	int parentHeight;
	parent->GetSize(&parentWidth, &parentHeight);

	// This is so the images sorta fit
	int cellWidth  = roundf((float)parentWidth / gridWidth);
	int cellHeight = roundf((float)parentHeight / gridHeight);

	// Set scaling for the image to render without wxImage
	dc.SetUserScale((double)cellWidth / theBitmap->GetWidth(), (double)cellHeight / theBitmap->GetHeight());

	// Call base class ::Draw to clear the cell and draw the borders etc.
	// Never selected, however
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
	// Draw rect in the right place
	// dc.Clear();
	dc.DrawBitmap(*theBitmap, rect.x, rect.y);
}

BottomUI::BottomUI(wxFrame* parentFrame, rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, DataProcessing* input) {
	// TODO set up joysticks
	buttonData   = buttons;
	mainSettings = settings;

	inputInstance = input;
	// Callback stuff
	inputInstance->setInputCallback(std::bind(&BottomUI::setIconState, this, std::placeholders::_1, std::placeholders::_2));

	// Game frame viewer

	mainSizer          = new wxBoxSizer(wxVERTICAL);
	horizontalBoxSizer = new wxBoxSizer(wxHORIZONTAL);

	leftJoystickDrawer = new JoystickCanvas(parentFrame);
	leftJoystickDrawer->setBackgroundColor(*wxWHITE);
	rightJoystickDrawer = new JoystickCanvas(parentFrame);
	rightJoystickDrawer->setBackgroundColor(*wxWHITE);

	frameViewerCanvas = new FrameViewerCanvas(parentFrame, new wxBitmap(HELPERS::resolvePath((*mainSettings)["videoViewerDefaultImage"].GetString()), wxBITMAP_TYPE_JPEG));

	// According to https://forums.wxwidgets.org/viewtopic.php?p=120136#p120136, it cant be wxDefaultSize
	buttonGrid = new wxGrid(parentFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	// Removes gridlines, this might be cool in the future
	// https://docs.wxwidgets.org/3.0/classwx_grid.html#abf968b3b0d70d2d9cc5bacf7f9d9891a
	buttonGrid->EnableGridLines(false);
	// Height * Width
	buttonGrid->CreateGrid(4, 11);
	buttonGrid->EnableEditing(false);

	// Handle grid clicking
	buttonGrid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &BottomUI::onGridClick, this);

	for(auto const& button : KeyLocs) {
		// https://forums.wxwidgets.org/viewtopic.php?t=40428
		wxGridCellAttr* attr = new wxGridCellAttr();
		attr->SetRenderer(new renderImageInGrid(buttonData->buttonMapping[button.first]->resizedGridOffBitmap, button.first, keyWidth, keyHeight, buttonGrid));
		attr->SetReadOnly(true);
		buttonGrid->SetAttr(button.second.y, button.second.x, attr);
	}

	// Nice source for sizer stuff
	// http://neume.sourceforge.net/sizerdemo/

	// No need for the weird header
	buttonGrid->SetRowLabelSize(0);
	buttonGrid->SetColLabelSize(0);
	// Fit cell size to contents
	buttonGrid->AutoSize();

	// These take up much less space than the grid
	horizontalBoxSizer->Add(leftJoystickDrawer, 0, wxSHAPED | wxEXPAND | wxALIGN_LEFT);
	horizontalBoxSizer->Add(rightJoystickDrawer, 0, wxSHAPED | wxEXPAND | wxALIGN_LEFT);

	// So it can get very small
	buttonGrid->SetMinSize(wxSize(0, 0));
	horizontalBoxSizer->Add(buttonGrid, 1, wxEXPAND | wxALL);

	// Proportion HAS to be zero here, it's a requirment
	mainSizer->Add(frameViewerCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(horizontalBoxSizer, 1, wxEXPAND | wxALL);

	theGrid->Add(mainSizer, 3, wxEXPAND | wxALL);
}

void BottomUI::onGridClick(wxGridEvent& event) {
	// https://forums.wxwidgets.org/viewtopic.php?t=21585
	long col = event.GetCol();
	long row = event.GetRow();

	wxGridCellRenderer* cellRenderer = buttonGrid->GetCellRenderer(row, col);
	Btn button                       = ((renderImageInGrid*)cellRenderer)->getButton();
	// If it has a renderer, it must be good
	// It spits out garbage like 4277075694 when the cell renderer is actually not a button renderer
	if(button < Btn::BUTTONS_SIZE) {
		// This is a custom cell renderer
		// Toggle the button state via the cell renderer hopefully
		inputInstance->toggleButtonState(button);
	}
	// Same DecRef stuff
	cellRenderer->DecRef();

	event.Skip();
}

void BottomUI::setIconState(Btn button, bool state) {
	// TODO this needs to be called by DataProcessing
	int x                       = KeyLocs[button].x;
	int y                       = KeyLocs[button].y;
	renderImageInGrid* renderer = (renderImageInGrid*)buttonGrid->GetCellRenderer(y, x);
	if(state) {
		// Set the image to the on image
		renderer->setBitmap(buttonData->buttonMapping[button]->resizedGridOnBitmap);
	} else {
		// Set the image to the off image
		renderer->setBitmap(buttonData->buttonMapping[button]->resizedGridOffBitmap);
	}
	// Have to DecRef https://docs.wxwidgets.org/3.0/classwx_grid.html#a9640007f1e60efbaf00b3ac6f6f50f8f
	renderer->DecRef();
	buttonGrid->RefreshRect(buttonGrid->CellToRect(y, x));

	// Don't set value in input instance because it
	// Was the one that sent us here
}
#include "bottomUI.hpp"

void JoystickCanvas::draw(wxDC* dc) {
	// Do thing
}

renderImageInGrid::renderImageInGrid(std::shared_ptr<wxBitmap> bitmap, Btn btn) {
	theBitmap = bitmap;
	button    = btn;
}

void renderImageInGrid::setBitmap(std::shared_ptr<wxBitmap> bitmap) {
	theBitmap = bitmap;
}

void renderImageInGrid::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
	// Call base class ::Draw to clear the cell and draw the borders etc.
	wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
	// Draw rect in the right place
	dc.DrawBitmap(*theBitmap, rect.x, rect.y);
}

BottomUI::BottomUI(wxFrame* parentFrame, std::shared_ptr<ButtonData> buttons, wxBoxSizer* theGrid, std::shared_ptr<DataProcessing> input) {
	// TODO set up joysticks
	buttonData = buttons;

	inputInstance = input;
	// Callback stuff
	inputInstance->setInputCallback(std::bind(&BottomUI::setIconState, this, std::placeholders::_1, std::placeholders::_2));

	horizontalBoxSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);

	leftJoystickDrawer = std::make_shared<JoystickCanvas>();
	leftJoystickDrawer->setBackgroundColor(*wxWHITE);
	rightJoystickDrawer = std::make_shared<JoystickCanvas>();
	rightJoystickDrawer->setBackgroundColor(*wxWHITE);

	horizontalBoxSizer->Add(leftJoystickDrawer.get(), 0, wxEXPAND | wxALL);
	horizontalBoxSizer->Add(rightJoystickDrawer.get(), 0, wxEXPAND | wxALL);

	buttonGrid = std::make_shared<wxGrid>(parentFrame, wxID_ANY);

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
		attr->SetRenderer(new renderImageInGrid(buttonData->buttonMapping[button.first]->offBitmapIcon, button.first));

		buttonGrid->SetAttr(button.second.y, button.second.x, attr);
		/*
		// Add the images (the pixbuf can and will be changed later)
		std::shared_ptr<Gtk::Image> image = std::make_shared<Gtk::Image>(buttonData->buttonMapping[button.first].offIcon);
		// Add the eventbox
		std::shared_ptr<Gtk::EventBox> eventBox = std::make_shared<Gtk::EventBox>();
		eventBox->add(*image);
		eventBox->set_events(Gdk::BUTTON_PRESS_MASK);
		eventBox->signal_button_press_event().connect(sigc::bind<Btn>(sigc::mem_fun(*this, &BottomUI::onButtonPress), button.first));

		images.insert(std::pair<Btn, std::pair<std::shared_ptr<Gtk::Image>, std::shared_ptr<Gtk::EventBox>>>(button.first, { image, eventBox }));

		// Designate the off image as the default
		buttonViewer.attach(*eventBox, button.second.x, button.second.y);
		*/
	}

	// Nice source for sizer stuff
	// http://neume.sourceforge.net/sizerdemo/

	// No need for the weird header
	buttonGrid->SetRowLabelSize(0);
	buttonGrid->SetColLabelSize(0);
	// Fit cell size to contents
	buttonGrid->AutoSize();

	horizontalBoxSizer->Add(buttonGrid.get(), 0, wxEXPAND | wxALL);

	theGrid->Add(horizontalBoxSizer.get(), 0, wxEXPAND | wxALL);
}

void BottomUI::onGridClick(wxGridEvent& event) {
	// https://forums.wxwidgets.org/viewtopic.php?t=21585
	long col = event.GetCol();
	long row = event.GetRow();

	wxGridCellRenderer* cellRenderer = buttonGrid->GetCellRenderer(row, col);
	Btn button                       = ((renderImageInGrid*)cellRenderer)->getButton();
	// If it has a renderer, it must be good
	if(button != NULL) {
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
	Location location           = KeyLocs[button];
	renderImageInGrid* renderer = (renderImageInGrid*)buttonGrid->GetCellRenderer(location.y, location.x);
	if(state) {
		// Set the image to the on image
		renderer->setBitmap(buttonData->buttonMapping[button]->onBitmapIcon);
	} else {
		// Set the image to the off image
		renderer->setBitmap(buttonData->buttonMapping[button]->offBitmapIcon);
	}
	// Have to DecRef https://docs.wxwidgets.org/3.0/classwx_grid.html#a9640007f1e60efbaf00b3ac6f6f50f8f
	renderer->DecRef();
	buttonGrid->RefreshRect(buttonGrid->CellToRect(location.x, location.x));

	// Don't set value in input instance because it
	// Was the one that sent us here
}
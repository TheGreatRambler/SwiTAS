#pragma once

#include <wx/sizer.h>

#include "dataProcessing.hpp"

class SideUI {
private:
	// Button group for play, pause, etc...
	Gtk::ButtonBox playButtons;
	// Buttons themselves
	Gtk::Button playPauseButton;
	Gtk::Button frameAdvanceButton;
	// The box holding everything
	Gtk::VBox everythingBox;

	// Minimum size of this widget (it just gets too small normally)
	static constexpr float minimumSize = 1 / 4;

	// Input instance to get inputs and such
	std::shared_ptr<DataProcessing> inputInstance;

public:
	SideUI();

	void setInputInstance(std::shared_ptr<DataProcessing> input);

	void addToGrid(wxFlexGridSizer* theGrid);
};
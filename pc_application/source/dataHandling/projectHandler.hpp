#pragma once

class ProjectHandler {
private:
	// Add directory of project, project name, various other info
	// This will handle opening a directory chooser at the beginning of each session
	// https://docs.wxwidgets.org/3.0/classwx_dir_dialog.html
	// Probably having a list of the recent projects as well
	// Store recent projects in mainSettings.json

public:
	ProjectHandler();
};
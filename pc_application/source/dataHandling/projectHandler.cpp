#include "projectHandler.hpp"

ProjectHandler::ProjectHandler(DataProcessing* dataProcessingInstance)
	: wxFrame(NULL, wxID_ANY, "Select Project", wxDefaultPosition, wxDefaultSize) {
	// Get global settings file
	getGlobalSettings();
	dataProcessing = dataProcessingInstance;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	projectList = new wxRichTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRE_MULTILINE | wxRE_READONLY);

	auto recentProjectsArray = mainSettings["recentProjects"].GetArray();

	wxTextAttr projectNameAttr;
	projectNameAttr.SetFontUnderlined(true);
	projectNameAttr.SetFontSize(30);
	projectNameAttr.SetFontStyle(wxFONTSTYLE_ITALIC);
	projectNameAttr.SetFontWeight(wxFONTWEIGHT_BOLD);
	projectNameAttr.SetTextColour(wxColor(mainSettings["ui"]["recentProjectsColors"]["projectName"].GetString()));

	wxTextAttr projectDirectoryAttr;
	projectDirectoryAttr.SetFontSize(22);
	projectDirectoryAttr.SetFontStyle(wxFONTSTYLE_NORMAL);
	projectDirectoryAttr.SetFontWeight(wxFONTWEIGHT_LIGHT);
	projectDirectoryAttr.SetTextColour(wxColor(mainSettings["ui"]["recentProjectsColors"]["projectDir"].GetString()));

	for(auto const& recentProject : recentProjectsArray) {
		// Mark as URL and give the directory as the URL
		projectList->BeginURL(recentProject["projectDirectory"].GetString());
		projectList->SetDefaultStyle(projectNameAttr);
		projectList->AppendText(recentProject["projectName"].GetString());
		projectList->AppendText(" ");
		projectList->SetDefaultStyle(projectDirectoryAttr);
		projectList->AppendText(recentProject["projectDirectory"].GetString());
		projectList->EndURL();
		projectList->AppendText("\n");
	}

	wxTextAttr newProjectAttr;
	newProjectAttr.SetFontSize(35);
	newProjectAttr.SetFontStyle(wxFONTSTYLE_NORMAL);
	newProjectAttr.SetFontWeight(wxFONTWEIGHT_LIGHT);
	newProjectAttr.SetTextColour(wxColor(mainSettings["ui"]["recentProjectsColors"]["newProject"].GetString()));

	projectList->BeginURL("newProject");
	projectList->SetDefaultStyle(newProjectAttr);
	projectList->AppendText("Create New Project");
	projectList->EndURL();

	projectList->Bind(wxEVT_TEXT_URL, &ProjectHandler::onClickProject, this);

	mainSizer->Add(projectList, 0, wxEXPAND | wxALL);

	// Print create project input

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);
}

void ProjectHandler::getGlobalSettings() {
	std::ifstream settingsFile("../mainSettings.json");
	std::string content((std::istreambuf_iterator<char>(settingsFile)), (std::istreambuf_iterator<char>()));
	// Allow comments in JSON
	mainSettings.Parse<rapidjson::kParseCommentsFlag>(content.c_str());
}

void ProjectHandler::onClickProject(wxTextUrlEvent& event) {
	wxString selectedProject = event.GetString();
	if(selectedProject == "newProject") {
		// Open up a new project dialog
		wxDirDialog dlg(NULL, "Choose Project Directory", "", wxDD_DEFAULT_STYLE);
		if(dlg.ShowModal() == wxID_OK) {
			// Directory chosen
			projectDir.Open(dlg.GetPath());
		}
	} else {
		// It's a project folder
		projectDir.Open(selectedProject);
		// Only difference between new project and existing project is
		// DataProcessing is automatically filled with the existing data
	}
}

void ProjectHandler::saveProject() {
	// Save each set of data one by one
}
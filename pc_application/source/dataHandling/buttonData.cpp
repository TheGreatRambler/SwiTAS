#include "buttonData.hpp"

void ButtonData::setupButtonMapping(rapidjson::Document* mainSettings) {
	// Set up button mapping with the data obtained via JSON
	for(auto& b : (*mainSettings)["buttons"].GetObject()) {
		Btn chosenButton = stringToButton[b.name.GetString()];

		std::string scriptName   = b.value["scriptName"].GetString();
		std::string normalName   = b.value["normalName"].GetString();
		std::string viewName     = b.value["viewName"].GetString();
		std::string onIconImage  = HELPERS::resolvePath(b.value["onIconImage"].GetString());
		std::string offIconImage = HELPERS::resolvePath(b.value["offIconImage"].GetString());
		std::string keybindName  = b.value["triggerKeybind"].GetString();
		// Get the gtk keyvalue from a gtk function

		std::shared_ptr<ButtonInfo> thisButtonInfo = std::make_shared<ButtonInfo>();

		thisButtonInfo->scriptName = scriptName;
		thisButtonInfo->normalName = normalName;
		thisButtonInfo->viewName   = viewName;

		thisButtonInfo->onIcon = std::make_shared<wxImage>();
		thisButtonInfo->onIcon->LoadFile(onIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->offIcon = std::make_shared<wxImage>();
		thisButtonInfo->offIcon->LoadFile(offIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->onBitmapIcon  = std::make_shared<wxBitmap>(*thisButtonInfo->onIcon, wxBITMAP_SCREEN_DEPTH);
		thisButtonInfo->offBitmapIcon = std::make_shared<wxBitmap>(*thisButtonInfo->offIcon, wxBITMAP_SCREEN_DEPTH);

		int listWidth  = (*mainSettings)["inputsList"]["imageWidth"].GetInt();
		int listHeight = (*mainSettings)["inputsList"]["imageHeight"].GetInt();
		int gridWidth  = (*mainSettings)["buttonGrid"]["imageWidth"].GetInt();
		int gridHeight = (*mainSettings)["buttonGrid"]["imageHeight"].GetInt();

		thisButtonInfo->resizedListOnBitmap  = std::make_shared<wxBitmap>(thisButtonInfo->onIcon->Rescale(listWidth, listHeight));
		thisButtonInfo->resizedListOffBitmap = std::make_shared<wxBitmap>(thisButtonInfo->offIcon->Rescale(listWidth, listHeight));
		thisButtonInfo->resizedGridOnBitmap  = std::make_shared<wxBitmap>(thisButtonInfo->onIcon->Rescale(gridWidth, gridHeight));
		thisButtonInfo->resizedGridOffBitmap = std::make_shared<wxBitmap>(thisButtonInfo->offIcon->Rescale(gridWidth, gridHeight));

		// Only one char
		// This is because functions listen for a single char
		// WxWidgets returns the raw char, so this can be used
		thisButtonInfo->toggleKeybind = keybindName.at(0);

		buttonMapping[chosenButton] = thisButtonInfo;
	}
}
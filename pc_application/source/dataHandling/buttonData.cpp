#include "buttonData.hpp"
#include <memory>

void ButtonData::setupButtonMapping(rapidjson::Document* mainSettings) {
	// Set up button mapping with the data obtained via JSON
	for(auto& b : (*mainSettings)["buttons"].GetObject()) {
		Btn chosenButton = stringToButton[b.name.GetString()];

		std::string scriptName   = b.value["scriptName"].GetString();
		std::string viewName     = b.value["viewName"].GetString();
		std::string onIconImage  = b.value["onIconImage"].GetString();
		std::string offIconImage = b.value["offIconImage"].GetString();
		std::string keybindName  = b.value["triggerKeybind"].GetString();
		// Get the gtk keyvalue from a gtk function

		std::shared_ptr<ButtonInfo> thisButtonInfo = std::make_shared<ButtonInfo>();

		thisButtonInfo->scriptName = scriptName;
		thisButtonInfo->viewName   = viewName;

		thisButtonInfo->onIcon = std::make_shared<wxImage>();
		thisButtonInfo->onIcon->LoadFile(onIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->offIcon = std::make_shared<wxImage>();
		thisButtonInfo->offIcon->LoadFile(offIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->onBitmapIcon = std::make_shared<wxBitmap>(thisButtonInfo->onIcon.get(), wxBITMAP_SCREEN_DEPTH);

		thisButtonInfo->offBitmapIcon = std::make_shared<wxBitmap>(thisButtonInfo->offIcon.get(), wxBITMAP_SCREEN_DEPTH);

		// Only one char
		// This is because functions listen for a single char
		// WxWidgets returns the raw char, so this can be used
		thisButtonInfo->toggleKeybind = keybindName.at(0);

		buttonMapping[chosenButton] = thisButtonInfo;
	}
}
#include "buttonData.hpp"

void ButtonData::maskifyBitmap(wxBitmap* bitmap, wxColour maskColor) {
	bitmap->SetMask(new wxMask(*bitmap, maskColor));
}

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
		int gridX                = b.value["gridX"].GetInt();
		int gridY                = b.value["gridY"].GetInt();
		// Get the gtk keyvalue from a gtk function

		wxColour maskColor;
		maskColor.Set((*mainSettings)["iconTransparent"].GetString());

		std::shared_ptr<ButtonInfo> thisButtonInfo = std::make_shared<ButtonInfo>();

		thisButtonInfo->scriptName = scriptName;
		thisButtonInfo->normalName = normalName;
		thisButtonInfo->viewName   = viewName;

		thisButtonInfo->onIcon = new wxImage();
		thisButtonInfo->onIcon->LoadFile(onIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->offIcon = new wxImage();
		thisButtonInfo->offIcon->LoadFile(offIconImage, wxBITMAP_TYPE_PNG);

		thisButtonInfo->onBitmapIcon  = new wxBitmap(*thisButtonInfo->onIcon, wxBITMAP_SCREEN_DEPTH);
		thisButtonInfo->offBitmapIcon = new wxBitmap(*thisButtonInfo->offIcon, wxBITMAP_SCREEN_DEPTH);

		// This is how transparency is supported
		maskifyBitmap(thisButtonInfo->onBitmapIcon, maskColor);
		maskifyBitmap(thisButtonInfo->offBitmapIcon, maskColor);

		int listWidth  = (*mainSettings)["inputsList"]["imageWidth"].GetInt();
		int listHeight = (*mainSettings)["inputsList"]["imageHeight"].GetInt();
		int gridWidth  = (*mainSettings)["buttonGrid"]["imageWidth"].GetInt();
		int gridHeight = (*mainSettings)["buttonGrid"]["imageHeight"].GetInt();

		thisButtonInfo->resizedListOnBitmap  = new wxBitmap(thisButtonInfo->onIcon->Rescale(listWidth, listHeight));
		thisButtonInfo->resizedListOffBitmap = new wxBitmap(thisButtonInfo->offIcon->Rescale(listWidth, listHeight));
		thisButtonInfo->resizedGridOnBitmap  = new wxBitmap(thisButtonInfo->onIcon->Rescale(gridWidth, gridHeight));
		thisButtonInfo->resizedGridOffBitmap = new wxBitmap(thisButtonInfo->offIcon->Rescale(gridWidth, gridHeight));

		maskifyBitmap(thisButtonInfo->resizedListOnBitmap, maskColor);
		maskifyBitmap(thisButtonInfo->resizedListOffBitmap, maskColor);
		maskifyBitmap(thisButtonInfo->resizedGridOnBitmap, maskColor);
		maskifyBitmap(thisButtonInfo->resizedGridOffBitmap, maskColor);

		// Only one char
		// This is because functions listen for a single char
		// WxWidgets returns the raw char, so this can be used
		thisButtonInfo->toggleKeybind = keybindName.at(0);

		thisButtonInfo->gridX = gridX;
		thisButtonInfo->gridY = gridY;

		buttonMapping[chosenButton] = thisButtonInfo;
	}
}

void ButtonData::transferControllerData(std::shared_ptr<ControllerData> src, std::shared_ptr<ControllerData> dest) {
	// Transfer all over

	dest->index      = src->index;
	dest->buttons    = src->buttons;
	dest->LS_X       = src->LS_X;
	dest->LS_Y       = src->LS_Y;
	dest->RS_X       = src->RS_X;
	dest->RS_Y       = src->RS_Y;
	dest->ACCEL_X    = src->ACCEL_X;
	dest->ACCEL_Y    = src->ACCEL_Y;
	dest->ACCEL_Z    = src->ACCEL_Z;
	dest->GYRO_1     = src->GYRO_1;
	dest->GYRO_2     = src->GYRO_2;
	dest->GYRO_3     = src->GYRO_3;
	dest->frameState = src->frameState;
}
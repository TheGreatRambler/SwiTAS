#include "buttonData.hpp"
#include <string>

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

		// For later
		scriptNameToButton[scriptName] = chosenButton;

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

void ButtonData::textToFrames(std::vector<std::shared_ptr<ControllerData>>& frames, std::string text, FrameNum startLoc, bool insertPaste, bool placePaste) {
	std::vector<std::string> frameParts = HELPERS::splitString(text, '\n');
	bool haveSetFirstFrame              = false;
	FrameNum firstFrame;
	FrameNum lastReadFrame;
	for(std::string frame : frameParts) {
		// Split on whitespace
		std::vector<std::string> parts = HELPERS::splitString(frame, ' ');
		// This is to allow a variable number of parts
		uint8_t currentIndexInParts = 0;

		// This is repeated to break function when it runs out of parts, allows variable
		// Amounts of data per line
		if(parts.size() == currentIndexInParts)
			continue;

		// Has to be decimal number
		FrameNum frame = std::stoi(parts[currentIndexInParts], nullptr, 10);

		if(!haveSetFirstFrame) {
			// This is the first script frame, it will be put at the startLoc
			firstFrame = frame;
		}

		// The actual index
		FrameNum actualIndex = startLoc + (frame - firstFrame);

		std::shared_ptr<ControllerData> thisData;
		if(insertPaste) {
			// It's a new one
			thisData = std::make_shared<ControllerData>();
			// Loop through and add frames to keep the right offset between this frame
			// And the last one
			if(haveSetFirstFrame) {
				// Can use expected indexing
				for(FrameNum i = lastReadFrame + 1; i < actualIndex; i++) {
					// Add a blank frame for buffer
					inputsList.insert(inputsList.begin() + i, std::make_shared<ControllerData>());
				}
				// Now can easily add the frame later
			}
			// Insert it now, it's just a pointer
			frames.insert(inputsList.begin() + actualIndex, thisData);
			// If it's the first frame, no need for padding
		} else {
			// Set the reference to the right one
			thisData = frames[actualIndex];
		}

		// Now can set the variable
		haveSetFirstFrame = true;
		// Have reference, so we can now do this
		lastReadFrame = actualIndex;

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Deal with buttons
		uint32_t buttonInfo;
		for(std::string buttonName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
			if(scriptNameToButton.count(buttonName)) {
				SET_BIT(buttonInfo, true, scriptNameToButton[buttonName])
			}
		}
		// placePaste doesn't replace the value
		if(placePaste) {
			// Just OR it
			thisData->buttons |= buttonInfo;
		} else {
			thisData->buttons = buttonInfo;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Joysticks
		std::vector<std::string> joystickPartsLeft = HELPERS::splitString(parts[currentIndexInParts], ';');
		thisData->LS_X                             = std::stoi(joystickPartsLeft[0], nullptr, 10);
		thisData->LS_Y                             = std::stoi(joystickPartsLeft[1], nullptr, 10);

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> joystickPartsRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		thisData->RS_X                              = std::stoi(joystickPartsRight[0], nullptr, 10);
		thisData->RS_Y                              = std::stoi(joystickPartsRight[1], nullptr, 10);

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Accelerometer and gyro data

		std::vector<std::string> accelParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		thisData->ACCEL_X                   = std::stoi(accelParts[0], nullptr, 10);
		thisData->ACCEL_Y                   = std::stoi(accelParts[1], nullptr, 10);
		thisData->ACCEL_Z                   = std::stoi(accelParts[2], nullptr, 10);

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> gyroParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		thisData->GYRO_1                   = std::stoi(gyroParts[0], nullptr, 10);
		thisData->GYRO_2                   = std::stoi(gyroParts[1], nullptr, 10);
		thisData->GYRO_3                   = std::stoi(gyroParts[2], nullptr, 10);
	}
}

std::string ButtonData::framesToText(std::vector<std::shared_ptr<ControllerData>>& frames, FrameNum startLoc, FrameNum endLoc) {
	std::vector<std::string> textVector;
	// Loop through each frame and convert it
	for(FrameNum i = startLoc; i <= endLoc; i++) {
		std::vector<std::string> parts;
		parts[0] = std::to_string(i);

		std::shared_ptr<ControllerData> frame = frames[i];

		std::vector<std::string> buttonParts;
		for(uint8_t i = 0; i < Btn::BUTTONS_SIZE; i++) {
			Btn button = (Btn)i;
			if(GET_BIT(frame->buttons, button)) {
				// Add to the string
				buttonParts.push_back(buttonMapping[button]->scriptName);
			}
		}
		parts[1] = HELPERS::joinString(buttonParts, ";");

		parts[2] = std::to_string(frame->LS_X) + ";" + std::to_string(frame->LS_Y);
		parts[3] = std::to_string(frame->ACCEL_X) + ";" + std::to_string(frame->ACCEL_Y) + ";" + std::to_string(frame->ACCEL_Z);
		parts[4] = std::to_string(frame->GYRO_1) + ";" + std::to_string(frame->GYRO_2) + ";" + std::to_string(frame->GYRO_3);

		textVector.push_back(HELPERS::joinString(parts, " "));
	}
	// Return with newlines
	return HELPERS::joinString(textVector, "\n");
}

void ButtonData::transferControllerData(std::shared_ptr<ControllerData> src, std::shared_ptr<ControllerData> dest, bool placePaste) {
	// Transfer all over

	dest->index = src->index;
	if(placePaste) {
		// Add them together, not replace (bitwise or)
		dest->buttons |= src->buttons;
	} else {
		// Just replace
		dest->buttons = src->buttons;
	}
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
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
		FrameNum frameNum = strtol(parts[currentIndexInParts].c_str(), nullptr, 10);

		if(!haveSetFirstFrame) {
			// This is the first script frame, it will be put at the startLoc
			firstFrame = frameNum;
		}

		// The actual index
		FrameNum actualIndex = startLoc + (frameNum - firstFrame);

		if(actualIndex >= frames.size()) {
			// Have to return, there are too many frames to paste
			return;
		}

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
					frames.insert(frames.begin() + i, std::make_shared<ControllerData>());
				}
				// Now can easily add the frame later
			}
			// Insert it now, it's just a pointer
			frames.insert(frames.begin() + actualIndex + 1, thisData);
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
		uint32_t buttonInfo = 0;
		// Can be no buttons at all
		if(parts[currentIndexInParts] != "NONE") {
			for(std::string buttonName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
				if(scriptNameToButton.count(buttonName)) {
					SET_BIT(buttonInfo, true, scriptNameToButton[buttonName]);
				}
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
		if(joystickPartsLeft.size() == 2) {
			thisData->LS_X = strtol(joystickPartsLeft[0].c_str(), nullptr, 10);
			thisData->LS_Y = strtol(joystickPartsLeft[1].c_str(), nullptr, 10);
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> joystickPartsRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(joystickPartsRight.size() == 2) {
			thisData->RS_X = strtol(joystickPartsRight[0].c_str(), nullptr, 10);
			thisData->RS_Y = strtol(joystickPartsRight[1].c_str(), nullptr, 10);
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Accelerometer and gyro data

		std::vector<std::string> accelParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(accelParts.size() == 3) {
			thisData->ACCEL_X = strtol(accelParts[0].c_str(), nullptr, 10);
			thisData->ACCEL_Y = strtol(accelParts[1].c_str(), nullptr, 10);
			thisData->ACCEL_Z = strtol(accelParts[2].c_str(), nullptr, 10);
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> gyroParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(gyroParts.size() == 3) {
			thisData->GYRO_1 = strtol(gyroParts[0].c_str(), nullptr, 10);
			thisData->GYRO_2 = strtol(gyroParts[1].c_str(), nullptr, 10);
			thisData->GYRO_3 = strtol(gyroParts[2].c_str(), nullptr, 10);
		} else {
			continue;
		}
	}
}

std::string ButtonData::framesToText(std::vector<std::shared_ptr<ControllerData>>& frames, FrameNum startLoc, FrameNum endLoc) {
	std::vector<std::string> textVector;
	// Loop through each frame and convert it
	for(FrameNum i = startLoc; i <= endLoc; i++) {
		std::shared_ptr<ControllerData> frame = frames[i];

		// Keeping empty ones there clutters things
		if(!isEmptyControllerData(frame)) {
			std::vector<std::string> parts;
			parts.push_back(std::to_string(i));

			std::vector<std::string> buttonParts;
			for(uint8_t i = 0; i < Btn::BUTTONS_SIZE; i++) {
				Btn button = (Btn)i;
				if(GET_BIT(frame->buttons, button)) {
					// Add to the string
					buttonParts.push_back(buttonMapping[button]->scriptName);
				}
			}

			if(buttonParts.size() == 0) {
				// Sometimes, it's nothing, so push a constant
				parts.push_back("NONE");
			} else {
				parts.push_back(HELPERS::joinString(buttonParts, ";"));
			}

			parts.push_back(std::to_string(frame->LS_X) + ";" + std::to_string(frame->LS_Y));
			parts.push_back(std::to_string(frame->RS_X) + ";" + std::to_string(frame->RS_Y));

			parts.push_back(std::to_string(frame->ACCEL_X) + ";" + std::to_string(frame->ACCEL_Y) + ";" + std::to_string(frame->ACCEL_Z));
			parts.push_back(std::to_string(frame->GYRO_1) + ";" + std::to_string(frame->GYRO_2) + ";" + std::to_string(frame->GYRO_3));

			textVector.push_back(HELPERS::joinString(parts, " "));
		}
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

bool ButtonData::isEmptyControllerData(std::shared_ptr<ControllerData> data) {
	ControllerData emptyData;
	// clang-format off
	return
		(data->index == emptyData.index) &&
		(data->buttons == emptyData.buttons) &&
		(data->LS_X       == emptyData.LS_X) &&
		(data->LS_Y       == emptyData.LS_Y) &&
		(data->RS_X       == emptyData.RS_X) &&
		(data->RS_Y       == emptyData.RS_Y) &&
		(data->ACCEL_X    == emptyData.ACCEL_X) &&
		(data->ACCEL_Y    == emptyData.ACCEL_Y) &&
		(data->ACCEL_Z    == emptyData.ACCEL_Z) &&
		(data->GYRO_1     == emptyData.GYRO_1) &&
		(data->GYRO_2     == emptyData.GYRO_2) &&
		(data->GYRO_3     == emptyData.GYRO_3) &&
		(data->frameState == emptyData.frameState);
	// clang-format on
}
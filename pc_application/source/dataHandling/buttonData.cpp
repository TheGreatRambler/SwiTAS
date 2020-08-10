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

		thisButtonInfo->resizedGridOnBitmap  = new wxBitmap(thisButtonInfo->onIcon->Copy().Rescale(gridWidth, gridHeight));
		thisButtonInfo->resizedGridOffBitmap = new wxBitmap(thisButtonInfo->offIcon->Copy().Rescale(gridWidth, gridHeight));
		thisButtonInfo->resizedListOnBitmap  = new wxBitmap(thisButtonInfo->onIcon->Copy().Rescale(listWidth, listHeight));
		thisButtonInfo->resizedListOffBitmap = new wxBitmap(thisButtonInfo->offIcon->Copy().Rescale(listWidth, listHeight));

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

FrameNum ButtonData::textToFrames(DataProcessing* dataProcessing, std::string text, FrameNum startLoc, bool insertPaste, bool placePaste) {
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

		// Just to keep the app running
		if(actualIndex % 50 == 0) {
			wxTheApp->Yield();
		}

		FrameNum thisDataIndex;

		if(insertPaste || actualIndex >= dataProcessing->getFramesSize()) {
			if(actualIndex >= dataProcessing->getFramesSize()) {
				actualIndex--;
			}
			// It's a new one
			// Loop through and add frames to keep the right offset between this frame
			// And the last one
			if(haveSetFirstFrame) {
				// Can use expected indexing
				for(FrameNum i = lastReadFrame + 1; i < actualIndex; i++) {
					// Add a blank frame for buffer
					dataProcessing->addFrame(i - 1);
				}
				// Now can easily add the frame later
			}
			// Insert it now, it's just a pointer
			dataProcessing->addFrame(actualIndex);
			thisDataIndex = actualIndex + 1;
			// If it's the first frame, no need for padding
		} else {
			thisDataIndex = actualIndex;
		}

		// Now can set the variable
		haveSetFirstFrame = true;
		// Have reference, so we can now do this
		lastReadFrame = actualIndex;

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		if(!placePaste) {
			// Clear the buttons if no place paste
			dataProcessing->clearAllButtons(thisDataIndex);
		}

		// Deal with buttons
		// Can be no buttons at all
		if(parts[currentIndexInParts] != "NONE") {
			for(std::string buttonName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
				if(scriptNameToButton.count(buttonName)) {
					dataProcessing->modifyButton(thisDataIndex, scriptNameToButton[buttonName], true);
				}
			}
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Joysticks
		std::vector<std::string> joystickPartsLeft = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(joystickPartsLeft.size() == 2) {
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::LEFT_X, strtol(joystickPartsLeft[0].c_str(), nullptr, 10));
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::LEFT_Y, strtol(joystickPartsLeft[1].c_str(), nullptr, 10));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> joystickPartsRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(joystickPartsRight.size() == 2) {
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::RIGHT_X, strtol(joystickPartsRight[0].c_str(), nullptr, 10));
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::RIGHT_Y, strtol(joystickPartsRight[1].c_str(), nullptr, 10));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		// Accelerometer and gyro data, both left and right

		std::vector<std::string> accelLeftParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(accelLeftParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_X_LEFT, strtof(accelLeftParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_Y_LEFT, strtof(accelLeftParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_Z_LEFT, strtof(accelLeftParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> gyroLeftParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(gyroLeftParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_X_LEFT, strtof(gyroLeftParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_Y_LEFT, strtof(gyroLeftParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_Z_LEFT, strtof(gyroLeftParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> angleLeftParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(angleLeftParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_X_LEFT, strtof(angleLeftParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_Y_LEFT, strtof(angleLeftParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_Z_LEFT, strtof(angleLeftParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> accelRightParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(accelRightParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_X_RIGHT, strtof(accelRightParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_Y_RIGHT, strtof(accelRightParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ACCEL_Z_RIGHT, strtof(accelRightParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> gyroRightParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(gyroRightParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_X_RIGHT, strtof(gyroRightParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_Y_RIGHT, strtof(gyroRightParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::GYRO_Z_RIGHT, strtof(gyroRightParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> angleRightParts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(angleRightParts.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_X_RIGHT, strtof(angleRightParts[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_Y_RIGHT, strtof(angleRightParts[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::ANGLE_Z_RIGHT, strtof(angleRightParts[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> numberOfTouches = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(numberOfTouches.size() == 1) {
			dataProcessing->setNumberOfTouches(thisDataIndex, strtol(numberOfTouches[0].c_str(), nullptr, 10));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> touch1Parts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(touch1Parts.size() == 2) {
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::TOUCH_X_1, strtol(touch1Parts[0].c_str(), nullptr, 10));
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::TOUCH_Y_1, strtol(touch1Parts[1].c_str(), nullptr, 10));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> touch2Parts = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(touch2Parts.size() == 2) {
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::TOUCH_X_2, strtol(touch2Parts[0].c_str(), nullptr, 10));
			dataProcessing->setNumberValuesJoystick(thisDataIndex, ControllerNumberValues::TOUCH_Y_2, strtol(touch2Parts[1].c_str(), nullptr, 10));
		} else {
			continue;
		}
	}

	return lastReadFrame;
}

std::string ButtonData::framesToText(DataProcessing* dataProcessing, FrameNum startLoc, FrameNum endLoc, int playerIndex, BranchNum branch) {
	// If the player index is provided, get every savestate hook in that player
	std::vector<std::string> textVector;
	// Loop through each frame and convert it
	SavestateBlockNum start;
	SavestateBlockNum end;
	if(playerIndex == -1) {
		start = dataProcessing->getCurrentSavestateHook();
		end   = dataProcessing->getCurrentSavestateHook() + 1;
	} else {
		start = 0;
		end   = dataProcessing->getNumOfSavestateHooks(playerIndex);
	}

	FrameNum indexForAllSavestateHooks = 0;
	for(SavestateBlockNum j = start; j < end; j++) {
		if(playerIndex != -1) {
			startLoc = 0;
			endLoc   = dataProcessing->getNumOfFramesInSavestateHook(j, playerIndex) - 1;
		}

		for(FrameNum i = startLoc; i <= endLoc; i++) {
			// Just to keep the app running
			if(i % 50 == 0) {
				wxTheApp->Yield();
			}

			// Keeping empty ones there clutters things
			if(!isEmptyControllerData(dataProcessing->getFrame(i))) {
				std::vector<std::string> parts;

				if(playerIndex == -1) {
					parts.push_back(std::to_string(i));
				} else {
					parts.push_back(std::to_string(indexForAllSavestateHooks));
				}

				std::vector<std::string> buttonParts;
				for(uint8_t btn = 0; btn < Btn::BUTTONS_SIZE; btn++) {
					Btn button = (Btn)btn;
					uint8_t isSelected;
					if(playerIndex == -1) {
						isSelected = dataProcessing->getButton(i, button);
					} else {
						isSelected = dataProcessing->getButtonSpecific(i, button, j, branch, playerIndex);
					}
					if(isSelected) {
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

				typedef ControllerNumberValues CNV;

				uint8_t realPlayer;
				if(playerIndex == -1) {
					realPlayer = dataProcessing->getCurrentPlayer();
				} else {
					realPlayer = playerIndex;
				}

				// clang-format off
				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificJoystick(i, CNV::LEFT_X, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificJoystick(i, CNV::LEFT_Y, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificJoystick(i, CNV::RIGHT_X, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificJoystick(i, CNV::RIGHT_Y, j,branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_X_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_Y_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_Z_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_X_LEFT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_Y_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_Z_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_X_LEFT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_Y_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_Z_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberOfTouchesSpecific(i, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberOfTouchesSpecific(i, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberOfTouchesSpecific(i, j, branch, realPlayer)));
				// clang-format on

				textVector.push_back(HELPERS::joinString(parts, " "));
			}

			indexForAllSavestateHooks++;
		}
	}
	// Return with newlines
	return HELPERS::joinString(textVector, "\n");
}

void ButtonData::transferControllerData(ControllerData src, std::shared_ptr<ControllerData> dest, bool placePaste) {
	// Transfer all over

	if(placePaste) {
		// Add them together, not replace (bitwise or)
		dest->buttons |= src.buttons;
	} else {
		// Just replace
		dest->buttons = src.buttons;
	}
	dest->numberOfTouches = src.numberOfTouches;
	dest->LS_X            = src.LS_X;
	dest->LS_Y            = src.LS_Y;
	dest->RS_X            = src.RS_X;
	dest->RS_Y            = src.RS_Y;
	dest->TOUCH_X_1       = src.TOUCH_X_1;
	dest->TOUCH_Y_1       = src.TOUCH_Y_1;
	dest->TOUCH_X_2       = src.TOUCH_X_2;
	dest->TOUCH_Y_2       = src.TOUCH_Y_2;
	dest->ACCEL_X_LEFT    = src.ACCEL_X_LEFT;
	dest->ACCEL_Y_LEFT    = src.ACCEL_Y_LEFT;
	dest->ACCEL_Z_LEFT    = src.ACCEL_Z_LEFT;
	dest->GYRO_X_LEFT     = src.GYRO_X_LEFT;
	dest->GYRO_Y_LEFT     = src.GYRO_Y_LEFT;
	dest->GYRO_Z_LEFT     = src.GYRO_Z_LEFT;
	dest->ANGLE_X_LEFT    = src.ANGLE_X_LEFT;
	dest->ANGLE_Y_LEFT    = src.ANGLE_Y_LEFT;
	dest->ANGLE_Z_LEFT    = src.ANGLE_Z_LEFT;
	dest->ACCEL_X_RIGHT   = src.ACCEL_X_RIGHT;
	dest->ACCEL_Y_RIGHT   = src.ACCEL_Y_RIGHT;
	dest->ACCEL_Z_RIGHT   = src.ACCEL_Z_RIGHT;
	dest->GYRO_X_RIGHT    = src.GYRO_X_RIGHT;
	dest->GYRO_Y_RIGHT    = src.GYRO_Y_RIGHT;
	dest->GYRO_Z_RIGHT    = src.GYRO_Z_RIGHT;
	dest->ANGLE_X_RIGHT   = src.ANGLE_X_RIGHT;
	dest->ANGLE_Y_RIGHT   = src.ANGLE_Y_RIGHT;
	dest->ANGLE_Z_RIGHT   = src.ANGLE_Z_RIGHT;
	dest->frameState      = src.frameState;
}

bool ButtonData::isEmptyControllerData(std::shared_ptr<ControllerData> data) {
	ControllerData emptyData;
	// clang-format off
	return
		(data->buttons    == emptyData.buttons)   &&
		(data->numberOfTouches    == emptyData.numberOfTouches)   &&
		(data->LS_X       == emptyData.LS_X)      &&
		(data->LS_Y       == emptyData.LS_Y)      &&
		(data->RS_X       == emptyData.RS_X)      &&
		(data->RS_Y       == emptyData.RS_Y)      &&
		(data->TOUCH_X_1       == emptyData.TOUCH_X_1)      &&
		(data->TOUCH_Y_1       == emptyData.TOUCH_Y_1)      &&
		(data->TOUCH_X_2       == emptyData.TOUCH_X_2)      &&
		(data->TOUCH_Y_2       == emptyData.TOUCH_Y_2)      &&
		(data->ACCEL_X    == emptyData.ACCEL_X)   &&
		(data->ACCEL_Y    == emptyData.ACCEL_Y)   &&
		(data->ACCEL_Z    == emptyData.ACCEL_Z)   &&
		(data->GYRO_X     == emptyData.GYRO_X)    &&
		(data->GYRO_Y     == emptyData.GYRO_Y)    &&
		(data->GYRO_Z     == emptyData.GYRO_Z)    &&
		(data->ANGLE_X     == emptyData.ANGLE_X)  &&
		(data->ANGLE_Y     == emptyData.ANGLE_Y)  &&
		(data->ANGLE_Z     == emptyData.ANGLE_Z)  &&
		(data->frameState == emptyData.frameState);
	// clang-format on
}
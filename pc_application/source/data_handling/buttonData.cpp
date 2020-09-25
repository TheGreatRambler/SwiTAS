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

	for(auto const& keyboardKey : stringToKeyboardKey) {
		keyboardKeyToString[keyboardKey.second] = keyboardKey.first;
	}

	for(auto const& keyboardModifier : stringToKeyboardModifier) {
		keyboardModifierToString[keyboardModifier.second] = keyboardModifier.first;
	}

	for(auto const& mouseButton : stringToMouseButton) {
		mouseButtonToString[mouseButton.second] = mouseButton.first;
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

		// clang-format off
		// FULL DATA SPECIFICATION
		// FRAME KEYS JOY_L_X;JOY_L_Y JOY_R_X;JOY_R_Y ACCEL_L_X;ACCEL_L_Y;ACCEL_L_Z GYRO_L_X;GYRO_L_Y;GYRO_L_Z ANGLE_L_X;ANGLE_L_Y;ANGLE_L_Z ACCEL_R_X;ACCEL_R_Y;ACCEL_R_Z GYRO_R_X;GYRO_R_Y;GYRO_R_Z ANGLE_R_X;ANGLE_R_Y;ANGLE_R_Z DIRECTION_L_XX;DIRECTION_L_XY;DIRECTION_L_XZ DIRECTION_L_YX;DIRECTION_L_YY;DIRECTION_L_YZ DIRECTION_L_ZX;DIRECTION_L_ZY;DIRECTION_L_ZZ DIRECTION_R_XX;DIRECTION_R_XY;DIRECTION_R_XZ DIRECTION_R_YX;DIRECTION_R_YY;DIRECTION_R_YZ DIRECTION_R_ZX;DIRECTION_R_ZY;DIRECTION_R_ZZ
		// clang-format on

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

		// DirectionState parts, left and right
		std::vector<std::string> directionXLeft = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionXLeft.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XX_LEFT, strtof(directionXLeft[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XY_LEFT, strtof(directionXLeft[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XZ_LEFT, strtof(directionXLeft[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> directionYLeft = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionYLeft.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YX_LEFT, strtof(directionYLeft[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YY_LEFT, strtof(directionYLeft[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YZ_LEFT, strtof(directionYLeft[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> directionZLeft = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionZLeft.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZX_LEFT, strtof(directionZLeft[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZY_LEFT, strtof(directionZLeft[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZZ_LEFT, strtof(directionZLeft[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> directionXRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionXRight.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XX_RIGHT, strtof(directionXRight[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XY_RIGHT, strtof(directionXRight[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_XZ_RIGHT, strtof(directionXRight[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> directionYRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionYRight.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YX_RIGHT, strtof(directionYRight[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YY_RIGHT, strtof(directionYRight[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_YZ_RIGHT, strtof(directionYRight[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		std::vector<std::string> directionZRight = HELPERS::splitString(parts[currentIndexInParts], ';');
		if(directionZRight.size() == 3) {
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZX_RIGHT, strtof(directionZRight[0].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZY_RIGHT, strtof(directionZRight[1].c_str(), nullptr));
			dataProcessing->setNumberValuesMotion(thisDataIndex, ControllerNumberValues::DIRECTION_ZZ_RIGHT, strtof(directionZRight[2].c_str(), nullptr));
		} else {
			continue;
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		if(!placePaste) {
			// Clear the buttons if no place paste
			dataProcessing->clearAllKeyboardButtons(thisDataIndex);
		}

		// Deal with buttons
		// Can be no buttons at all
		if(parts[currentIndexInParts] != "NONE") {
			for(std::string keyboardKeyName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
				if(stringToKeyboardKey.count(keyboardKeyName)) {
					dataProcessing->setKeyboardButton(thisDataIndex, stringToKeyboardKey[keyboardKeyName], true);
				}
			}
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		if(!placePaste) {
			dataProcessing->clearAllKeyboardModifiers(thisDataIndex);
		}

		if(parts[currentIndexInParts] != "NONE") {
			for(std::string keyboardModifierName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
				if(stringToKeyboardModifier.count(keyboardModifierName)) {
					dataProcessing->setKeyboardModifier(thisDataIndex, stringToKeyboardModifier[keyboardModifierName], true);
				}
			}
		}

		currentIndexInParts++;
		if(parts.size() == currentIndexInParts)
			continue;

		if(!placePaste) {
			dataProcessing->clearAllMouseButtons(thisDataIndex);
		}

		if(parts[currentIndexInParts] != "NONE") {
			for(std::string mouseButtonName : HELPERS::splitString(parts[currentIndexInParts], ';')) {
				if(stringToMouseButton.count(mouseButtonName)) {
					dataProcessing->setMouseButton(thisDataIndex, stringToMouseButton[mouseButtonName], true);
				}
			}
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
			if(!isEmptyControllerData(dataProcessing->getFrame(i)) || !isEmptyExtraData(dataProcessing->getFrameExtra(i))) {
				std::vector<std::string> parts;

				if(playerIndex == -1) {
					parts.push_back(std::to_string(i));
				} else {
					parts.push_back(std::to_string(indexForAllSavestateHooks));
				}

				std::vector<std::string> buttonParts;
				for(uint8_t btn = 0; btn < (uint8_t)Btn::BUTTONS_SIZE; btn++) {
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

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_X_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_Y_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ACCEL_Z_RIGHT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_X_RIGHT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_Y_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::GYRO_Z_RIGHT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_X_RIGHT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_Y_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::ANGLE_Z_RIGHT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XX_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XY_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XZ_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YX_LEFT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YY_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YZ_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZX_LEFT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZY_LEFT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZZ_LEFT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XX_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XY_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_XZ_RIGHT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YX_RIGHT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YY_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_YZ_RIGHT, j, branch, realPlayer)));

				parts.push_back(std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZX_RIGHT, j,branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZY_RIGHT, j, branch, realPlayer)) + \
					";" + std::to_string(dataProcessing->getNumberValuesSpecificMotion(i, CNV::DIRECTION_ZZ_RIGHT, j, branch, realPlayer)));
				// clang-format on

				parts.push_back(getKeyboardKeysString(dataProcessing->getControllerDataExtra(j, branch, i)));
				parts.push_back(getKeyboardModifiersString(dataProcessing->getControllerDataExtra(j, branch, i)));
				parts.push_back(getMouseButtonsString(dataProcessing->getControllerDataExtra(j, branch, i)));

				textVector.push_back(HELPERS::joinString(parts, " "));
			}

			indexForAllSavestateHooks++;
		}
	}
	// Return with newlines
	return HELPERS::joinString(textVector, "\n");
}

void ButtonData::transferControllerData(ControllerData src, std::shared_ptr<ControllerData> dest, bool placePaste) {
	if(placePaste) {
		// Add them together, not replace (bitwise or)
		dest->buttons |= src.buttons;
	} else {
		// Just replace
		dest->buttons = src.buttons;
	}
	dest->LS_X               = src.LS_X;
	dest->LS_Y               = src.LS_Y;
	dest->RS_X               = src.RS_X;
	dest->RS_Y               = src.RS_Y;
	dest->ACCEL_X_LEFT       = src.ACCEL_X_LEFT;
	dest->ACCEL_Y_LEFT       = src.ACCEL_Y_LEFT;
	dest->ACCEL_Z_LEFT       = src.ACCEL_Z_LEFT;
	dest->GYRO_X_LEFT        = src.GYRO_X_LEFT;
	dest->GYRO_Y_LEFT        = src.GYRO_Y_LEFT;
	dest->GYRO_Z_LEFT        = src.GYRO_Z_LEFT;
	dest->ANGLE_X_LEFT       = src.ANGLE_X_LEFT;
	dest->ANGLE_Y_LEFT       = src.ANGLE_Y_LEFT;
	dest->ANGLE_Z_LEFT       = src.ANGLE_Z_LEFT;
	dest->ACCEL_X_RIGHT      = src.ACCEL_X_RIGHT;
	dest->ACCEL_Y_RIGHT      = src.ACCEL_Y_RIGHT;
	dest->ACCEL_Z_RIGHT      = src.ACCEL_Z_RIGHT;
	dest->GYRO_X_RIGHT       = src.GYRO_X_RIGHT;
	dest->GYRO_Y_RIGHT       = src.GYRO_Y_RIGHT;
	dest->GYRO_Z_RIGHT       = src.GYRO_Z_RIGHT;
	dest->ANGLE_X_RIGHT      = src.ANGLE_X_RIGHT;
	dest->ANGLE_Y_RIGHT      = src.ANGLE_Y_RIGHT;
	dest->ANGLE_Z_RIGHT      = src.ANGLE_Z_RIGHT;
	dest->DIRECTION_XX_LEFT  = src.DIRECTION_XX_LEFT;
	dest->DIRECTION_XY_LEFT  = src.DIRECTION_XY_LEFT;
	dest->DIRECTION_XZ_LEFT  = src.DIRECTION_XZ_LEFT;
	dest->DIRECTION_YX_LEFT  = src.DIRECTION_YX_LEFT;
	dest->DIRECTION_YY_LEFT  = src.DIRECTION_YY_LEFT;
	dest->DIRECTION_YZ_LEFT  = src.DIRECTION_YZ_LEFT;
	dest->DIRECTION_ZX_LEFT  = src.DIRECTION_ZX_LEFT;
	dest->DIRECTION_ZY_LEFT  = src.DIRECTION_ZY_LEFT;
	dest->DIRECTION_ZZ_LEFT  = src.DIRECTION_ZZ_LEFT;
	dest->DIRECTION_XX_RIGHT = src.DIRECTION_XX_RIGHT;
	dest->DIRECTION_XY_RIGHT = src.DIRECTION_XY_RIGHT;
	dest->DIRECTION_XZ_RIGHT = src.DIRECTION_XZ_RIGHT;
	dest->DIRECTION_YX_RIGHT = src.DIRECTION_YX_RIGHT;
	dest->DIRECTION_YY_RIGHT = src.DIRECTION_YY_RIGHT;
	dest->DIRECTION_YZ_RIGHT = src.DIRECTION_YZ_RIGHT;
	dest->DIRECTION_ZX_RIGHT = src.DIRECTION_ZX_RIGHT;
	dest->DIRECTION_ZY_RIGHT = src.DIRECTION_ZY_RIGHT;
	dest->DIRECTION_ZZ_RIGHT = src.DIRECTION_ZZ_RIGHT;
	dest->frameState         = src.frameState;
}

void ButtonData::transferExtraData(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest, bool placePaste) {
	if(placePaste) {
		// Add them together, not replace (bitwise or)
		dest->keyboardModifiers |= src.keyboardModifiers;
		dest->mouseButtons |= src.mouseButtons;
		dest->keyboardKeys[0] |= src.keyboardKeys[0];
		dest->keyboardKeys[1] |= src.keyboardKeys[1];
		dest->keyboardKeys[2] |= src.keyboardKeys[2];
		dest->keyboardKeys[3] |= src.keyboardKeys[3];
		dest->keyboardKeys[4] |= src.keyboardKeys[4];
		dest->keyboardKeys[5] |= src.keyboardKeys[5];
		dest->keyboardKeys[6] |= src.keyboardKeys[6];
		dest->keyboardKeys[7] |= src.keyboardKeys[7];
	} else {
		// Just replace
		dest->keyboardModifiers = src.keyboardModifiers;
		dest->mouseButtons      = src.mouseButtons;
		memcpy(dest->keyboardKeys, src.keyboardKeys, 8);
	}

	dest->touchX1         = src.touchX1;
	dest->touchY1         = src.touchY1;
	dest->touchX2         = src.touchX2;
	dest->touchY2         = src.touchY2;
	dest->numberOfTouches = src.numberOfTouches;
	dest->mouseX          = src.mouseX;
	dest->mouseY          = src.mouseY;
	dest->mouseVelocityX  = src.mouseVelocityX;
	dest->mouseVelocityY  = src.mouseVelocityY;
	dest->scrollVelocityX = src.scrollVelocityX;
	dest->scrollVelocityY = src.scrollVelocityY;
}

void ButtonData::transferOnlyKeyboard(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest) {
	dest->keyboardModifiers = src.keyboardModifiers;
	dest->mouseButtons      = src.mouseButtons;
	memcpy(dest->keyboardKeys, src.keyboardKeys, 8);
	dest->mouseX          = src.mouseX;
	dest->mouseY          = src.mouseY;
	dest->mouseVelocityX  = src.mouseVelocityX;
	dest->mouseVelocityY  = src.mouseVelocityY;
	dest->scrollVelocityX = src.scrollVelocityX;
	dest->scrollVelocityY = src.scrollVelocityY;
}

void ButtonData::transferOnlyTouch(TouchAndKeyboardData src, std::shared_ptr<TouchAndKeyboardData> dest) {
	dest->touchX1         = src.touchX1;
	dest->touchY1         = src.touchY1;
	dest->touchX2         = src.touchX2;
	dest->touchY2         = src.touchY2;
	dest->numberOfTouches = src.numberOfTouches;
}

bool ButtonData::isEmptyControllerData(std::shared_ptr<ControllerData> data) {
	ControllerData emptyData;
	// clang-format off
	return
		(data->buttons            == emptyData.buttons)            &&
		(data->LS_X               == emptyData.LS_X)               &&
		(data->LS_Y               == emptyData.LS_Y)               &&
		(data->RS_X               == emptyData.RS_X)               &&
		(data->RS_Y               == emptyData.RS_Y)               &&
		(data->ACCEL_X_LEFT       == emptyData.ACCEL_X_LEFT)       &&
		(data->ACCEL_Y_LEFT       == emptyData.ACCEL_Y_LEFT)       &&
		(data->ACCEL_Z_LEFT       == emptyData.ACCEL_Z_LEFT)       &&
		(data->GYRO_X_LEFT        == emptyData.GYRO_X_LEFT)        &&
		(data->GYRO_Y_LEFT        == emptyData.GYRO_Y_LEFT)        &&
		(data->GYRO_Z_LEFT        == emptyData.GYRO_Z_LEFT)        &&
		(data->ANGLE_X_LEFT       == emptyData.ANGLE_X_LEFT)       &&
		(data->ANGLE_Y_LEFT       == emptyData.ANGLE_Y_LEFT)       &&
		(data->ANGLE_Z_LEFT       == emptyData.ANGLE_Z_LEFT)       &&
		(data->ACCEL_X_RIGHT      == emptyData.ACCEL_X_RIGHT)      &&
		(data->ACCEL_Y_RIGHT      == emptyData.ACCEL_Y_RIGHT)      &&
		(data->ACCEL_Z_RIGHT      == emptyData.ACCEL_Z_RIGHT)      &&
		(data->GYRO_X_RIGHT       == emptyData.GYRO_X_RIGHT)       &&
		(data->GYRO_Y_RIGHT       == emptyData.GYRO_Y_RIGHT)       &&
		(data->GYRO_Z_RIGHT       == emptyData.GYRO_Z_RIGHT)       &&
		(data->ANGLE_X_RIGHT      == emptyData.ANGLE_X_RIGHT)      &&
		(data->ANGLE_Y_RIGHT      == emptyData.ANGLE_Y_RIGHT)      &&
		(data->ANGLE_Z_RIGHT      == emptyData.ANGLE_Z_RIGHT)      &&
		(data->DIRECTION_XX_LEFT  == emptyData.DIRECTION_XX_LEFT)  &&
		(data->DIRECTION_XY_LEFT  == emptyData.DIRECTION_XY_LEFT)  &&
		(data->DIRECTION_XZ_LEFT  == emptyData.DIRECTION_XZ_LEFT)  &&
		(data->DIRECTION_YX_LEFT  == emptyData.DIRECTION_YX_LEFT)  &&
		(data->DIRECTION_YY_LEFT  == emptyData.DIRECTION_YY_LEFT)  &&
		(data->DIRECTION_YZ_LEFT  == emptyData.DIRECTION_YZ_LEFT)  &&
		(data->DIRECTION_ZX_LEFT  == emptyData.DIRECTION_ZX_LEFT)  &&
		(data->DIRECTION_ZY_LEFT  == emptyData.DIRECTION_ZY_LEFT)  &&
		(data->DIRECTION_ZZ_LEFT  == emptyData.DIRECTION_ZZ_LEFT)  &&
		(data->DIRECTION_XX_RIGHT == emptyData.DIRECTION_XX_RIGHT) &&
		(data->DIRECTION_XY_RIGHT == emptyData.DIRECTION_XY_RIGHT) &&
		(data->DIRECTION_XZ_RIGHT == emptyData.DIRECTION_XZ_RIGHT) &&
		(data->DIRECTION_YX_RIGHT == emptyData.DIRECTION_YX_RIGHT) &&
		(data->DIRECTION_YY_RIGHT == emptyData.DIRECTION_YY_RIGHT) &&
		(data->DIRECTION_YZ_RIGHT == emptyData.DIRECTION_YZ_RIGHT) &&
		(data->DIRECTION_ZX_RIGHT == emptyData.DIRECTION_ZX_RIGHT) &&
		(data->DIRECTION_ZY_RIGHT == emptyData.DIRECTION_ZY_RIGHT) &&
		(data->DIRECTION_ZZ_RIGHT == emptyData.DIRECTION_ZZ_RIGHT) &&
		(data->frameState      == emptyData.frameState);
	// clang-format on
}

bool ButtonData::isEmptyExtraData(std::shared_ptr<TouchAndKeyboardData> data) {
	TouchAndKeyboardData emptyData;
	// clang-format off
	return
		(data->touchX1           == emptyData.touchX1) &&
		(data->touchY1           == emptyData.touchY1) &&
		(data->touchX2           == emptyData.touchX2) &&
		(data->touchY2           == emptyData.touchY2) &&
		(data->numberOfTouches   == emptyData.numberOfTouches) &&
		(data->keyboardModifiers == emptyData.keyboardModifiers) &&
		(memcmp(data->keyboardKeys, emptyData.keyboardKeys, 8) == 0) &&
		(data->mouseX            == emptyData.mouseX) &&
		(data->mouseY            == emptyData.mouseY) &&
		(data->mouseVelocityX    == emptyData.mouseVelocityX) &&
		(data->mouseVelocityY    == emptyData.mouseVelocityY) &&
		(data->scrollVelocityX   == emptyData.scrollVelocityX) &&
		(data->scrollVelocityY   == emptyData.scrollVelocityY) &&
		(data->mouseButtons      == emptyData.mouseButtons);
	// clang-format on
}

std::string ButtonData::getKeyboardKeysString(std::shared_ptr<TouchAndKeyboardData> data) {
	std::vector<std::string> keys;
	for(auto const& key : stringToKeyboardModifier) {
		bool isHeld = IS_KEYBOARD_HELD(data->keyboardKeys, (int32_t)key.second);
		if(isHeld) {
			keys.push_back(key.first);
		}
	}
	if(keys.size() == 0) {
		return "NONE";
	} else {
		return HELPERS::joinString(keys, ";");
	}
}

std::string ButtonData::getKeyboardModifiersString(std::shared_ptr<TouchAndKeyboardData> data) {
	std::vector<std::string> keys;
	for(auto const& key : stringToKeyboardModifier) {
		bool isHeld = data->keyboardModifiers & (uint8_t)key.second;
		if(isHeld) {
			keys.push_back(key.first);
		}
	}
	if(keys.size() == 0) {
		return "NONE";
	} else {
		return HELPERS::joinString(keys, ";");
	}
}

std::string ButtonData::getMouseButtonsString(std::shared_ptr<TouchAndKeyboardData> data) {
	std::vector<std::string> keys;
	for(auto const& key : stringToMouseButton) {
		bool isHeld = data->mouseButtons & (uint8_t)key.second;
		if(isHeld) {
			keys.push_back(key.first);
		}
	}
	if(keys.size() == 0) {
		return "NONE";
	} else {
		return HELPERS::joinString(keys, ";");
	}
}
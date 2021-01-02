#include "controller.hpp"

ControllerHandler::ControllerHandler(
	std::shared_ptr<CommunicateWithNetwork> networkImp, uint8_t startingID) {
	networkInstance      = networkImp;
	controllerStartingID = startingID;

#ifdef __SWITCH__
	deviceLeftJoycon.deviceType = HidDeviceType_JoyLeft4;

	deviceLeftJoycon.npadInterfaceType = NpadInterfaceType_Bluetooth;

	deviceLeftJoycon.singleColorBody    = RGBA8_MAXALPHA(0x00, 0x00, 0xFF);
	deviceLeftJoycon.singleColorButtons = RGBA8_MAXALPHA(0xFF, 0x00, 0x00);
	deviceLeftJoycon.colorLeftGrip      = deviceLeftJoycon.singleColorBody;
	deviceLeftJoycon.colorRightGrip     = deviceLeftJoycon.singleColorBody;

	stateLeftJoycon.batteryCharge = 4;

	stateLeftJoycon.buttons                      = 0;
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx  = 0;
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy  = 0;
	stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dx = 0;
	stateLeftJoycon.joysticks[JOYSTICK_RIGHT].dy = 0;

	rc = hiddbgAttachHdlsVirtualDevice(
		&hdlsHandleLeftJoycon, &deviceLeftJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);

	rc = hiddbgSetHdlsState(hdlsHandleLeftJoycon, &stateLeftJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);

	deviceRightJoycon.deviceType = HidDeviceType_JoyRight5;

	deviceRightJoycon.npadInterfaceType = NpadInterfaceType_Bluetooth;

	deviceRightJoycon.singleColorBody    = RGBA8_MAXALPHA(0x00, 0x00, 0xFF);
	deviceRightJoycon.singleColorButtons = RGBA8_MAXALPHA(0xFF, 0x00, 0x00);
	deviceRightJoycon.colorLeftGrip      = deviceRightJoycon.singleColorBody;
	deviceRightJoycon.colorRightGrip     = deviceRightJoycon.singleColorBody;

	stateRightJoycon.batteryCharge = 4;

	stateRightJoycon.buttons                      = 0;
	stateRightJoycon.joysticks[JOYSTICK_LEFT].dx  = 0;
	stateRightJoycon.joysticks[JOYSTICK_LEFT].dy  = 0;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dx = 0;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dy = 0;

	rc = hiddbgAttachHdlsVirtualDevice(
		&hdlsHandleRightJoycon, &deviceRightJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);

	rc = hiddbgSetHdlsState(hdlsHandleRightJoycon, &stateRightJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);

#endif

#ifdef YUZU
	id = (PluginDefinitions::ControllerNumber)controllerStartingID;

	yuzu_joypad_enablejoypad(yuzuInstance, id, true);
	yuzu_joypad_setjoypadtype(
		yuzuInstance, id, PluginDefinitions::ControllerType::JoyDual);
#endif
}

void ControllerHandler::setFrame(ControllerData& controllerData) {
	clearState();
// Set data one at a time
#ifdef __SWITCH__
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx   = controllerData.LS_X;
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy   = controllerData.LS_Y;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dx = controllerData.RS_X;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dy = controllerData.RS_Y;
	for(auto const& button : btnToHidKeys) {
		if(GET_BIT(controllerData.buttons, (uint8_t)button.first)) {
			stateLeftJoycon.buttons |= button.second;
		}
	}
#endif

#ifdef YUZU
	yuzu_joypad_setjoystick(yuzuInstance, id,
		PluginDefinitions::YuzuJoystickType::LeftX, controllerData.LS_X);
	yuzu_joypad_setjoystick(yuzuInstance, id,
		PluginDefinitions::YuzuJoystickType::LeftY, controllerData.LS_Y);
	yuzu_joypad_setjoystick(yuzuInstance, id,
		PluginDefinitions::YuzuJoystickType::RightX, controllerData.RS_X);
	yuzu_joypad_setjoystick(yuzuInstance, id,
		PluginDefinitions::YuzuJoystickType::RightY, controllerData.RS_Y);
	uint64_t buttons = 0;
	for(auto const& button : btnToHidKeys) {
		if(GET_BIT(controllerData.buttons, (uint8_t)button.first)) {
			SET_BIT(buttons, true, (uint8_t)button.second);
		}
	}
	yuzu_joypad_set(yuzuInstance, id, buttons);
#endif

	setInput();
}

void ControllerHandler::setFrame(uint64_t buttons, int32_t leftX, int32_t leftY,
	int32_t rightX, int32_t rightY) {
	clearState();

#ifdef __SWITCH__
	stateLeftJoycon.buttons                       = buttons;
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx   = leftX;
	stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy   = leftY;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dx = rightX;
	stateRightJoycon.joysticks[JOYSTICK_RIGHT].dy = rightY;
#endif

#ifdef YUZU
	yuzu_joypad_setjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::LeftX, leftX);
	yuzu_joypad_setjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::LeftY, leftY);
	yuzu_joypad_setjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::RightX, rightX);
	yuzu_joypad_setjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::RightY, rightY);
	yuzu_joypad_set(yuzuInstance, id, buttons);
#endif

	setInput();
}

std::shared_ptr<ControllerData> ControllerHandler::getControllerData() {
	std::shared_ptr<ControllerData> newControllerData
		= std::make_shared<ControllerData>();
#ifdef __SWITCH__
	for(auto const& button : btnToHidKeys) {
		if(stateLeftJoycon.buttons & button.second) {
			SET_BIT(newControllerData->buttons, true, (uint8_t)button.first);
		} else {
			SET_BIT(newControllerData->buttons, false, (uint8_t)button.first);
		}
	}
#endif

#ifdef YUZU
	uint64_t buttons = yuzu_joypad_read(yuzuInstance, id);
	for(auto const& button : btnToHidKeys) {
		if(GET_BIT(buttons, (uint8_t)button.second)) {
			SET_BIT(newControllerData->buttons, true, (uint8_t)button.first);
		} else {
			SET_BIT(newControllerData->buttons, false, (uint8_t)button.first);
		}
	}
#endif

#ifdef __SWITCH__
	newControllerData->LS_X = stateLeftJoycon.joysticks[JOYSTICK_LEFT].dx;
	newControllerData->LS_Y = stateLeftJoycon.joysticks[JOYSTICK_LEFT].dy;
	newControllerData->RS_X = stateRightJoycon.joysticks[JOYSTICK_RIGHT].dx;
	newControllerData->RS_Y = stateRightJoycon.joysticks[JOYSTICK_RIGHT].dy;
#endif

#ifdef YUZU
	newControllerData->LS_X = yuzu_joypad_readjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::LeftX);
	newControllerData->LS_Y = yuzu_joypad_readjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::LeftY);
	newControllerData->RS_X = yuzu_joypad_readjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::RightX);
	newControllerData->RS_Y = yuzu_joypad_readjoystick(
		yuzuInstance, id, PluginDefinitions::YuzuJoystickType::RightY);
#endif

	newControllerData->frameState = 0;

	return newControllerData;
}

ControllerHandler::~ControllerHandler() {
// Detatch Controller
#ifdef __SWITCH__
	rc = hiddbgDetachHdlsVirtualDevice(hdlsHandleLeftJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);

	rc = hiddbgDetachHdlsVirtualDevice(hdlsHandleRightJoycon);
	if(R_FAILED(rc))
		fatalThrow(rc);
#endif

#ifdef YUZU
	yuzu_joypad_enablejoypad(yuzuInstance,
		(PluginDefinitions::ControllerNumber)controllerStartingID, false);
#endif
}
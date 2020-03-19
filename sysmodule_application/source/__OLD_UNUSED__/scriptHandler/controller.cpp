#include "controller.hpp"
#include "script_init.hpp"
#include "script_populator.hpp"
#include "script_provider.hpp"

TasController::TasController(uint8_t deviceType, uint8_t bodyR, uint8_t bodyG,
                             uint8_t bodyB, uint8_t buttonR, uint8_t buttonG,
                             uint8_t buttonB) {
  device.deviceType = deviceType;

  // Set the interface type
  device.npadInterfaceType = NpadInterfaceType_Bluetooth;

  // Colors
  device.singleColorBody = RGBA8_MAXALPHA(bodyR, bodyG, bodyB);
  device.singleColorButtons = RGBA8_MAXALPHA(buttonR, buttonG, buttonB);
  device.colorLeftGrip = device.singleColorBody;
  device.colorRightGrip = device.singleColorBody;

  // Charge is max
  state.batteryCharge = 4;

  // Set Buttons and Joysticks
  state.buttons = 0;
  state.joysticks[JOYSTICK_LEFT].dx = 0;
  state.joysticks[JOYSTICK_LEFT].dy = 0;
  state.joysticks[JOYSTICK_RIGHT].dx = 0;
  state.joysticks[JOYSTICK_RIGHT].dy = 0;

  // Attach the controller
  Result rc = hiddbgAttachHdlsVirtualDevice(&HdlsHandle, &device);
  if (R_FAILED(rc))
    fatalThrow(rc);

  // Update the state
  rc = hiddbgSetHdlsState(HdlsHandle, &state);
  if (R_FAILED(rc))
    fatalThrow(rc);
}

TasController::~TasController() {
  // Detatch Controller
  Result rc = hiddbgDetachHdlsVirtualDevice(HdlsHandle);
  if (R_FAILED(rc))
    fatalThrow(rc);
}

// This also resets the state of the controller after pressing so only to be
// used when pairing and not running a script
void TasController::pressA() { runScript<PressAProvider>(); }

// This also resets the state of the controller after pressing so only to be
// used when pairing and not running a script
void TasController::pressLR() { runScript<PressLRProvider>(); }

void TasController::waitForVsync() {
  Result rc = eventWait(&vsync_event, U64_MAX);
  if (R_FAILED(rc))
    fatalThrow(rc);
}
void TasController::setInputNextFrame() {
  waitForVsync();
  Result rc = hiddbgSetHdlsState(HdlsHandle, &state);
  if (R_FAILED(rc))
    fatalThrow(rc);
}

void TasController::runMsg(std::shared_ptr<struct controlMsg> msg) {
  state.buttons = msg->keys;
  state.joysticks[JOYSTICK_LEFT].dx = msg->joy_l_x;
  state.joysticks[JOYSTICK_LEFT].dy = msg->joy_l_y;
  state.joysticks[JOYSTICK_RIGHT].dx = msg->joy_r_x;
  state.joysticks[JOYSTICK_RIGHT].dy = msg->joy_r_y;
}
void TasController::emptyMsg() {
  state.buttons = 0;
  state.joysticks[JOYSTICK_LEFT].dx = 0;
  state.joysticks[JOYSTICK_LEFT].dy = 0;
  state.joysticks[JOYSTICK_RIGHT].dx = 0;
  state.joysticks[JOYSTICK_RIGHT].dy = 0;
}

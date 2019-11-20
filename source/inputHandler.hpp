extern "C" {
#include <switch.h>
}

struct ControllerData {
    // This controller's index
    u8 index;
    // Button data
    bool A = false;
    bool B = false;
    bool X = false;
    bool Y = false;
    bool L = false;
    bool R = false;
    bool ZL = false;
    bool ZR = false;
    bool DUP = false;
    bool DDOWN = false;
    bool DLEFT = false;
    bool DRIGHT = false;
    bool PLUS = false;
    bool MINUS = false;
    bool HOME = false;
    bool CAPT = false;
    bool LS = false;
    bool RS = false;
    // Joystick values
    short int LS_X = 0;
    short int LS_Y = 0;
    short int RS_X = 0;
    short int RS_Y = 0;
    // Gyroscope and Accelerometer data (when it is implemented)
    s16 ACCEL_X = 0;
    s16 ACCEL_Y = 0;
    s16 ACCEL_Z = 0;
    s16 GYRO_1 = 0;
    s16 GYRO_2 = 0;
    s16 GYRO_3 = 0;
}

class InputHandler {
    private:
    // Controller data for all possible controllers
    ControllerData controllers[8];
    // Number of currently emulated controllers
    constexpr u8 numOfControllers = 0;
    public:
    InputHandler() {
        // Initialize all the controllers with their index
        for (int i = 0; i < 8; i++) {
            controllers[i].index = i;
        }
    }

    ControllerData* getController(u8 controllerIndex) {
        // Get the pointer to the controller specified by the index
        return &controllers[controllerIndex];
    }

    void addController() {
        numOfControllers++;
    }

    void removeController() {
        numOfControllers--;
    }
}
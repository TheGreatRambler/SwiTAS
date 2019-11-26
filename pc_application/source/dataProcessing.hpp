#pragma once

// A: Char itself
// B: Index
// C: Value of bit
#define BIT_SET(a,b,c) (a ^= (-(unsigned long)c ^ a) & (1UL << b))

#include <cstdint>
#include <cstdlib>
#include <vector>

// Buttons enum (coencides with the index of the bit in the input struct)
enum Btn {
    A,
    B,
    X,
    Y,
    L,
    R,
    ZL,
    ZR,
    SL,
    SR,
    DUP,
    DDOWN,
    DLEFT,
    DRIGHT,
    PLUS,
    MINUS,
    HOME,
    CAPT,
    LS,
    RS,
};

// Controller data that will be packed into the array and will be recieved from the switch
struct ControllerData {
    // This controller's index
    uint8_t index;
    // Button data (they are bits stored inside of chars to save space)
    uint8_t firstBlock;
    uint8_t secondBlock;
    uint8_t thirdBlock;
    // Joystick values
    int16_t LS_X = 0;
    int16_t LS_Y = 0;
    int16_t RS_X = 0;
    int16_t RS_Y = 0;
    // Gyroscope and Accelerometer data (when it is implemented)
    int16_t ACCEL_X = 0;
    int16_t ACCEL_Y = 0;
    int16_t ACCEL_Z = 0;
    int16_t GYRO_1 = 0;
    int16_t GYRO_2 = 0;
    int16_t GYRO_3 = 0;
} __attribute__((__packed__));

class DataProcessing {
    private:
    // Vector storing ALL inputs
    std::vector<ControllerData*> inputsList;
    // Current input
    ControllerData* currentData;

    public:
    DataProcessing() {
        currentData = new ControllerData();
    }

    void setButtonState(Btn button, bool state) {
        // If state is true, on, else off
        if (button < 8) {
            // First group
            BIT_SET(currentData->firstBlock, button, state);
        } else if (button < 16) {
            // Second group
            uint8_t temp = button - 8;
            BIT_SET(currentData->secondBlock, temp, state);
        } else {
            // Last group
            uint8_t temp = button - 16;
            BIT_SET(currentData->thirdBlock, temp, state);
        }
    }

    ~DataProcessing() {
        // Dereference all the pointers in the vector
        for(ControllerData* thisFrame : inputsList) {
            // Remember, the input struct must ALWAYS BE DESTROYED
            delete thisFrame;
        }
    }
}
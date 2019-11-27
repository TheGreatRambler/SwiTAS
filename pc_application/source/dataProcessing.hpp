#pragma once

// A: Char itself
// B: Index
// C: Value of bit
#define BIT_SET(a,b,c) (a ^= (-(unsigned long)c ^ a) & (1UL << b))

// A: Char itself
// B: Index
#define BIT_GET(a,b) ((a >> b) & 1U)

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

class InputColumns : public Gtk::TreeModelColumnRecord {
    public:
    Gtk::TreeModelColumn<uint32_t> frameNum;
    // Repeat them buttons
    // https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en
    Gtk::TreeModelColumn<Gdk::Pixbuf> button1;

    InputColumns() {
        add(frameNum);
        add(button1);
    }
}

class DataProcessing {
    private:
    // Vector storing ALL inputs
    // Shared pointer so everything is nice
    std::vector<std::shared_ptr<ControllerData>> inputsList;
    // Current input
    std::shared_ptr<ControllerData> currentData;
    // Current frame
    uint32_t currentFrame;
    // Tree data storing the controller stuffs
    Glib::RefPtr<Gtk::ListStore> controllerListStore;
    // Stores the columns for the above list store
    InputColumns inputColumns;

    public:
    DataProcessing() {
        // Add columns
        controllerListStoreColumns->add(frameNum);
        controllerListStoreColumns->add(button1);
        //Add the list store from the columns
        controllerListStore = Gtk::ListStore::create(inputColumns);
        // Add this first frame
        addNewFrame(true);
    }

    bool getButtonState(Btn button) {
        if (button < 8) {
            // First group
            return BIT_GET(currentData->firstBlock, button);
        } else if (button < 16) {
            // Second group
            uint8_t temp = button - 8;
            return BIT_GET(currentData->secondBlock, temp);
        } else {
            // Last group
            uint8_t temp = button - 16;
            return BIT_GET(currentData->thirdBlock, temp);
        }
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

    void setCurrentFrame(uint32_t frameNum) {
        
    }

    void addNewFrame(bool isFirstFrame = false) {
        if (!isFirstFrame) {
            // On the first frame, it is already set right
            currentFrame++;
        }
        // Will overwrite previous frame if need be
        currentData = std::make_shared<ControllerData>();
        // Add this to the vector
        inputsList.push_back(currentData);
    }

    ~DataProcessing() {}
}
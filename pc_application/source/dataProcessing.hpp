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
#include <utility>
#include <vector>
#include <map>
#include <tuple>
#include <string>

// Buttons enum (coencides with the index of the bit in the input struct)
enum class Btn {
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

auto getNewColumn() {
    return new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>();
}

auto getNewIcon(std::string name) {
    // https://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
    return Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/22x22/apps/" + name + ".png");
}

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button listview object and button printing pixbuf
// TODO finish these
typedef std::tuple<Glib::ustring, Glib::ustring, Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>*, Glib::RefPtr<Gdk::Pixbuf>> ButtonInfoType;
std::map<Btn, ButtonInfoType> buttonMapping;
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::A, ButtonInfoType("BUTTON_A", "A Button", getNewColumn(), getNewIcon(""))));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::B, "BUTTON_B"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::X, "BUTTON_X"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::Y, "BUTTON_Y"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::L, "BUTTON_L"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::R, "BUTTON_R"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::ZL, "BUTTON_ZL"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::ZR, "BUTTON_ZR"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::SL, "BUTTON_SL"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::SR, "BUTTON_SR"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::DUP, "BUTTON_DUP"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::DDOWN, "BUTTON_DDOWN"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::DLEFT, "BUTTON_DLEFT"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::DRIGHT, "BUTTON_DRIGHT"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::PLUS, "BUTTON_PLUS"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::MINUS, "BUTTON_MINUS"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::HOME, "BUTTON_HOME"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::CAPT, "BUTTON_CAPT"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::LS, "BUTTON_LS"));
buttonMapping.insert(std::pair<Btn, ButtonInfoType>(Btn::RS, "BUTTON_RS"));



class InputColumns : public Gtk::TreeModelColumnRecord {
    public:
    Gtk::TreeModelColumn<uint32_t> frameNum;
    // All the buttons are stored inside of buttonMapping
    // https://developer.gnome.org/gtkmm-tutorial/stable/sec-treeview-examples.html.en

    InputColumns() {
        add(frameNum);
        // Loop through the buttons and add them
        for (auto const& button : buttonMapping) {
            // Gets pointer from tuple
            Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* thisIcon = std::get<2>(button.second);
            // Add to the columns themselves (gives value, not pointer)
            add(*thisIcon);
        }

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
    // Tree view viewed in the UI
    Gtk::TreeView treeView;

    public:
    DataProcessing() {
        //Add the list store from the columns
        controllerListStore = Gtk::ListStore::create(inputColumns);
        // Set this tree view to this model
        treeView.set_model(controllerListStore);
        // Add all the columns, this somehow wasn't done already
        treeView.append_column("Frame", inputColumns.frameNum);
        // Loop through buttons and add all of them
        for (auto const& thisButton : inputColumns.buttons) {
            // Append with the string specified by Button Mapping
            treeView.append_column(buttonMapping[thisButton.first], *thisButton.second);
        }
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
        // Must be a frame that has already been written, else, raise error
        if (frameNum < inputsList.size() && frameNum > -1) {
            // Set the current frame to this frame
            // Shared pointer so this can be done
            currentData = inputsList[frameNum];
        }
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
#pragma once

#include <bitset>
#include <map>
#include <gtkmm.h>

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
    // Button data (stored as a bitset because it will be serialized better later)
    // 20 buttons
    // This storage can be sacrificed for the simple reason that the other data already takes up so much space
    std::bitset<20> buttons;
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
}

// Struct containing button info
struct ButtonInfo {
    Glib::ustring scriptName;
    Glib::ustring viewName;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* columnIcon;
    Glib::RefPtr<Gdk::Pixbuf> bottomInputViewIcon;
    Glib::RefPtr<Gdk::Pixbuf> offIcon;
}

Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* getNewColumn() {
    return new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>();
}

Glib::RefPtr<Gdk::Pixbuf> getNewIcon(std::string name) {
    // https://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
    return Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/22x22/apps/" + name + ".png");
}

ButtonInfo* getButtonInfo(Glib::ustring scriptName,
        Glib::ustring viewName,
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* columnIcon,
        Glib::RefPtr<Gdk::Pixbuf> viewIcon,
        Glib::RefPtr<Gdk::Pixbuf> offIcon) {
    ButtonInfo* thisButtonInfo = new ButtonInfo();
    thisButtonInfo->scriptName = scriptName;
    thisButtonInfo->viewName = viewName;
    thisButtonInfo->columnIcon = columnIcon;
    thisButtonInfo->bottomInputViewIcon = viewIcon;
    thisButtonInfo->offIcon = viewIcon;
    return thisButtonInfo;
}

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button listview object, Button printing pixbuf when on and when off
// This info is contained in a struct
// TODO finish these
std::map<Btn, ButtonInfo*> buttonMapping;
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::A, getButtonInfo("BUTTON_A", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::B, "BUTTON_B"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::X, "BUTTON_X"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::Y, "BUTTON_Y"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::L, "BUTTON_L"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::R, "BUTTON_R"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::ZL, "BUTTON_ZL"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::ZR, "BUTTON_ZR"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::SL, "BUTTON_SL"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::SR, "BUTTON_SR"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::DUP, "BUTTON_DUP"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::DDOWN, "BUTTON_DDOWN"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::DLEFT, "BUTTON_DLEFT"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::DRIGHT, "BUTTON_DRIGHT"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::PLUS, "BUTTON_PLUS"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::MINUS, "BUTTON_MINUS"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::HOME, "BUTTON_HOME"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::CAPT, "BUTTON_CAPT"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::LS, "BUTTON_LS"));
buttonMapping.insert(std::pair<Btn, ButtonInfo*>(Btn::RS, "BUTTON_RS"));
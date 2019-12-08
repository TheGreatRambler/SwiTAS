#pragma once

#include <bitset>
#include <gtkmm.h>
#include <map>
#include <utility>

// Buttons enum (coencides with the index of the bit in the input struct)
// Also used to identify the button everywhere else in the program
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

// Controller data that will be packed into the array and will be recieved from
// the switch
struct ControllerData {
  // This controller's index
  uint8_t index;
  // Button data (stored as a bitset because it will be serialized better later)
  // 20 buttons
  // This storage can be sacrificed for the simple reason that the other data
  // already takes up so much space
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
  Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> *columnIcon;
  Glib::RefPtr<Gdk::Pixbuf> onIcon;
  Glib::RefPtr<Gdk::Pixbuf> offIcon;
}

Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> *
getNewColumn() {
  return new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>();
}

Glib::RefPtr<Gdk::Pixbuf> getNewIcon(std::string name) {
  // https://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
  return Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/22x22/apps/" +
                                       name + ".png");
}

ButtonInfo *gBI(Glib::ustring scriptName, Glib::ustring viewName,
                Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> *columnIcon,
                Glib::RefPtr<Gdk::Pixbuf> viewIcon,
                Glib::RefPtr<Gdk::Pixbuf> offIcon) {
  ButtonInfo *thisButtonInfo = new ButtonInfo();
  thisButtonInfo->scriptName = scriptName;
  thisButtonInfo->viewName = viewName;
  thisButtonInfo->columnIcon = columnIcon;
  thisButtonInfo->onIcon = viewIcon;
  thisButtonInfo->offIcon = offIcon;
  return thisButtonInfo;
}

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button
// listview object, Button printing pixbuf when on and when off This info is
// contained in a struct
// TODO finish these
std::map<Btn, ButtonInfo *> buttonMapping{
    {Btn::A,
     gBI("KEY_A", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::B,
     gBI("KEY_B", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::X,
     gBI("KEY_X", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::Y,
     gBI("KEY_Y", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::L,
     gBI("KEY_L", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::R,
     gBI("KEY_R", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::ZL,
     gBI("KEY_ZL", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::ZR,
     gBI("KEY_ZR", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::SL,
     gBI("KEY_SL", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::SR,
     gBI("KEY_SR", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::DUP, gBI("KEY_DUP", "A Button", getNewColumn(), getNewIcon(""),
                   getNewIcon(""))},
    {Btn::DDOWN, gBI("KEY_DDOWN", "A Button", getNewColumn(), getNewIcon(""),
                     getNewIcon(""))},
    {Btn::DLEFT, gBI("KEY_DLEFT", "A Button", getNewColumn(), getNewIcon(""),
                     getNewIcon(""))},
    {Btn::DRIGHT, gBI("KEY_DRIGHT", "A Button", getNewColumn(), getNewIcon(""),
                      getNewIcon(""))},
    {Btn::PLUS, gBI("KEY_PLUS", "A Button", getNewColumn(), getNewIcon(""),
                    getNewIcon(""))},
    {Btn::MINUS, gBI("KEY_MINUS", "A Button", getNewColumn(), getNewIcon(""),
                     getNewIcon(""))},
    {Btn::HOME, gBI("KEY_HOME", "A Button", getNewColumn(), getNewIcon(""),
                    getNewIcon(""))},
    {Btn::CAPT, gBI("KEY_CAPT", "A Button", getNewColumn(), getNewIcon(""),
                    getNewIcon(""))},
    {Btn::LS,
     gBI("KEY_LS", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
    {Btn::RS,
     gBI("KEY_RS", "A Button", getNewColumn(), getNewIcon(""), getNewIcon(""))},
};
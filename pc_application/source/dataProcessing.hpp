#pragma once

#include <cstdint>
#include <cstdlib>
#include <bitset>
#include <utility>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <gtkmm.h>

#include "buttonData.hpp"

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
            Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* thisIcon = button.second->columnIcon;
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
        for (auto const& thisButton : buttonMapping) {
            // Append with the string specified by Button Mapping
            // Get value of columnIcon, not pointer
            treeView.append_column(thisButton.second->viewName, *thisButton.second->columnIcon);
        }
        // Add this first frame
        addNewFrame(true);
    }

    bool getButtonState(Btn button) {
        // Get value from the bitset
        return currentData->buttons.test(button);
    }

    void setButtonState(Btn button, bool state) {
        // If state is true, on, else off
        return currentData->buttons.set(button, state);
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
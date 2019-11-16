#pragma once

extern "C" {
#include <cstdio>
}

#include "lvgl/lvgl.h"

// Location union
union Loc {
    int x;
    int y;
}

// Maps loosely to this https://cdn.discordapp.com/attachments/404722395845361668/554318002645106700/NintendoExt003_R.png
namespace KeyLetters {
    char A_OFF[] = "\xEE\x80\x80";
    char A_ON[] = "\xEE\x83\xA0";
    char B_OFF[] = "\xEE\x80\x81";
    char B_ON[] = "\xEE\x83\xA1";
    char X_OFF[] = "\xEE\x80\x82";
    char X_ON[] = "\xEE\x83\xA2";
    char Y_OFF[] = "\xEE\x80\x83";
    char Y_ON[] = "\xEE\x83\xA3";
    char L_OFF[] = "\xEE\x80\x84";
    char L_ON[] = "\xEE\x83\xA4";
    char R_OFF[] = "\xEE\x80\x85";
    char R_ON[] = "\xEE\x83\xA5";
};

namespace KeyLocs {
    Loc A = {2, 6};
};

class UIWidgets {
    private:
    // Stuff for the input table
    constexpr int numRows = 12;
    constexpr int numCols = 7;
    // Stuff for the input display table
    constexpr int numRowsInput = 5;
    constexpr int numColsInput = 7;
    // Various UI elements
    lv_obj_t* playButtons;
    lv_obj_t* inputTable;
    lv_obj_t* inputDisplayTable;
    // Joysticks
    constexpr int joystickRadius = 70;
    lv_color_t* leftJoystickBuffer;
    lv_color_t* rightJoystickBuffer;
    lv_obj_t* leftJoystick;
    lv_obj_t* rightJoystick;

    void addTopButtons(lv_obj_t* container) {
        // Top buttons includes Close, Play and Frame Advance
        static const char* buttonString[] = {"X", ">", "F", ""};
        playButtons = lv_btnm_create(container, NULL);
        lv_btnm_set_map(playButtons, buttonString);
        // No toggling
        lv_btnm_set_btn_ctrl_all(playButtons, LV_BTNM_CTRL_NO_REPEAT);
        // I don't want toggling of any buttons, hopefully this does that
        lv_btnm_set_one_toggle(playButtons, false);
    }

    void addInputTable(lv_obj_t* container) {
        inputTable = lv_table_create(container, NULL);
        // Frame then as many inputs as will fit
        lv_table_set_col_cnt(inputTable, numCols);
        // 12 rows is about how many will fit
        lv_table_set_row_cnt(inputTable, numRows);
        for (int i = 0; i < 12; i++) {

        }
        //lv_table_set_col_width(table, 0, LV_DPI / 3);
        //lv_table_set_col_width(table, 1, LV_DPI / 2);
        //lv_table_set_col_width(table, 2, LV_DPI / 2);
    }

    void addJoysticks(lv_obj_t* container) {
        // Rendering part TODO
        leftJoystick = lv_canvas_create(container, NULL);
        rightJoystick = lv_canvas_create(container, NULL);
        // Create buffers
        int sideLength = joystickRadius * 2;
        leftJoystickBuffer = new lv_color_t[LV_CANVAS_BUF_SIZE_TRUE_COLOR(sideLength, sideLength)];
        rightJoystickBuffer = new lv_color_t[LV_CANVAS_BUF_SIZE_TRUE_COLOR(sideLength, sideLength)];
        // Assign buffers to canvas
        lv_canvas_set_buffer(leftJoystick, leftJoystickBuffer, sideLength, sideLength, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_set_buffer(rightJoystick, rightJoystickBuffer, sideLength, sideLength, LV_IMG_CF_TRUE_COLOR);
        // Align them to the correct things
        lv_obj_align(leftJoystick, container, LV_ALIGN_IN_LEFT_MID, 10, 0);
        lv_obj_align(rightJoystick, leftJoystick, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    }

    void setInputDisplayStatus(char* character, Loc location) {
        // Row THEN Column so reversed
        lv_table_set_cell_value(inputDisplayTable, location.y, location.x, character);
    }

    void addInputDisplayToTable(char* character, Loc location) {
        // Row THEN Column so reversed
        lv_table_set_cell_value(inputDisplayTable, location.y, location.x, character);
        // Set alignment
        lv_table_set_cell_align(inputDisplayTable, location.y, location.x, LV_LABEL_ALIGN_CENTER);
    }

    void addInputDisplayTable(lv_obj_t* container) {
        inputDisplayTable = lv_table_create(container, NULL);
        lv_table_set_col_cnt(inputDisplayTable, numColsInput);
        lv_table_set_row_cnt(inputDisplayTable, numRowsInput);
        // Set the location of ALL THE INPUTS
        // All are off
        // TODO repeat for all
        addInputDisplayToTable(KeyLetters::A_OFF, KeyLocs::A);
        // ... code ...
        // Align with joystick view
        lv_obj_align(inputDisplayTable, rightJoystick, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    }

    public:
    void createLeftWidgets(lv_obj_t* leftScr) {
        // Might create container with style later
        addTopButtons(leftScr);
        // Puts the buttons right below the top of the screen
        lv_obj_align(playButtons, leftScr, LV_ALIGN_IN_TOP_MID, 0, 10);
        addInputTable(leftScr);
        // Puts the table right below the buttons
        lv_obj_align(inputTable, playButtons, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }

    void createBottomWidgets(lv_obj_t* bottomScr) {
        // Add joysticks
        addJoysticks(bottomScr);
        // Create a BIG table with all the inputs
        addInputDisplayTable(bottomScr);
    }

    ~UIWidgets() {
        delete [] leftJoystickBuffer;
        delete [] rightJoystickBuffer;
    }
}
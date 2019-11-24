#pragma once

#include <cstdio>

#include "thirdParty/lvgl/lvgl.h"

#include "utils.hpp"

// Location union
union Loc {
	int x;
	int y;
}

// Maps loosely to this https://cdn.discordapp.com/attachments/404722395845361668/554318002645106700/NintendoExt003_R.png
// Use this http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=e0e3&mode=hex
namespace KeyLetters {
	char A_OFF[] = "\xEE\x82\xA0";
	char A_ON[] = "\xEE\x83\xA0";

	char B_OFF[] = "\xEE\x82\xA1";
	char B_ON[] = "\xEE\x83\xA1";

	char X_OFF[] = "\xEE\x82\xA2";
	char X_ON[] = "\xEE\x83\xA2";

	char Y_OFF[] = "\xEE\x82\xA3";
	char Y_ON[] = "\xEE\x83\xA3";

	char L_OFF[] = "\xEE\x82\xA4";
	char L_ON[] = "\xEE\x83\xA4";

	char R_OFF[] = "\xEE\x82\xA5";
	char R_ON[] = "\xEE\x83\xA5";

	char ZL_OFF[] = "\xEE\x82\xA6";
	char ZL_ON[] = "\xEE\x83\xA6";

	char ZR_OFF[] = "\xEE\x82\xA7";
	char ZR_ON[] = "\xEE\x83\xA7";

	char DUP_OFF[] = "\xEE\x82\xAF";
	char DUP_ON[] = "\xEE\x83\xAB";

	char DDOWN_OFF[] = "\xEE\x82\xB0";
	char DDOWN_ON[] = "\xEE\x83\xAC";

	char DLEFT_OFF[] = "\xEE\x82\xB1";
	char DLEFT_ON[] = "\xEE\x83\xAD";

	char DRIGHT_OFF[] = "\xEE\x82\xB2";
	char DRIGHT_ON[] = "\xEE\x83\xAE";

	char PLUS_OFF[] = "\xEE\x82\xB3";
	char PLUS_ON[] = "\xEE\x83\xAF";

	char MINUS_OFF[] = "\xEE\x82\xB4";
	char MINUS_ON[] = "\xEE\x83\xB0";

	char HOME_OFF[] = "\xEE\x82\xB9";
	char HOME_ON[] = "\xEE\x83\xB4";
	
	char CAPT_OFF[] = "\xEE\x82\xBA";
	char CAPT_ON[] = "\xEE\x83\xB5";

	// Press in Left Stick
	char LS_OFF[] = "\xEE\x83\x84";
	char LS_ON[] = "\xEE\x84\x84";

	// Press in Right Stick
	char RS_OFF[] "\xEE\x83\x85";
	char RS_ON[] = "\xEE\x84\x85";
};

// https://www.tablesgenerator.com/text_tables
//      0   1    2    3   4  5   6   7   8    9  10
//   +----+---+----+----+---+--+---+---+----+---+---+
// 0 | LS |   | ZL | L  |   |  |   | R | ZR |   |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 1 |    | ^ |    | -  |   |  |   | + |    | X |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 2 | <  |   | >  |    | C |  | H |   | Y  |   | A |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 3 |    | v |    | RS |   |  |   |   |    | B |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
namespace KeyLocs {
	Loc A = {10, 2};
	Loc B = {9, 3};
	Loc X = {9, 1};
	Loc Y = {8, 2};
	Loc L = {3, 0};
	Loc R = {7, 0};
	Loc ZL = {2, 0};
	Loc ZR = {8, 0};
	Loc DUP = {1, 1};
	Loc DDOWN = {1, 3};
	Loc DLEFT = {0, 2};
	Loc DRIGHT = {2, 2};
	Loc PLUS = {7, 1};
	Loc MINUS = {3, 1};
	Loc HOME = {6, 2};
	Loc CAPT = {4, 2};
	Loc LS = {0, 0};
	Loc RS = {3, 3};
};

class UIWidgets {
	private:
	// Stuff for the input table
	constexpr int numRows = 12;
	constexpr int numCols = 7;
	// Stuff for the input display table
	constexpr int numRowsInput = 4;
	constexpr int numColsInput = 11;
	// Various UI elements
	lv_obj_t* playButtons;
	lv_obj_t* inputTable;
	lv_obj_t* inputDisplayTable;
	// Set inputs button stuff
	lv_obj_t* inputsSetButton;
	lv_obj_t* inputsSetButtonText;
	constexpr char defaultInputsSetButtonText[] = "Set\nInputs";
	// This is in seconds
	constexpr int defaultSetInputsDelay = 3;
	// Joysticks
	constexpr int joystickRadius = 70;
	static lv_style_t joyStyle;
	static lv_style_t joyStyleBall;
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

	void addInputsSetButton(lv_obj_t* container) {
		inputsSetButton = lv_btn_create(container, NULL);
		// Create the label for the button
		inputsSetButtonText = lv_label_create(inputsSetButton, NULL);
		lv_label_set_text(inputsSetButtonText, defaultInputsSetButtonText);
	}

	void clickSetInputsButton(int numOfSeconds) {
		// Count down
		if (numOfSeconds == 0) {
			// Have reached the end, set the current inputs
		} else {
			// Loop around
			setTimeout([numOfSeconds]() {
				numOfSeconds--;
				// Loop around with the incremented value
				clickSetInputsButton(numOfSeconds);
			}, 1000);
		}
	}

	void drawStick(lv_obj_t* joystick, int x, int y) {
		// This can both update and create the joystick
		// x and y are -joystickRadius to joystickRadius
		// Draws the joystick onto it's canvas
		// Clear screen
		int centerX = joystickRadius - 1;
		int centerY = joystickRadius - 1;
		lv_canvas_fill_bg(joystick, LV_COLOR_ORANGE);
		// Draw the circle
		lv_canvas_draw_arc(joystick, centerX, centerY, joystickRadius - 1, 0, 360, &joyStyle);
		// Draw the line from the center to the "ball"
		static lv_point_t linePoints[] = {{centerX, centerY}, {x + centerX, y + centerY}};
		lv_canvas_draw_line(joystick, linePoints, 2, &joyStyle);
		// Draw the "ball"
		// It is filled with black
		lv_canvas_draw_arc(joystick, x + centerX, y + centerY, 15, 0, 360, &joyStyleBall);
	}

	// TODO add manual degree setter underneath
	void addJoysticks(lv_obj_t* container) {
		// Style for joysticks
		lv_style_copy(&joyStyle, &lv_style_plain);
		joyStyle.line.width = 5;
		joyStyle.line.color = LV_COLOR_BLACK;
		joyStyleBall.line.color = LV_COLOR_BLACK;
		joyStyleBall.body.main_color = LV_COLOR_BLACK;
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
		// Add the joysticks
		drawStick(leftJoystick, 0, 0);
		drawStick(rightJoystick, 0, 0);
	}

	void setInputDisplayStatus(char* character, Loc location) {
		// Row THEN Column so reversed
		lv_table_set_cell_value(inputDisplayTable, location.y, location.x, character);
	}

	void addInputDisplayToTable(char* character, Loc location) {
		// Row THEN Column so reversed
		setInputDisplayStatus(character, location);
		// Set alignment so all the text is centered within the grid
		lv_table_set_cell_align(inputDisplayTable, location.y, location.x, LV_LABEL_ALIGN_CENTER);
	}

	void addDefaultInputs() {
		// Add all the inputs
		addInputDisplayToTable(KeyLetters::A_OFF, KeyLocs::A);
		addInputDisplayToTable(KeyLetters::B_OFF, KeyLocs::B);
		addInputDisplayToTable(KeyLetters::X_OFF, KeyLocs::X);
		addInputDisplayToTable(KeyLetters::Y_OFF, KeyLocs::Y);
		addInputDisplayToTable(KeyLetters::L_OFF, KeyLocs::R);
		addInputDisplayToTable(KeyLetters::R_OFF, KeyLocs::R);
		addInputDisplayToTable(KeyLetters::ZL_OFF, KeyLocs::ZL);
		addInputDisplayToTable(KeyLetters::ZR_OFF, KeyLocs::ZR);
		addInputDisplayToTable(KeyLetters::DUP_OFF, KeyLocs::DUP);
		addInputDisplayToTable(KeyLetters::DDOWN_OFF, KeyLocs::DDOWN);
		addInputDisplayToTable(KeyLetters::DLEFT_OFF, KeyLocs::DLEFT);
		addInputDisplayToTable(KeyLetters::DRIGHT_OFF, KeyLocs::DRIGHT);
		addInputDisplayToTable(KeyLetters::PLUS_OFF, KeyLocs::PLUS);
		addInputDisplayToTable(KeyLetters::MINUS_OFF, KeyLocs::MINUS);
		addInputDisplayToTable(KeyLetters::HOME_OFF, KeyLocs::HOME);
		addInputDisplayToTable(KeyLetters::CAPT_OFF, KeyLocs::CAPT);
		addInputDisplayToTable(KeyLetters::LS_OFF, KeyLocs::LS);
		addInputDisplayToTable(KeyLetters::RS_OFF, KeyLocs::RS);
	}
	
	void addInputDisplayTable(lv_obj_t* container) {
		inputDisplayTable = lv_table_create(container, NULL);
		lv_table_set_col_cnt(inputDisplayTable, numColsInput);
		lv_table_set_row_cnt(inputDisplayTable, numRowsInput);
		// Set the location of ALL THE INPUTS
		// All are off
		addDefaultInputs();
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
		addInputsSetButton(leftScr);
		// Put input set button right below the table
		lv_obj_align(inputsSetButton, inputTable, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
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
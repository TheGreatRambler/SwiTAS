#pragma once

extern "C" {
#include <pthread.h>
#include <time.h>
#include <switch.h>
}

#include "lvgl/lvgl.h"

class AppUI {
	private:
	// Pthread that deals with updating Lvgl
	pthread_t updateLvgl
	// Wether to keep looping
	constexpr bool shouldUpdate = true;
	// Milliseconds between updates
	constexpr int sleepMilliseconds = 5;
	// UI Stuff
	// Left side screen
	lv_obj_t* leftScr;
	// Right (actually bottom) screen
	lv_obj_t* rightScr;
	// left size buffer
	lv_disp_drv_t leftDspDrv;
	lv_disp_buf_t leftDispBuf;
	lv_disp_t* leftDisp;
	lv_color_t* leftBuf;
	// Right (Or bottom, made a mistake) size buffer
	lv_disp_drv_t rightDspDrv;
	lv_disp_buf_t rightDispBuf;
	lv_disp_t* rightDisp;
	lv_color_t* rightBuf;
	// Some various variables
	int leftDisplayWidth;
	int leftDisplayHeight;
	int rightDisplayWidth;
	int rightDisplayHeight;
	int leftDisplayX;
	int leftDisplayY;
	int rightDisplayX;
	int rightDisplayY;
	// Wether to Draw
	bool shouldWrite = false;

	void sleepMs(int milliseconds) {
		struct timespec ts;
    	ts.tv_sec = milliseconds / 1000;
    	ts.tv_nsec = (milliseconds % 1000) * 1000000;
    	nanosleep(&ts, NULL);
	}

	void* lvglUpdateLoop() {
		while (shouldUpdate) {
			// Update all lvgl uis with time
			lv_tick_inc(sleepMilliseconds);
			// Update actual UI
            if (shouldUpdate) {
			    lv_task_handler();
            }
			// Sleep for 5 
			sleepMs(sleepMilliseconds);
			// Will now loop around and do it again
		}
		// This while loop will break when shouldUpdate is false
	}

	void bufFlush(lv_disp_t* disp, const lv_area_t* area, lv_color_t * color_p) {
		/*
		if (shouldWrite) {
		int32_t x, y;
		// Start writing
		writeToScreen->startFrame();
    	for(y = area->y1; y <= area->y2; y++) {
    	    for(x = area->x1; x <= area->x2; x++) {
				// Draw pixel to display
				if (disp == leftDisp) {
					// This is the left display
					// Draw pixel
					// Each is uint8_t, or u8
					// color_p->red
					// color_p->green
					// color_p->blue
					// Have to account for offsets
					int xLoc = leftDisplayX + x;
					int yLoc = leftDisplayY + y;
					writeToScreen->setPixel(xLoc, yLoc, color_p->red, color_p->green, color_p->blue);
				} else if (disp == rightDisp) {
					// This is the right display
					// Draw pixel
					int xLoc = rightDisplayX + x;
					int yLoc = rightDisplayY + y;
					writeToScreen->setPixel(xLoc, yLoc, color_p->red, color_p->green, color_p->blue);
				}
    	        color_p++;
    	    }
    	}
		// Flush to framebuffer
		writeToScreen->endFrame();
		*/
		// This function actually does nothing right now because the main code does the rendering
		// Done with flushing
    	lv_disp_flush_ready(disp);
		}
	}

	public:
	AppUI(int leftWidth, int leftHeight, 
		int bottomWidth, int bottomHeight) {
		leftDisplayWidth = leftWidth;
		leftDisplayHeight = leftHeight;
		rightDisplayWidth = bottomWidth;
		rightDisplayHeight = bottomHeight;
		// Create buffers
		leftBuf = new lv_color_t[leftWidth * leftHeight];
		rightBuf = new lv_color_t[bottomWidth * bottomHeight];
		lv_disp_buf_init(&leftDispBuf, leftBuf, NULL, leftWidth * leftHeight);  
		lv_disp_buf_init(&rightDispBuf, rightBuf, NULL, bottomWidth * bottomHeight);
		// Specify some details
		// Left
		lv_disp_drv_init(&leftDspDrv);
		leftDspDrv.flush_cb = my_disp_flush;
		leftDspDrv.buffer = &leftDispBuf;
		leftDspDrv.hor_res = leftWidth;
		leftDspDvr.ver_res = leftHeight;
		leftDisp = lv_disp_drv_register(&leftDspDrv);
		// Right
		lv_disp_drv_init(&rightDspDrv);
		rightDspDrv.flush_cb = my_disp_flush;
		rightDspDrv.buffer = &rightDispBuf;
		leftDspDrv.hor_res = bottomWidth;
		leftDspDvr.ver_res = bottomHeight;
		rightDisp = lv_disp_drv_register(&rightDspDrv);
		// Create LVGL instance
		lv_init();
		// Create UI before starting update
		createUI();
		// Create updating thread and it will start automatically
		pthread_create(&updateLvgl, NULL, lvglUpdateLoop);
	}

	lv_color_t* getLeftBuf() {
		return leftBuf;
	}

	lv_color_t* getBottomBuf() {
		return rightBuf;
	}

	int getLeftWidth() {
		return leftDisplayWidth;
	}

	int getLeftHeight() {
		return leftDisplayHeight;
	}

	int getBottomWidth() {
		return rightDisplayWidth;
	}

	int getBottomHeight() {
		return rightDisplayHeight;
	}

	void createUI() {
		// Create the left and right (bottom) UIs
		lv_disp_set_default(leftDisp);
		leftScr = lv_obj_create(NULL, NULL);
		lv_disp_set_default(rightDisp);
		rightScr = lv_obj_create(NULL, NULL);
		// Add UIs
		// Left UI selected
		lv_scr_load(leftScr);
		// Pause, Play and Frame advance 1 buttons

		// Right UI selected
		lv_scr_load(rightScr);
		// ... code ...
	}

	void enableDrawing() {
		shouldWrite = true;
	}

	void disableDrawing() {
		shouldWrite = false;
	}

	~AppUI() {
		delete [] leftBuf;
		leftBuf = NULL; 
		delete [] rightBuf;
		rightBuf = NULL; 
		shouldUpdate = false;
		// Wait for the thread to end, shouldn't take long
		pthread_join(updateLvgl, NULL);
	}
}
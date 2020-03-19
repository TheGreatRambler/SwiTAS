The format of a line of a script is as follows:

[frame number] [list of keys to press, separated by semicolons] [x and y coordinates of the left joystick, sparated by a semicolon] [x and y coordinates of the right joystick, separated by a semicolon]

Note: The x and y coordinates should range from the value -30000 to 30000.

The list of keys is as follows:
KEY_A, KEY_B, KEY_X, KEY_Y, KEY_LSTICK, KEY_RSTICK, KEY_L, KEY_R, KEY_ZL, KEY_ZR, KEY_PLUS, KEY_MINUS, KEY_DLEFT, KEY_DUP, KEY_DRIGHT, KEY_DDOWN

The keys must be spelled EXACTLY like this or else it will not register.
To press no keys on a frame, simply put NONE.

You only need to specify frames where there are inputs as the program will default to nothing pressed on a frame. However, if you specify one part, you must specify all of them per frame (ex. if you only want to press the A button, you still need to tell the program that the analog sticks are to be at pos 0;0 and 0;0).

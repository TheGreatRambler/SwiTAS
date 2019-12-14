# Plans for this program
## Setting inputs
1. Set inputs via a large TreeView backed by a ListView, like this:
![ListView example](inputSelection.png)
2. Inputs, when clicked inside of the listview, will toggle
3. A dashboard on the bottom of the program will contain inputs (arranged similarly to how they are arranged on a Pro Controller), and pressing these will also toggle the *current* inputs
4. All operations are done by editing the current frame, which can be edited if you select the frame in the ListView or you load a savestate at that frame or you frameadvance to that frame
## Savestates
1. Soooooo... these are very limited savestates, let's just say that
2. Two different methods exist for savestates, the first is not game specific but is harder to set up and generally harder to use, the second is game specific but it maynot work with the game you are trying to TAS
### Generic savestate method
1. This method only saves three things within the savestate, the frame of the savestate, the *hash* of the screenshot of that frame and some other miscellaneous data such as the current time
2. There is a mode called the *savestate searching* mode. When this is activated, each frame is checked to see if the *hash* of its screenshot matches that of the savestate
3. Once it is found, the user is asked if the frame is correct and they would like to load this frame
4. If no is selected, nothing happens. If yes is selected, nx-TAS jumps to the frame of the savestate in the frame history, essentially rewinding your input history
5. The frames that are edited then overwrite the other frames, normal stuff
6. If you wish to have a savestate somewhere else, a more interesting setup is used
7. Proceed to obtain the savestate as usual, but then set the intended goal of the savestate by choosing a frame after the savestate
8. When this savestate is loaded, the frames after the savestate and before the goal are ran in order to get the frame to the one chosen by the savestate
9. Because of the way this method works, it essentially requires having the savestate be at a location that can be returned to very easily, as in, a level start screen (as, getting to this frame is the user's responsibility)
10. Because this is somewhat limiting in where the savestate can be, that is why an intended target frame can be different than the actual savestate frame
### Game specific savestate method
1. This method allows you to use the native save-storing method of the chosen game
2. This allows you to choose a savestate at essentially any location where the game allows you to save
3. This method simply abstracts away the generic savestate method but builds in a macro to save and load the savefile
4. It is important to note that both require getting the frame of the game in a perfect, repeatable way, which, as of now, is not possible
## Communication with switch (or other targets)
1. The communication of this program to the target will be a simple WebSocket protocol transfering things such as the frame changing, inputs changing, obtaining the savestate, etc...
2. This is intentionally generic enough to allow usage with a Switch but also with an emulator, another TAS system...
3. The goal is for this program to possibly become a frontend for all different types of TAS creators, but this is not the current goal

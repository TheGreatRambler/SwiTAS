# SwiTAS
A TAS toolkit for the Nintendo Switch. Usuable via hardware, homebrew and emulator. Join [the Discord](https://discord.gg/3zDmqpQ)!

# [RELEASES HERE](https://github.com/TheGreatRambler/SwiTAS/releases)
# [BUG REPORTS HERE](https://github.com/TheGreatRambler/SwiTAS/issues)

## Why this?
This project intends to give a easy to use yet powerful frontend to create Switch TASes. While projects exist that play back inputs on a console, usually through the usage of an arduino, these programs lack the ability to frame advance or pause the game consistently.

In the spirit of TASBot, it is generally considered more prestigious to display the final product via unmodified console hardware. This is the approach taken by all of the above products. Dealing with unmodified hardware, however, imposes many limitations during TAS creation. Introducing... this project!

Having access to the entire system via CFW introduces a lot of freedoms, so this project intends to place this functionality into a standard PC application, much like Bizhawk or other TAS creators. This power allows you to tas with the freedom of an emulator through homebrew.

Yuzu support through a simple plugin system is planned, check up on our [Discord](https://discord.gg/3zDmqpQ) to hear about it first.

This editor is intended to most closely resemble the [FCEUX TAS Editor](http://www.fceux.com/web/help/taseditor/). We welcome bug reports on this Github page and invite contributions.

# UI Elements

The main window:

![](https://i.imgur.com/01Z1oYY.png)

The load savestate dialog:

![](https://i.imgur.com/OoMkHXy.png)

The create savestate dialog, which highly resembles the load savestate dialog:

![](https://i.imgur.com/2W1N4q0.png)

## Handling Projects

To start, open the program by navigating to the `release/bin/switas.exe` on Windows, double clicking the app bundle on Macos, or by opening Switas on Debian and Ubuntu systems.

![](https://i.imgur.com/JyCttlK.gif)

Choose an empty folder to create a new project, load from the recent list, or navigate to a project folder on your system.

![](https://i.imgur.com/aLQDLcD.gif)
![](https://i.imgur.com/I2tzGEX.gif)

The project folder can be freely transported on your hard drive and to other PCs and is designed to be friendly to version systems such as GIT. Exporting to the nx-TAS test format can be done with `File/Export To Text Format`. This same text format can also be imported as the current branch using `File/Import From Text Format`.

![](https://i.imgur.com/jJrvTc4.gif)

In order to save the project, you can simply close the window, SwiTAS saves automatically when it is closed naturally. You may also save explicitly by pressing `File/Save Project` or `Ctrl + S`.

## Connecting to Your Switch

From here, you may choose to hook up your PC to your Switch to have the full experience. You can do this by entering the IP address of your system, making sure the sysmodule *SwiTAS* is enabled. You can do this by opening Kosmos/Hekate Toolbox and pressing on *SwiTAS* until it says *on*.  Get your Switch IP address in the *Network Settings* and enter it into the popup, either on startup or whenever you wish by pressing `File/Set Switch IP`.

You may also opt to edit your TAS without having to connect to your Switch. This can be done by simply pressing *cancel* when SwiTAS asks for your IP. You will not be able to frame advance or create savestates, however.

## Syncing With the Switch

In order to take advantage of what SwiTAS has to offer, it is neccessary that you put SwiTAS into a tethered state. This can be done by Creating, Loading or Modifying a savestate.

Whenever you change the number of players in a TAS (or connect to the Switch for the first time), you will be required to disconnect all of your controllers. If you have joycons connected physically to the Switch, remove them. Enter the change grip screen in order to fully disconnect all controllers. Once you do so, a number of controllers corresponding to the number of players will automatically connect themselves. Once this is done, you may reconnect your controllers (it is recommended to reconnect just one for input capturing, another feature) and press *ok* on the dialog. You should not reenter the change grip screen unless you are prompted to remove all controllers again. If you did not see controllers connect themselves in the change grip order screen when you disconnected your controllers, there was an error and you should report the issue.

![](https://i.imgur.com/wqfU2b5.gif)

## Savestate Creation/Loading

The savestates in use in this application are designed to overcome the technical limitations of a lack of true savestates, where the state of the game is saved to a file to be reloaded. Instead, the system depends on detecting similarity between the framebuffers, or screenshot of the game.

Every chunk of inputs you edit is linked to a savestate in order to keep the TASing deterministic and as such you are required to create at least one savestate in order to begin TASing.

In order to choose a good frame for this type of savestate, it is neccessary to choose a frame that:
* Can be returned to easily
* Is visually unique

While each game will have different recommendations, it is usually good to search for a place following the loading of an in game save (the end of a kingdom cutscene in SMO) or the fade in of a level (a frame inside of the black fade in on a MM2 level). To get recommendations for your specific game, the Switch TAS discord would be glad to help.

![](https://i.imgur.com/2O7hoRO.png)

To aid in choosing the perfect frame, the dialog has a number of options. You can choose to play back the game in real time, increment a frame at a time, or increment frames with a specified delay in between each frame in milliseconds. Once you are satisfied with the frame you have chosen (you cannot rewind so don't worry if you have to loop a couple times in order to not miss your frame), press the checkmark on the right side to select the frame.

![](https://i.imgur.com/Pxit1Wt.gif)

It is important to create savestates often. Many games have variable load times that are not full understood by the community. As such, it is important that you create a savestate after every variable load if you are making a long TAS. This includes a savestate for every kingdom in SMO and a savestate for every level in MM2.

## Frame Editing

Select a frame in the *Piano Roll* to begin editing frames. Add frames with `Ctrl + Plus`, create and remove savestate blocks with the buttons on the top, and create and select players. All the familiar commands are here, right click to see them all. Included are `Paste Place`, which only adds buttons if they are not present, and `Paste Insert`, which inserts the frame below the selected one. The text put onto the Clipboard can be pasted in other programs to share inputs with others. `Undo` and `Redo` support are planned.

![](https://i.imgur.com/3Z8ftDP.gif)

To allow for seamless usage, button inputs support keyboards, gamepads, and selecting the corresponding button in the bottom right as methods to change the inputs of the currently selected frame. The keyboard bindings (which mirror that of [the Yuzu emulator](https://yuzu-emu.org/) with some slight changes) are as follows:

![](https://i.imgur.com/uQF8HJZ.png)

To change various settings of the app, refer to the corresponding entry in `switas_settings.json`. This settings file can be found in the root folder on the Windows and Macos releases and can be found at `/etc/switas/switas_settings.json` on Debian and Ubuntu systems. The application will have to be reloaded for the changes to take effect. 

To change keybinds, each character need to be typed with its lowercase equivalent to register. As of now, gamepad support is restricted to a wired pro controller on Windows 10. No drivers are needed. Select the gamepad in `File/List Joysticks` and you're good to go. Note, joystick inputs are not registered until you press the lock button underneath either the right or left joysticks.

SwiTAS also has a video view, in order to use a video from youtube/twitter/twitch/etc to compare against your TAS. All that is needed is YouTube DL, which can be installed with a simple `sudo -H pip install --upgrade youtube-dl`. To use, simply enter the URL of your video and press enter. A few seconds later, select your format and wait for it to download. Now, when selecting frames in the *Piano Roll*, the viewer will automatically adjust the selected frame of the video. The video is saved to your filesystem so you can easily load the video again later. To use this feature, press `File/List Recent Comparison Videos`.

# Examples Including the Switch Screen

Firstly, start the application, creating a new project.

![](https://i.imgur.com/Kvuj8Um.gif)

Next, input the IP address of the system to connect.

![](https://i.imgur.com/E7fMNZC.gif)

Next, create a savestate in order to tether your TAS.

![](https://i.imgur.com/kdsORwd.gif)

Next, use a feature called `auto-frame`. This feature allows you to use the first controller connected to your system to control the current player. The inputs on your controller will be captured and saved within the TAS. You may uncheck `Include Controller Data` in the bottom left if you have weak internet and you would like to increment faster. The delay between frames can be set in the bottom left.

> NOTE: All the images optained both through auto-frame and normal increment can be removed from the project directory without consequences to save on file space, as these images can quickly build up in a large TAS. They can be found in the `framebuffers` folder.

![](https://i.imgur.com/tE7IOM2.gif)

Finally, in order to load a savestate and trigger a rerecord, a dialog similar to savestate creation will be opened. This dialog allows you to return to the framebuffer, or screenshot, that you created the screenshot on. Simply start over the level or reload a save to return to that state.

As a tip, once you are within a half second of the frame, you can simply automatically increment the frame and the system will infer when it approaches the frame and ask you if you wish to load. To decide for yourself about a frame, the closer the number in the bottom left is to zero, the more similar the two framebuffers are.

![](https://i.imgur.com/cWK8i2v.gif)

## Thanks
- [Hamhub7](https://github.com/hamhub7) & [Soren](https://github.com/sorenbug): The creators of [nx-TAS](https://github.com/hamhub7/nx-TAS)
- [MonsterDruide1](https://github.com/MonsterDruide1): Providing many valuable insights into the behavior of hardware-based TAS setups
- [AtlasNX](https://twitter.com/atlasnx?lang=en): For their help during development
- Members of the wxWidgets forums: For being incredibly useful for every question
- [Shakkar23](https://github.com/shakkar23): For being a chad, obviously

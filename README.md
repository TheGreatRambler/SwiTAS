# NX TAS UI
A PC frontend for the Nintendo Switch homebrew [Nx-TAS](https://github.com/hamhub7/nx-TAS).

## Why this?
You have probably heard of the [TASBOT Nintendo Switch setup](https://www.youtube.com/watch?v=VXRL7dUVT-M), [Switch Fightstick](https://github.com/shinyquagsire23/Switch-Fightstick) or even the macros of [JoyCon Droid](https://play.google.com/store/apps/details?id=com.rdapps.gamepad&hl=en_US). This program was created to address the shortcomings of these other, similar, projects.

In the case of the first two, there is a very important problem; they emulate a Horipad, which does not actually have any analog sticks. As you can imagine, this makes TASing games beyond a 2D platformer close to impossible. In the case of JoyCon Droid, while supporting analog sticks and even motion controls, the nature of bluetooth makes it a poor protocol for TAS.

In the spirit of TASBot, it is generally considered more prestigious to display the final product via unmodified console hardware. This is the approach taken by all of the above products. Dealing with unmodified hardware, however, imposes many limitations during TAS creation. Introducing... this project!

Having access to the entire system via CFW introduces a lot of freedoms, so this project intends to place this functionality into a standard PC application, much like Bizhawk or other TAS creators. This power allows you to tas with the freedom of an emulator even though this is not an emulator. Yuzu can't TAS yet, to any dissenters out there :)

This editor most closely resembles the [FCEUX TAS Editor](http://www.fceux.com/web/help/taseditor/).

## How To Use:
* Create a project, load a project, or load a recent project with the dialog at start.
![Load Project](screenshots/loadproject.png)
* Select the IP address of your switch in order to take advantage of Switch homebrew for TASing. This menu appears at start and in `File/Set Switch IP`. (**THIS FEATURE IS CURRENTLY UNSUPPORTED**)
![Select Switch IP](screenshots/selectip.png)
* Select a frame in the *Piano Roll* to begin editing frames. Add frames with `Ctrl + Plus`, create and remove savestate blocks with the buttons on the top, and create and select players. All the familiar commands are here, right click to see them all. Included are `Paste Place`, which only adds buttons if they are not present, and `Paste Insert`, which inserts the frame below the selected one. `Undo` and `Redo` support are planned.
![Piano Roll](screenshots/pianoroll.png)

## Thanks
- [Hamhub7](https://github.com/hamhub7): The creator of Nx-TAS
- [MonsterDruide1](https://github.com/MonsterDruide1): Providing many valuable insights into the behavior of hardware-based TAS setups
- [AtlasNX](https://twitter.com/atlasnx?lang=en): For their help during development
- 黯然的饭#8969 on discord: Providing the ever-elusive answer to the problem of obtaining the framebuffer
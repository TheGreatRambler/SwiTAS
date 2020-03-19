# nx-TAS
> Rewrite of [TAS-nx](https://github.com/hamhub7/TAS-nx)

nx-TAS is fundamentally different than its predecessor, TAS-nx, in that it uses a virtual controller to send inputs instead of changing the inputs of the currently used controller.

## Features
New features are already planned, such as changing the tye of controller (pro, joycons, sideways joycons, and possibly other controllers), multiple player TAS support, motion controls, some way to combat variable load times and rng (for most games), and color customizaition of the controllers.

## Usage
First of all, you need to have an [Atmosphere-injected Switch](https://switch.homebrew.guide/).

### Installation
1. Acquire `nx-TAS.nsp` either from the latest release, or more preferrably build yourself via the instructions below
2. Create the relevant title directory on your Switch's SD card at `sdmc:/atmosphere/titles/0100000000000cab/`
3. Place the NSP as `sdmc:/atmosphere/titles/0100000000000cab/exefs.nsp`
4. Create an empty file at `sdmc:/atmosphere/titles/0100000000000cab/flags/boot2.flag`
5. Reboot into CFW

### Keybinds
You need to have a USB keyboard attached to your Switch, either via the dock or a USB-A to USB-C adapter in the charging port.

#### Controllers
- `1` = Pro Controller  
- `2` = Pair of Joycons (this adds two controllers; exists for ease-of-use)  
- `3` = Left Joycon  
- `4` = Right Joycon  
- `5` = Famicom Controller  
- `6` = Famicom Controller with mic  
- `7` = NES Controller  
- `8` = Unused SNES-style Controller/Pad
- `-` = Remove the most recently added controller

#### Scripts
- `f1` = `sdmc:/scripts/script1.txt`
- `f2` = `sdmc:/scripts/script2.txt`
- ...
- `f12` = `sdmc:/scripts/script12.txt`

#### Utilities
- `Q` = Press L+R on the most recently added controller
- `W` = Press A on the most recently added controller
- `PAUSE` = Pauses and resumes execution of current script
- `SCROLLLOCK` = Stops execution of current script

## Building from Source
1. [Set up the required libraries](https://switchbrew.org/wiki/Setting_up_Development_Environment)
2. In the newly installed MSYS2, navigate to the folder you'd like to build this project in, either by opening MSYS in that directory or using the `cd` command to navigate
3. `git clone https://github.com/hamhub7/nx-TAS.git`
4. `cd nx-TAS`
5. `make`
6. You should see a file called `nx-TAS.nsp`

The `make` process will fail for one of three reasons:
- You cloned a commit that doesn't compile (unlikely)
- Your folder is named something other than `nx-TAS`; solution is to `ln -s nx-TAS.json config.json`
- Your directory's full path has spaces in it (usually your account name)

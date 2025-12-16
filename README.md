# TRAWindowed - Tomb Raider Windowed
This is a custom d3d9.dll that enables more display modes for the three second era Tomb Raider games. This allows you to play the game in "windowed mode", as well as "borderless-window mode"

## Supported Games
* Tomb Raider: Legend
* Tomb Raider: Anniversary
* Tomb Raider: Underworld

## How to use
### Installation
1. Download the latest dll from the [release page](https://github.com/chreden/trawindowed/releases/)
2. Navigate to your Tomb Raider game directory
3. Place the d3d9.dll in the same folder as the Tomb Raider game binary (exe file)

### Switch display modes
1. Start the game
2. Press F5 to switch between bordered and borderless windowed mode.

### Center the window
1. Start the game
2. Press Shift + F5 to center the window on the monitor the window is currently on.

### Toggle Always on Top
1. Start the game
2. Press F6 to toggle always on top

### Toggle mouse capture
1. Start the game
2. Press F8 to toggle mouse capture.

This can be useful if you want to interact with the TRAE Menu Hook.

### Command Line Arguments
These can all be added via the "Launch Options" in Steam.

- Passing `-borderless` to the game exe will launch the game in borderless window mode (as if you had started the game and pressed the toggle button).
- Passing `-camerafix` to the game exe will fix an issue with the camera (if it gets stuck looking down).
- Passing `-autocenter` to the game exe will start the window in the center of the primary display. Underworld may need to be manually centered.
- Passing `-disableaspectfix` to the game exe will stop the TRU aspect ratio being calculated based on the window size and will use the game setting instead.
- Passing `-ontop` to the game exe will start the window on top.
- Passing `-nomousecapture` will start the game without forcing mouse capture.

## Information for developers
### Build
You need Visual Studio to compile the dll. Just import the ```trashim.sln``` project file. Before you start the build you have to change the active configuration to "Release" and "Win32" as target platform.

After the build is completed the final ```d3d9.dll``` will be located in the ```Release``` folder.
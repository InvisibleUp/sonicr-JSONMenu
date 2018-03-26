JSONMenu
========

JSONMenu is a rewrite of Sonic R's menu system meant to make customization, expandability and testing much, much easier than the vanilla menu.

Features
--------

- Launch Sonic R into a race from the command line
- Define your own styles with your own custom mod (See the SonicRushMenu project for an example)
- Easily add options from your mod to an existing menu
- Faster and quieter

Todo
----

- Move graphics, font, and music routines into a seperate "libcore" sort of library
- Finish Sonic Rush example menu (only title screen is implemented right now)
- Add support for mod-defined racers and tracks (likely will be rolled into that "libcore" thing)
- Add multiplayer support
- Port Options menu
- Add ability to specify screen alignment of a Menu2DElement

Command Line Quick Reference
----------------------------
(Assuming no other theming mods are installed) Call SonicR.exe with the following arguments:

- `qstart=gp` : Start race as Sonic (character 0) in Resort Island in Grand Prix mode
- `qstart=tt` : Start race as Sonic (character 0) in Resort Island in Normal Time Attack mode
- `gp [racer id] [track id]` : Start race as given racer in given track in Grand Prix mode
- `tt [tt mode] [racer id] [track id]` : Start race as given racer in given track in given Time Attack mode

- `[racer id]`: One of
	- sonic
	- tails
	- knux
	- amy
	- robotnik
	- msonic
	- dtails
	- mknux
	- eggrobo
	- ssonic
- `[track id]`: One of
	- island
	- city
	- ruin
	- factory
	- emerald
- `[tt mode]`: One of
	- normal
	- reverse
	- balloon
	- tag4

API Quick Reference
-------------------
Include the `export` directory in a new DLL project. To define a new style, do the following:

1. Define SonicRModInfo and Init to make the mod loader happy
2. Create a new class that extends MenuSelector and defines
	- Constructor: Reads the JSON for that style and populates the menu
	- loadTextures: Loads whatever textures are needed, not including any textures required by option icons.
	- draw: Updates and draws the menu elements
	- hscroll: Horizontal scroll action. Typically just moves the current selection or sub-selection.
	- vscroll: Vertical scroll action. Typically just moves the current selection or sub-selection.
	- querySelection: Gets the currently selected option
	- querySubSelection: For options with sub-options, gets the current suboption. Otherwise returns -1.
	- doAction: If your theme supports custom actions (eg: changing the background, transitions, etc.) put them here
3. Define a constructor function that returns a new instance of your style class as a `void *`
4. In `DllMain` > `DLL_PROCESS_ATTACH`,
	1. Load JSONMenu.dll using LoadLibrary, and relevant functions using GetProcAddress
	2. Check that the result of `MajorVersion` == 1. Alert user and return false otherwise.
	3. To register your style, call `AddMenuClass` with a `std::pair` containing your class name and the constructor function you created earlier.
	4. To add or replace an existing menu, call `AddMenu(dump_json(filepath).GetObject());` where filepath is a JSON file containing a single menu.
	5. To add or replace an existing menu option, call `AddMenuOption(dump_json(filepath).GetObject());` where filepath is a JSON file containing a single option.
	6. Free the JSONMenu.dll library!

If you're stuck, refer to the example project in the SonicRushMenu folder, which defines a custom title screen mimicking the Sonic Rush title screen.

JSON Quick Reference
--------------------
### Important notes
- The screen is 640x480. For widescreen displays, negative values will still be shown.
- Colors are stored in the format "0xAaRrGgBb". Sonic R uses "0xFFE0E0E0" for most graphics for some reason.
- Menus run at 60FPS, or probably whatever your monitor's refresh rate is

### Annotated example
```javascript
{
	"id": "test", // ID that will be used for goto commands and the command line interface
        "layout": { // See below
            "class": "simple_vertical", // One of the defined style classes
            "properties": { // Style-specific properties. See below
                "xoff": 200,
                "yoff": 400,
                "selcolor": "0xFFFFFFFF"
            }
        },
        "timeout": {
		"time": 600, // Frames to show menu for
		"actions": [...] // Actions to be performed when time runs out. After all actions, returns to previous menu
	},
        "music": "mods/MyCoolStyle/music/menu.adx", // Music to play in ADS, OGG, BRTSM, etc.
        "background": {
            "src": "mods/MyCoolStyle/art/bg.raw" // Optional background to display. Can be in PNG or RAW. Must be 640x480.
        },
        "options": [
            {
                "id": "start", // ID used for command line interface
                "text": "Press Start Button", // Text to display on menu
		"icon": { // Icon to display on menu
                    "filepath": "bin/option/smode00.raw", // Path to 256x256 PNG or RAW texture to use
                    "x": 0, // X position of icon in texture
		    "y": 160, // Y position of icon in texture
		    "w": 56, // Width of icon in texture
		    "h": 48, // Height of icon in texture
		    "scale": 2 // Integer scaling factor
                }
                "actions": [ // List of actions to perform. See below.
                    {"goto": "root"}
                ]
            }
        ]
}
```

### Style Classes
- `simple_vertical`: Just a simple vertical menu with no scrolling
	- `xoff`: Position menu starts from the left side of the screen
	- `yoff`: Position menu starts from the top side of the screen
	- `xspacing`: Spacing between icon and text
	- `yspacing`: Spacing between menu options
	- `basecolor`: Color of unselected text
	- `selcolor`: Color of selected text
	- `disabledcolor`: Color of disabled text
	- `brightcolor`: Color used for icons & graphics. (white)
- `scroll_vertical`: Like simple_vertical except it scrolls up and down to accommodate many options
	- `xoff`: Position menu starts from the left side of the screen
	- `yoff`: Position menu starts from the top side of the screen
	- `xspacing`: Spacing between icon and text
	- `yspacing`: Spacing between menu options
	- `basecolor`: Color of unselected text
	- `selcolor`: Color of selected text
	- `disabledcolor`: Color of disabled text
	- `brightcolor`: Color used for icons & graphics. (white)
	- `scrollspeed`: Speed to scroll at, in pixels/frame
- `scroll_horizontal`: Horizontal layout that scrolls
	- `xoff`: Position menu starts from the left side of the screen
	- `yoff`: Position menu starts from the top side of the screen
	- `xspacing`: Spacing between menu options
	- `yspacing`: Spacing between icon and text
	- `basecolor`: Color of unselected text
	- `selcolor`: Color of selected text
	- `disabledcolor`: Color of disabled text
	- `brightcolor`: Color used for icons & graphics. (white)
	- `scrollspeed`: Speed to scroll at, in pixels/frame

### Actions
- `{functcall: [address]}`: Calls the given address, which is assumed to be a function that takes no arguments and returns no arguments.
- `{setInt: [address], value: [integer]}`: Sets the Int32 at `[address]` to `[integer]`.
- `{goto: [menuID]}`: Suspends the current menu as-is and starts execution of the menu given by `[menuID]`.
- `{stopmusic: true}`: Stops any currently playing music.
- `{startgame: true}`: Starts the game with the current settings. Be careful; without proper setup this WILL crash the game!
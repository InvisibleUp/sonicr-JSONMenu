// JSONMenu.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "MenuClasses\simplevertical.h"
#include "MenuClasses\scrollvertical.h"
#include "MenuClasses\scrollhorizontal.h"

rapidjson::Document menus;

int Menu_Action(rapidjson::Value &x, MenuSelector *menu);
int Menu_Loop(int menuNo);
void Menu_StartRoot(void);
void Menu_StartIntro(void);
void Menu_Init(void);

int Menu_Action(rapidjson::Value &action, MenuSelector *menu) {
	if (menu->doAction(action)) { return 0; }

	if (action.HasMember("functcall")) {

		int offset = strtol(action["functcall"].GetString(), NULL, 0);
		//PrintDebug("Calling function at %X", offset);
		FunctionPointer(void, tmpfunct, (void), (offset));

		tmpfunct();

	} else if (action.HasMember("setInt") && action.HasMember("value")) {

		int offset = strtol(action["setInt"].GetString(), NULL, 0);
		int value = strtol(action["value"].GetString(), NULL, 0);
		PrintDebug("Setting %X to %d\n", offset, value);

		*(int *)(offset) = value;

	} else if (action.HasMember("startgame")) {

		*(int *)0x7356EC = 1; // Set this to 1, or else game will crash
		if (Track_ID < 1) { Track_ID = 1; }
		RandomizeGPPlayers();

		// Return from menu with a 1, and try to just force self through existing system
		return 1;

	} else if (action.HasMember("goto")) {

		int newIdx = Menu_GetIndexFromID(action["goto"].GetString(), menus);
		if (newIdx != -1) {
			int retval = Menu_Loop(newIdx);

			// If trying to start game, do that instead
			if (retval == 1) {
				return 1;
			}
		}
	}

	return 0;
}

// Main loop
int Menu_Loop(int menuNo) {
	// Get variables from JSON
	if (!menus.IsArray() || !menus[menuNo].IsObject()) {
		PrintDebug("Malformed menu JSON!");
		return -1;
	}
	PrintDebug("Starting menu system!\n");
	int Menu_TimeLeft = menus[menuNo]["timeout"].GetInt();
	bool Menu_MovementLatched = true;

	std::vector<const char *> GFXList = Menu_LoadGFX(menus[menuNo]);

	// Layout
	MenuSelector *root;
	const char *menuClass = menus[menuNo]["layout"]["class"].GetString();
	if (strcmp(menuClass, "simple_vertical") == 0) {
		root = new MenuSelectorSimpleVertical(menus[menuNo], GFXList);
	} else if (strcmp(menuClass, "scroll_vertical") == 0) {
		root = new MenuSelectorScrollVertical(menus[menuNo], GFXList);
	} else if (strcmp(menuClass, "scroll_horizontal") == 0) {
		root = new MenuSelectorScrollHorizontal(menus[menuNo]);
	} else {
		PrintDebug("Invalid menu type! Defaulting to simple_vertical");
		root = new MenuSelectorSimpleVertical(menus[menuNo], GFXList);
	}

	// Play music if applicable
	/*if (
		menus[menuNo].HasMember("music") && 
		CurrentMusicTrack != menus[menuNo]["music"].GetInt()
	) {
			Music_Play(menus[menuNo]["music"].GetInt());
	}*/

	// Main loop
	while (Menu_TimeLeft > 0) {
		PrintDebug("%d frames remaining\r", Menu_TimeLeft);

		// Handle Win32 stuff
		User32_PeekMessageA();

		// Handle inputs
		{
			struct Inputs inputs = Pad_GetInputsUsable(0);

			// Prevent movement if key is held
			if (
				Menu_MovementLatched &&
				!inputs.Up && !inputs.Down &&
				!inputs.Left && !inputs.Right &&
				!inputs.A && !inputs.B && !inputs.Start
			) {
				Menu_MovementLatched = false;
			}

			// Handle up/down movement
			if (
				!Menu_MovementLatched &&
				(inputs.Up == true || inputs.Down == true)
			) {
				Menu_MovementLatched = true;
				if (inputs.Up) {
					root->vscroll(-1);
				} else if (inputs.Down) {
					root->vscroll(1);
				}
			}

			// Handle left/right movement
			if (
				!Menu_MovementLatched &&
				(inputs.Left == true || inputs.Right == true)
			) {
				Menu_MovementLatched = true;
				if (inputs.Left) {
					root->hscroll(-1);
				}
				else if (inputs.Right) {
					root->hscroll(1);
				}
			}

			// Handle A/Start press
			if (!Menu_MovementLatched && (inputs.A || inputs.Start)) {
				Menu_MovementLatched = true;

				const rapidjson::Value& a = menus[menuNo]["options"][root->selection];

				// Cycle through actions
				if (a.HasMember("actions") && a["actions"].IsArray()) {
					for (rapidjson::SizeType i = 0; i < a["actions"].Size(); i++) {
						rapidjson::Value& x = menus[menuNo]["options"][root->selection]["actions"][i];
						int retval = Menu_Action(x, root);
						if (retval == 1) { return 1; }
					}
					// Reload GFX & timeout after finishing actions
					// (loading the GFX is a bit of a waste, but meh, it takes milliseconds)
					Menu_LoadGFX(menus[menuNo]);
					Menu_TimeLeft = menus[menuNo]["timeout"].GetInt();
				}
			}

			// Handle B press
			if (!Menu_MovementLatched && inputs.B) {
				// Go to previous menu
				return -1;
			}
		}

		// Begin rendering
		D3D_BeginScene();
		{
			root->draw(-1);

			// Render BG
			D3D_RenderBG(0);
			D3D_Flush();
		}
		D3D_EndScene();
		D3D_ScreenSwap();

		// Wait until next frame
		Menu_TimeLeft -= 1;
		FrameDelay(1); // Basically a vblank no matter what this is set to
	}
	return -1;
}

// Start "intro" menu, or root if title does not exist
void Menu_StartIntro(void) {
	int intro = Menu_GetIndexFromID("intro", menus);
	if (intro == -1) { Menu_StartRoot(); }
	Menu_Loop(intro);
}

// Start root menu
void Menu_StartRoot(void) {
	int root = Menu_GetIndexFromID("root", menus);
	if (root == -1) { 
		PrintDebug("'root' menu not found! Cannot continue!");
		QuitSonicR(); 
	}
	Menu_Loop(root);

	// If we back out from root after Retire, go back to intro
	int intro = Menu_GetIndexFromID("intro", menus);
	if (intro != -1) { Menu_Loop(intro); }
}

// Load default menus
void Menu_Init(void) {
	// Load default menus
	menus = dump_json("mods/jsonmenu/menus/default.json");
}

extern "C" {
	__declspec(dllexport) ModInfo SonicRModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init() {

		// Set the intial state of the wobbly screen transition oval
		Oval_Speed = 16;
		Oval_Size = 0xFFFFFF00;

		// Replace SEGA Logo with our own loop.
		WriteJump((void *)0x444C90, &Menu_StartIntro);

		// Replace menus with NOP
		unsigned char nop[64];
		std::fill_n(nop, 64, 0x90);

		WriteData((void *)0x43AB9B, &nop, 5); // TTales
		WriteData((void *)0x43AC18, &nop, 5); // Title
		WriteData((void *)0x43AC80, &nop, 5); // Mode
		WriteData((void *)0x43ACD0, &nop, 5); // MP
		WriteData((void *)0x43AD90, &nop, 5); // CharSelect
		WriteData((void *)0x43ADDB, &nop, 5); // TAMode
		WriteData((void *)0x43AE6D, &nop, 5); // Course Select
		WriteData((void *)0x43BFE4, &nop, 5); // MP Mode

		WriteData((void *)0x43ABB9, &nop, 5); // GFX_Title
		//WriteData((void *)0x43BFDF, &nop, 5); // GFX_Options (1)
		WriteData((void *)0x43BEDB, &nop, 5); // GFX_Options (2)
		WriteData((void *)0x43AC4E, &nop, 5); // GFX_Options (3)
		WriteData((void *)0x43AC58, &nop, 6); // Mode screen init

		WriteData((void *)0x43BFC1, &nop, 2); // Force "retire" to take MP path
		WriteCall((void *)0x43BFDF, &Menu_StartRoot); // Redirect now-useless MP menu to our menu
		WriteData((void *)0x43C087, &nop, 4); // Don't reset player count to 1
		WriteData((void *)0x43C066, &nop, 10); // Don't clear TT mode
		WriteData((void *)0x43AD84, &nop, 12); // Don't clear game/TT mode when going through GP path
		WriteData((void *)0x43AC76, &nop, 10); // Don't reset racer count to 5 on GP path
	}
}
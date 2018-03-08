// JSONMenu.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "MenuClasses\simplevertical.h"
#include "MenuClasses\scrollvertical.h"
#include "MenuClasses\scrollhorizontal.h"

rapidjson::Document menus;

int Menu_GetIndexFromID(const char *id) {
	for (rapidjson::SizeType i = 0; i < menus.Size(); i++) {
		if (strcmp(menus[i]["name"].GetString(), id) == 0) {
			return i;
		}
	}

	return -1;
}

void Menu_LoadGFX(int menuNo) {
	// Assume D3D, as that's the only thing supported
	D3D_ClearTextures();

	// Required for wobbly oval of doom
	D3D_LoadTexture(0, "general\\player00.raw");

	// Load BG (1-6)
	D3D_LoadWallpaper(menus[menuNo]["background"]["src"].GetString());

	// Required for text
D3D_LoadTexture(7, "mods\\jsonmenu\\font.raw");

// Load textures required for preview and icons

// Finalize
D3D_TexturesToGPU();
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

	// Layout
	MenuSelector *root;
	const char *menuClass = menus[menuNo]["layout"]["class"].GetString();
	if (strcmp(menuClass, "simple_vertical") == 0) {
		root = new MenuSelectorSimpleVertical(menus[menuNo]);
	} else if (strcmp(menuClass, "scroll_vertical") == 0) {
		root = new MenuSelectorScrollVertical(menus[menuNo]);
	} else if (strcmp(menuClass, "scroll_horizontal") == 0) {
		root = new MenuSelectorScrollHorizontal(menus[menuNo]);
	} else {
		PrintDebug("Invalid menu type! Defaulting to simple_vertical");
		root = new MenuSelectorSimpleVertical(menus[menuNo]);
	}

	Menu_LoadGFX(menuNo);

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
					root->scroll(-1);
				} else if (inputs.Down) {
					root->scroll(1);
				}
			}

			// Handle A/Start press
			if (!Menu_MovementLatched && (inputs.A || inputs.Start)) {
				Menu_MovementLatched = true;

				const rapidjson::Value& a = menus[menuNo]["options"][root->selection];

				// Cycle through actions
				if (a.HasMember("actions") && a["actions"].IsArray()) {
					for (rapidjson::SizeType i = 0; i < a["actions"].Size(); i++) {
						const rapidjson::Value& x = a["actions"][i];

						if (x.HasMember("functcall")) {
							int offset = strtol(x["functcall"].GetString(), NULL, 0);
							PrintDebug("Calling function at %X", offset);
							FunctionPointer(void, tmpfunct, (void), (offset));

							tmpfunct();
						}

						if (x.HasMember("functjmp")) {
							int offset = strtol(x["functjmp"].GetString(), NULL, 0);
							PrintDebug("Jumping to code at %X", offset);
							FunctionPointer(void, tmpfunct, (void), (offset));
							
							__asm { // What's a stack?
								jmp tmpfunct;
							}
						}

						if (x.HasMember("setvar") && x.HasMember("value")) {
							int offset = strtol(x["setvar"].GetString(), NULL, 0);
							int value = strtol(x["value"].GetString(), NULL, 0);
							//DataPointer(int, tmpvar, (offset));
							PrintDebug("Setting %X to %d\n", offset, value);

							*(int *)(offset) = value;
						}

						if (x.HasMember("startgame")) {
							// Set this to 1, or else game will crash
							DataPointer(int, unk, 0x7356ec);
							unk = 1;
							// If course < 1, snap it to 1
							DataPointer(int, course, 0x7af13c);
							if (course < 1) { course = 1; }
							// Return from menu with a 1, and try to just force self through existing system
							return 1;
						}
					}
				}

				// Go to new menu
				if (a.HasMember("target")) {

					int newIdx = Menu_GetIndexFromID(a["target"].GetString());
					if (newIdx != -1) {
						int retval = Menu_Loop(newIdx);

						// If trying to start game, do that instead
						if (retval == 1) {
							return 1;
						}

						// Reload GFX & timeout after finishing new menu
						Menu_LoadGFX(menuNo);
						Menu_TimeLeft = menus[menuNo]["timeout"].GetInt();
					}
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
			root->draw(0);

			// Render BG
			D3D_RenderBG(0);
			D3D_Flush();
		}
		D3D_EndScene();
		D3D_ScreenSwap();

		// Wait until next frame
		Menu_TimeLeft -= 1;
		FrameDelay(30); // in ms (TODO: Replace with VBlank?)
	}
	return -1;
}

// Start first menu
void Menu_Start(void) {
	Menu_Loop(0);
}

// Load default menus
void Menu_Init(void) {
	// Load default menus
	menus = dump_json("mods/jsonmenu/menus/default.json");
}

extern "C" {
	__declspec(dllexport) ModInfo SonicRModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init() {

		// Replace SEGA Logo with our own loop.
		WriteJump((void *)0x444C90, &Menu_Start);

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
		WriteCall((void *)0x43BFDF, &Menu_Start); // Redirect now-useless function call to menu
		WriteData((void *)0x43C087, &nop, 4); // Don't reset player count to 1
	}
}
// JSONMenu.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "MenuClasses\simplevertical.h"
#include "MenuClasses\scrollvertical.h"
#include "MenuClasses\scrollhorizontal.h"

rapidjson::Document menus;
std::queue<const char *> optionQueue;
std::map<
	std::string,
	void * (*) (rapidjson::Value &a, std::vector<const char *> GFXList)
> menuClassList;

int Menu_Action(rapidjson::Value &x, MenuSelector *menu);
int Menu_Loop(int menuNo);
void Menu_StartRoot(void);
void Menu_StartIntro(void);
void Menu_Init(void);
bool Menu_FromCmd(std::queue<const char *> &args, int menuID);
void Init_ReadCmdlineOptions(char **argv, int argc);

// MenuSelector can be NULL!
int Menu_Action(rapidjson::Value &action, MenuSelector *menu) {
	if (menu != NULL && menu->doAction(action)) { return 0; }

	if (action.HasMember("functcall")) {
		int offset = strtol(action["functcall"].GetString(), NULL, 0);
		FunctionPointer(void, tmpfunct, (void), (offset));

		tmpfunct();

	} else if (action.HasMember("setInt") && action.HasMember("value")) {
		int offset = strtol(action["setInt"].GetString(), NULL, 0);
		int value = strtol(action["value"].GetString(), NULL, 0);
		PrintDebug("Setting %X to %d\n", offset, value);

		*(int *)(offset) = value;

	} else if (action.HasMember("startgame")) {
		StopMusic();
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
	} else if (action.HasMember("stopmusic")) {
		StopMusic();
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
	bool Menu_MovementLatched = true;
	int Menu_TimeLeft = std::numeric_limits<int>::max();

	if (menus[menuNo].HasMember("timeout") && menus[menuNo]["timeout"].IsObject()) {
		Menu_TimeLeft = JSON_GetNum(menus[menuNo]["timeout"], "time", std::numeric_limits<int>::max());
	}

	// Layout
	MenuSelector *root;
	const char *menuClass = menus[menuNo]["layout"]["class"].GetString();
	if (menuClassList.find(menuClass) == menuClassList.end()) {
		PrintDebug("Unknown type %s! Defaulting to simple_vertical", menuClass);
		menuClass = "simple_vertical";
	}
	
	// Textures & other variables
	std::vector<const char *> GFXList = Menu_LoadGFX(menus[menuNo]);
	root = (MenuSelector *)menuClassList[menuClass](menus[menuNo], GFXList);
	root->loadTextures(GFXList);

	// Play music if applicable
	if (
		menus[menuNo].HasMember("music") &&
		menus[menuNo]["music"].IsString()
	) {
			PlayMusicFromFile(menus[menuNo]["music"].GetString());
	}

	// Main loop
	while (Menu_TimeLeft > 0) {
		//PrintDebug("%d frames remaining\r", Menu_TimeLeft);

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

				int selection = root->querySelection();
				int subSelection = root->querySubSelection();

				if (selection >= 0) {
					const rapidjson::Value& a = menus[menuNo]["options"][selection];

					// Cycle through choice actions
					if (
						subSelection >= 0 &&
						a.HasMember("choices") &&
						a["choices"].IsArray() &&
						subSelection < a["choices"].Size() &&
						a["choices"][subSelection].HasMember("actions") &&
						a["choices"][subSelection]["actions"].IsArray()
						) {
						for (rapidjson::SizeType i = 0; i < a["choices"][subSelection]["actions"].Size(); i++) {
							rapidjson::Value& x = menus[menuNo]["options"][selection]["choices"][subSelection]["actions"][i];
							int retval = Menu_Action(x, root);
							if (retval == 1) { return 1; }
						}
					}

					// Cycle through primary actions
					if (a.HasMember("actions") && a["actions"].IsArray()) {
						for (rapidjson::SizeType i = 0; i < a["actions"].Size(); i++) {
							rapidjson::Value& x = menus[menuNo]["options"][root->selection]["actions"][i];
							int retval = Menu_Action(x, root);
							if (retval == 1) { return 1; }
						}
					}

					// Reload GFX & msuic & timeout after finishing actions
					// (loading the GFX is a bit of a waste, but meh, it takes milliseconds)
					GFXList = Menu_LoadGFX(menus[menuNo]);
					root->loadTextures(GFXList);
					if (
						menus[menuNo].HasMember("music") &&
						menus[menuNo]["music"].IsString()
						) {
						PlayMusicFromFile(menus[menuNo]["music"].GetString());
					}
					if (menus[menuNo].HasMember("timeout") && menus[menuNo]["timeout"].IsObject()) {
						Menu_TimeLeft = JSON_GetNum(menus[menuNo]["timeout"], "time", std::numeric_limits<int>::max());
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
			root->draw(-1);

			// Render BG
			D3D_RenderBG(0);
			D3D_Flush();
		}
		D3D_EndScene();
		D3D_ScreenSwap(); // Wait for vblank
		Menu_TimeLeft -= 1;
	}

	// Timeout actions
	if (
		Menu_TimeLeft == 0 &&
		menus[menuNo].HasMember("timeout") &&
		menus[menuNo]["timeout"].HasMember("actions")
	) {
		for (rapidjson::SizeType i = 0; i < menus[menuNo]["timeout"]["actions"].Size(); i++) {
			rapidjson::Value& x = menus[menuNo]["timeout"]["actions"][i];
			int retval = Menu_Action(x, root);
			if (retval == 1) { return 1; }
		}
	}

	return -1;
}

// Start "intro" menu, or root if title does not exist
void Menu_StartIntro(void) {
	// Handle automenu
	int root = Menu_GetIndexFromID("root", menus);
	bool skipIntro = false;
	if (root != -1 && optionQueue.size() != 0) {
		Menu_FromCmd(optionQueue, root);
		skipIntro = true;
	}

	if (!skipIntro) {
		int intro = Menu_GetIndexFromID("intro", menus);
		if (intro == -1) { Menu_StartRoot(); }
		else { Menu_Loop(intro); }
	}
}

// Start root menu
void Menu_StartRoot(void) {
	int root = Menu_GetIndexFromID("root", menus);
	if (root == -1) { 
		PrintDebug("'root' menu not found! Cannot continue!");
		QuitSonicR(); 
	}
	int retval = Menu_Loop(root);

	// If we back out from root after Retire, go back to intro
	if (retval == -1) {
		int intro = Menu_GetIndexFromID("intro", menus);
		if (intro != -1) { Menu_Loop(intro); }
	}
}

// Load default menus
void Menu_Init(void) {
	// Load default menus
	menus = dump_json("mods/jsonmenu/menus/default.json");

	// Load default classes
	menuClassList.insert(
		std::pair<
			std::string,
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
		>("simple_vertical", &MenuSelectorSimpleVertical_Constructor)
	);
	menuClassList.insert(
		std::pair<
			std::string,
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
		>("scroll_vertical", &MenuSelectorScrollVertical_Constructor)
	);
	menuClassList.insert(
		std::pair<
			std::string,
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
		>("scroll_horizontal", &MenuSelectorScrollHorizontal_Constructor)
	);
}

// returns true when game is started, false if an error occurs
bool Menu_FromCmd(std::queue<const char*> &args, int menuID) {
	if (args.size() == 0) { return false; }
	const char *nextOptionID = args.front();
	args.pop();
	PrintDebug(">%s\n", nextOptionID);
	std::vector<std::string> choiceIDs = SplitString(nextOptionID, '=');

	if (!menus[menuID].HasMember("options")) { return false; }
	rapidjson::Value &options = menus[menuID]["options"];

	for (unsigned int i = 0; i < options.Size(); i++) {
		// Check if option we're looking at is the right one
		if (!options[i].HasMember("id")) { continue; }
		const char *thisID = options[i]["id"].GetString();
		if (choiceIDs[0] != thisID) { continue; }

		// Iterate through subselections if applicable
		if (options[i].HasMember("choices") && choiceIDs.size() > 1) {
			rapidjson::Value &choices = options[i]["choices"];
			for (unsigned int j = 0; j < choices.Size(); j++) {
				if (!choices[j].HasMember("id")) { continue; }
				const char *choiceID = choices[j]["id"].GetString();
				if (choiceIDs[1] != choiceID) { continue; }

				if (choices[j].HasMember("actions")) {
					for (unsigned int k = 0; k < choices[j]["actions"].Size(); k++) {
						if (choices[j]["actions"][k].HasMember("goto")) {
							int newID = Menu_GetIndexFromID(choices[j]["actions"][k]["goto"].GetString(), menus);
							return Menu_FromCmd(args, newID);
						}
						else if (choices[j]["actions"][k].HasMember("startgame")) {
							RandomizeGPPlayers();
							return true;
						}
						else {
							Menu_Action(choices[j]["actions"][k], NULL);
						}
					}
				}
			}
		}

		// Iterate through primary actions if applicable
		if (options[i].HasMember("actions")) {
			rapidjson::Value &actions = options[i]["actions"];

			for (unsigned int j = 0; j < actions.Size(); j++) {
				if (actions[j].HasMember("goto")) {
					int newID = Menu_GetIndexFromID(actions[j]["goto"].GetString(), menus);
					return Menu_FromCmd(args, newID);
				}
				else if (actions[j].HasMember("startgame")) {
					RandomizeGPPlayers();
					return true;
				}
				else {
					Menu_Action(actions[j], NULL);
				}
			}
		}
	}

	// Selected item didn't lead to a startgame action
	PrintDebug("Option ID %s not found in this menu.\n", nextOptionID);
	return false;
}

void Init_ReadCmdlineOptions(char ** argv, int argc) {
	for (int i = 1; i < argc; i++) {
		optionQueue.push(_strdup(argv[i]));
	}
}

extern "C" {
	__declspec(dllexport) int __cdecl MajorVersion() { return 1; }
	__declspec(dllexport) int __cdecl MinorVersion() { return 0; }

	__declspec(dllexport) void __cdecl AddMenuClass(
		std::pair<
			std::string,
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
		> value
	) {
		menuClassList.insert(value);
	}

	__declspec(dllexport) bool __cdecl RemoveMenuByID(const char * menuID) {
		for (rapidjson::Value::ConstValueIterator itr = menus.Begin(); itr != menus.End(); itr++) {
			if (!itr[0].HasMember("id")) { continue; }
			if (strcmp(itr[0]["id"].GetString(), menuID) == 0) {
				menus.Erase(itr);
				return true;
			}
		}
		return false;
	}

	__declspec(dllexport) bool __cdecl AddMenu(rapidjson::Value menu) {
		// Reject if no ID
		if (!menu.IsObject() || !menu.HasMember("id")) { return false; }

		// Duplicate object (or else it'll dealloc and program will crash)
		rapidjson::Value menu2;
		menu2.CopyFrom(menu, menus.GetAllocator());

		RemoveMenuByID(menu2["id"].GetString());
		menus.PushBack(menu2, menus.GetAllocator());

		return true;
	}

	__declspec(dllexport) bool __cdecl RemoveMenuOption(const char *menuID, const char *optionID) {
		// Search for menu number
		int menuNo = Menu_GetIndexFromID(menuID, menus);
		if (menuNo == -1) { return false; } // Menu not found
		if (!menus[menuNo].HasMember("options")) { return false; } // Options not found
		
		for (
			rapidjson::Value::ConstValueIterator itr = menus[menuNo]["options"].Begin();
			itr != menus[menuNo]["options"].End(); itr++
		) {
			if (!itr[0].HasMember("id")) { continue; }
			if (strcmp(itr[0]["id"].GetString(), optionID) == 0) {
				menus[menuNo]["options"].Erase(itr);
				return true;
			}
		}

		return false;
	}

	__declspec(dllexport) bool __cdecl AddMenuOption(const char * menuID, rapidjson::Value option) {

		// Reject if invalid
		if (!option.IsObject() || !option.HasMember("id")) {
			return false;
		}

		// Duplicate object (or else it'll dealloc and program will crash)
		rapidjson::Value option2;
		option2.CopyFrom(option, menus.GetAllocator());

		// Search for menu number
		int menuNo = Menu_GetIndexFromID(menuID, menus);
		if (menuNo == -1) { return false; } // Menu not found

		// Add options array if none exist
		if (!menus[menuNo].HasMember("options")) {
			rapidjson::Value optionsArray;
			optionsArray.SetArray();
			menus[menuNo].AddMember("options", optionsArray, menus.GetAllocator());
		}

		// Add option to options
		RemoveMenuOption(menuID, option2["id"].GetString());
		menus[menuNo]["options"].PushBack(option2, menus.GetAllocator());

		return true;
	}

	__declspec(dllexport) ModInfo SonicRModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init() {

		// Set the intial state of the wobbly screen transition oval
		Oval_Speed = 16;
		Oval_Size = 0xFFFFFF00;

		// Replace SEGA Logo with our own loop.
		WriteJump((void *)0x444C90, &Menu_StartIntro);

		// Replace command line option parser with our own
		WriteCall((void *)0x433486, &Init_ReadCmdlineOptions);

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

		WriteData((void *)0x43a6bd, &nop, 40); // Don't init certain RAM values on startup
	}
}
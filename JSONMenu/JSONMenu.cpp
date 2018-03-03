// JSONMenu.cpp : Defines the exported functions for the DLL application.

#include <cstdio>
#include <cstring>
#include <cstdbool>
#include <algorithm>
#include "stdafx.h"
#include "SonicRModLoader.h"
#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"

// https://stackoverflow.com/a/4609795
template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

// General functions
FunctionPointer(
	void, D3D_LoadTexture, 
	(int index, const char *filepath), 0x407A00
);
FunctionPointer(
	void, D3D_LoadWallpaper,
	(const char *filepath), 0x408500
);
FunctionPointer(void, D3D_TexturesToGPU, (void), 0x442850);
FunctionPointer(int, User32_PeekMessageA, (void), 0x404BF0);
FunctionPointer(void, Menu_SetOvalSize, (void), 0x409975);
FunctionPointer(void, D3D_BeginScene, (void), 0x4424D0);
FunctionPointer(void, D3D_SetViewport, (int offset), 0x439D80);
FunctionPointer(void, D3D_RenderBG, (int unk), 0x442650);
FunctionPointer(void, D3D_EndScene, (void), 0x4425F0);
FunctionPointer(void, D3D_Flush, (void), 0x417A30);
FunctionPointer(void, D3D_ScreenSwap, (void), 0x441EF0);
FunctionPointer(
	void, D3D_Render2DObject,
	(int x, int y, float z, int w, int h, int tpage,
	int tx, int ty, int tw, int th, int tcolor), 0x40C270
);
FunctionPointer(void, D3D_ClearTextures, (void), 0x4427B0);
FunctionPointer(void, Pad_GetInputs, (void), 0x421330);
FunctionPointer(void, Music_Play, (int trackID), 0x43C210);
FunctionPointer(void, QuitSonicR, (void), 0x439270);

rapidjson::Document menus;

rapidjson::Document dump_json(const char *filepath) {
	FILE* fp;
	char buf[65536];

	fopen_s(&fp, filepath, "rb");
	rapidjson::FileReadStream is(fp, buf, sizeof(buf));

	rapidjson::Document d;
	d.ParseStream(is);
	fclose(fp);

	return d;
}

unsigned int JSON_GetNum(rapidjson::Value &base, const char *prop, unsigned int def)
{
	if (
		base.HasMember(prop) &&
		base[prop].IsInt()
	) {
		return base[prop].GetInt();
	} else if (
		base.HasMember(prop) &&
		base[prop].IsString()
	) {
		return strtoul(base[prop].GetString(), NULL, 0);
	} else {
		return def;
	}
}

struct Inputs {
	bool Up;
	bool Down;
	bool Left;
	bool Right;
	bool A;
	bool B;
	bool C;
	bool Start;
	bool L;
	bool R;
};

struct Inputs Pad_GetInputsUsable(int player) {
	DataPointer(unsigned short, rawinput, (0x4A3608 + 2*player));
	struct Inputs retval = { 0 };

	Pad_GetInputs();

	retval.Up = (rawinput & 0x1000) != 0;
	retval.Down = (rawinput & 0x2000) != 0;
	retval.Left = (rawinput & 0x4000) != 0;
	retval.Right = (rawinput & 0x8000) != 0;
	retval.A = (rawinput & 0x0400) != 0;
	retval.B = (rawinput & 0x0100) != 0;
	retval.C = (rawinput & 0x0040) != 0;
	retval.Start = (rawinput & 0x0800) != 0;
	retval.L = (rawinput & 0x0008) != 0;
	retval.R = (rawinput & 0x0080) != 0;

	return retval;
}

void Text_Draw(const char *text, int x, int y, int color) {
	int len = strlen(text);
	for (int i = 0; i < len; i++) {
		int ty = ((text[i] - 1) >> 4) << 4;
		int tx = ((text[i] - 1) & 0x0F) << 4;
		D3D_Render2DObject(
			x, y, 500.0, 16, 16, 7, tx, ty, 16, 16, color);
		x += 14;
		if (text[i] == '\n') { y += 16; }
	}
}

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

// element interface
class MenuElement {
public:
	int x;
	int y;
	virtual void draw(unsigned int state) = 0;
	virtual void moveTo(int x, int y) = 0;
};

class Menu2DElement: public MenuElement {
public:
	int x_tgt;
	int y_tgt;

	float z;
	int w;
	int h;
	int tpage;
	int tx;
	int ty;
	int tw;
	int th;
	std::vector<unsigned int> colors;

	Menu2DElement(
		int _x, int _y, float _z, int _w, int _h, int _tpage, 
		int _tx, int _ty, int _tw, int _th, std::vector<unsigned int> _colors
	) {
		x = _x; y = _y; z = _z; w = _w; h = _h; tpage = _tpage;
		tx = _tx; ty = _ty; tw = _tw; th = _th; colors = _colors;
	}

	// Render element to screen
	void draw(unsigned int state) {
		int color;
		if (state > colors.size()) {
			color = colors[colors.size()];
		} else {
			color = colors[state];
		}
		//PrintDebug("Drawing at %d:%d\n", x, y);
		D3D_Render2DObject(x, y, z, w, h, tpage, tx, ty, tw, th, color);
	}

	void moveTo(int _x, int _y) {
		x = _x; y = _y;
	}
};

// Collection of MenuElements
class MenuLayer: public MenuElement {
public:
	std::vector<MenuElement *> elems;
	std::vector<unsigned int> states;

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < elems.size(); i++) {
			elems[i]->draw(states[i]);
		}
	}

	void moveTo(int _x, int _y) {
		//PrintDebug("%d -> %d\n", y, _y);
		for (unsigned int i = 0; i < elems.size(); i++) {
			//PrintDebug("\t%d -> %d\n", elems[i]->y, _y - (y - elems[i]->y));
			elems[i]->moveTo(_x - (x - elems[i]->x), _y - (y - elems[i]->y));
		}
		x = _x;
		y = _y;
	}
};

// A whole bunch of 2D elements
class MenuText : public MenuLayer {
public:
	MenuText(
		const char *text,
		int _x, int _y,
		std::vector<unsigned int> _colors
	) {
		int x = _x;
		for (unsigned int i = 0; i < strlen(text); i++) {
			int _ty = ((text[i] - 1) >> 4) << 4;
			int _tx = ((text[i] - 1) & 0x0F) << 4;

			Menu2DElement *elem = new Menu2DElement(
				x, _y, 500.0, 16, 16, 7, _tx, _ty, 16, 16, _colors
			);
			elems.push_back(elem);
			states.push_back(0);
			x += 14;
		}
	}

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < elems.size(); i++) {
			elems[i]->draw(state);
		}
	}
};

// A generic menu selection layer
class MenuSelector : public MenuLayer {
public:
	int selection = 0;
	int speed = 0;
	virtual void scroll(int amount) = 0;
};

// A menu selection layer that scrolls vertically
class MenuSelectorSimpleVertical : public MenuSelector {
public:
	MenuSelectorSimpleVertical(rapidjson::Value &a) {
		PrintDebug("Creating menu from file...\n");
		// Fetch layout properties
		std::vector<unsigned int> colors;
		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
		} else {
			colors.push_back(0xFF666666);
			colors.push_back(0xFF66FF66);
			x = 20;
			y = 60;
		}

		// Fill text
		int _x = x;
		int _y = y;
		for (rapidjson::SizeType i = 0; i < a["options"].Size(); i++) {
			//PrintDebug("Making text %s\n", a["options"][i]["text"].GetString());
			MenuText *text = new MenuText(
				a["options"][i]["text"].GetString(), _x, _y, colors
			);
			elems.push_back(text);
			states.push_back(0);
			_y += 20;
		}
	}

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < elems.size(); i++) {
			states[i] = (selection == i);
			elems[i]->draw(states[i]);
		}
	}

	void scroll(int amount) {
		selection += amount;
		if (selection < 0) { selection = 0; }
		else if (selection > (elems.size() - 1)) { selection = (elems.size() - 1); }
	}
};

// A menu selection layer that scrolls vertically
class MenuSelectorScrollVertical : public MenuSelector {
private:
	int x_tgt;
	int y_tgt;
	int scrollspeed;
public:
	MenuSelectorScrollVertical(rapidjson::Value &a) {
		PrintDebug("Creating menu from file...\n");
		// Fetch layout properties
		std::vector<unsigned int> colors;
		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
			scrollspeed = JSON_GetNum(a["layout"]["properties"], "scrollspeed", 2);
		} else {
			colors.push_back(0xFF666666);
			colors.push_back(0xFF66FF66);
			x = 20;
			y = 60;
			scrollspeed = 2;
		}
		x_tgt = x;
		y_tgt = y;

		// Fill text
		int _x = x;
		int _y = y;
		for (rapidjson::SizeType i = 0; i < a["options"].Size(); i++) {
			MenuText *text = new MenuText(
				a["options"][i]["text"].GetString(), _x, _y, colors
			);
			elems.push_back(text);
			states.push_back(0);
			_y += 20;
		}
	}

	void draw(unsigned int state) {
		y += scrollspeed * sign(y_tgt - y);
		for (unsigned int i = 0; i < elems.size(); i++) {
			elems[i]->moveTo(x, y);
			states[i] = (selection == i);
			elems[i]->draw(states[i]);
		}
	}

	void scroll(int amount) {
		selection += amount;
		y_tgt -= 20 * amount;
		if (selection < 0) {
			selection = 0; 
			y_tgt -= 20;
		}
		else if (selection >(elems.size() - 1)) { 
			selection = (elems.size() - 1);
			y_tgt += 20;
		}
	}
};

// A menu selection layer that scrolls horizontally

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
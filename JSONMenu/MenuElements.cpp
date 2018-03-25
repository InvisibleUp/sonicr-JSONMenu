#include "stdafx.h"
#include "MenuElements.h"

int Menu_GetIndexFromID(const char *id, rapidjson::Document &menus) {
	for (rapidjson::SizeType i = 0; i < menus.Size(); i++) {
		if (strcmp(menus[i]["id"].GetString(), id) == 0) {
			return i;
		}
	}

	return -1;
}

std::vector<const char *> Menu_LoadGFX(rapidjson::Value &menu) {
	std::vector<const char *> GFXList;

	// Assume D3D, as that's the only thing supported
	D3D_ClearTextures();

	// Required for wobbly oval of doom
	D3D_LoadTexture(0, "general\\player00.raw");
	GFXList.push_back("general\\player00.raw");

	// Load BG (1-6)
	if (menu.HasMember("background") && menu["background"].HasMember("src")) {
		D3D_LoadWallpaper(menu["background"]["src"].GetString());
		for (int i = 0; i < 6; i++) {
			GFXList.push_back(menu["background"]["src"].GetString());
		}
	} else {
		D3D_LoadWallpaper("mods/JSONMenu/nullbg.raw");
		for (int i = 0; i < 6; i++) {
			GFXList.push_back("mods/JSONMenu/nullbg.raw");
		}
	}

	// Required for text
	D3D_LoadTexture(7, "mods\\jsonmenu\\font.raw");
	GFXList.push_back("mods\\jsonmenu\\font.raw");

	// Load textures required for preview and icons
	if (menu.HasMember("options")) {
		for (rapidjson::SizeType i = 0; i < menu["options"].Size(); i++) {
			if (!menu["options"][i].HasMember("icon")) { continue; }
			const char *filepath = menu["options"][i]["icon"]["filepath"].GetString();
			bool fileLoaded = false;

			// Check if already loaded
			std::vector<const char *>::iterator iter = std::find_if(
				GFXList.begin(), GFXList.end(),
				[filepath](const char *a) {return (strcmp(a, filepath) == 0); }
			);
			int tpage = std::distance(GFXList.begin(), iter);

			// If not, add new texture slot
			if (tpage == GFXList.size()) {
				D3D_LoadTexture(GFXList.size(), filepath);
				GFXList.push_back(filepath);
			}
		}
	}

	// Finalize
	D3D_TexturesToGPU();

	return GFXList;
}

// Input is option in question. Defaults to true.
bool Menu_OptEnabled(rapidjson::Value &option) {
	if (!option.HasMember("condition")) { return true; }

	if (option["condition"].HasMember("getInt") && option["condition"].HasMember("value")) {
		return *(int *)(JSON_GetNum(option["condition"], "getInt", 0)) == JSON_GetNum(option["condition"], "value", 0);
	}

	return true;
}

// Creates an icon from JSON
Menu2DElement * Menu_CreateIcon(rapidjson::Value &iconJSON, int _x, int _y, std::vector<const char *> GFXList) {
	const char *filepath = iconJSON["filepath"].GetString();
	int tx = JSON_GetNum(iconJSON, "x", 0);
	int ty = JSON_GetNum(iconJSON, "y", 0);
	int tw = JSON_GetNum(iconJSON, "w", 16);
	int th = JSON_GetNum(iconJSON, "h", 16);
	int scale = JSON_GetNum(iconJSON, "scale", 1);
	std::vector<unsigned int> dummycolors = std::vector<unsigned int>(4, 0xFFFFFFFF);

	std::vector<const char *>::iterator iter = std::find_if(
		GFXList.begin(), GFXList.end(),
		[filepath](const char *a) {return (strcmp(a, filepath) == 0); }
	);
	int tpage = std::distance(GFXList.begin(), iter);

	Menu2DElement *icon = new Menu2DElement(
		_x, _y, 500.0, tw*scale, th*scale, tpage, tx, ty, tw, th, dummycolors
	);

	return icon;
}
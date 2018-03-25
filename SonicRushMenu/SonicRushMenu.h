#pragma once
#include "stdafx.h"
#include "SonicRModLoader.h"
#include "SonicRFuncts.h"
#include "MenuElements.h"

// A menu selection layer that lists elements vertically
class SonicRushTitle : public MenuSelector {
private:
	int globalTimer;
	int bgX;
public:
	SonicRushTitle(rapidjson::Value &a, std::vector<const char *> GFXList) {
		// Fetch layout properties
		std::vector<unsigned int> colors;
		colors.push_back(0xFFFFFFFF);
		x = 0;
		y = 0;
		bgX = 0;
		globalTimer = 0;

		// Add background (256x32 strip; 3 wide by 8 tall)
		MenuLayer *bg = new MenuLayer();
		for (int ty = 0; ty < 8; ty++) {
			for (int tx = 0; tx < 3; tx++) {
				Menu2DElement *tile = new Menu2DElement(
					tx * 512, ty * 64, 500.0,
					512, 64, 8,
					0, 224, 256, 32, colors
				);
				bg->addElem(tile);
			}
		}
		addElem(bg);

		// Add purple arrow (comes in from right)
		Menu2DElement *purpleArrow = new Menu2DElement(640, 40, 500.0, 512, 178, 8, 0, 0, 256, 89, colors);
		addElem(purpleArrow);

		// Add blue arrow (comes in from left with slight delay)
		Menu2DElement *blueArrow = new Menu2DElement(1480, 40, 500.0, 512, 150, 8, 0, 90, 256, 76, colors);
		addElem(blueArrow);

		// SONIC
		Menu2DElement *sonic = new Menu2DElement(120, 70, 500.0, 364, 100, 8, 0, 166, 182, 50, colors);
		addElem(sonic);

		// R
		Menu2DElement *r = new Menu2DElement(360, 160, 500.0, 100, 100, 8, 184, 172, 50, 50, colors);
		addElem(r);

		// PRESS START
		Menu2DElement *start = new Menu2DElement(150, 340, 500.0, 176*2, 24*2, 9, 0, 24, 176, 24, colors);
		addElem(start);
	}

	void draw(unsigned int state) {

		// Moving background
		bgX -= 4;
		if (bgX <= -512) { bgX = 0; }
		getElem(0)->moveTo(bgX, 0);

		// Right arrow
		if (globalTimer < 49) {
			getElem(1)->moveTo(getElem(1)->x - 12, 40);
		}

		// Left arrow
		if (globalTimer < 115) {
			getElem(2)->moveTo(getElem(2)->x - 12, 40);
		}

		// Draw graphics
		for (int i = 0; i < getElemSize(); i++) {
			getElem(i)->draw(0);
		}

		globalTimer += 1;
	}

	void vscroll(int dir) {}
	void hscroll(int dir) {}
	bool doAction(rapidjson::Value &action) { return false; }

	int querySelection() {return 0;}
	int querySubSelection() {return -1;}

	void loadTextures(std::vector<const char *> &GFXList) {
		D3D_LoadTexture(GFXList.size(), "mods/SonicRushMenu/art/titlegfx.raw");
		GFXList.push_back("mods/SonicRushMenu/art/titlegfx.raw");

		D3D_LoadTexture(GFXList.size(), "mods/SonicRushMenu/art/buttons.raw");
		GFXList.push_back("mods/SonicRushMenu/art/buttons.raw");

		D3D_TexturesToGPU();
	}
};

void * SonicRushTitle_Constructor(
	rapidjson::Value &a, std::vector<const char *> GFXList
) {
	return new SonicRushTitle(a, GFXList);
}
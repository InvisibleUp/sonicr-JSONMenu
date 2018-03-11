#pragma once
#include "stdafx.h"
#include "../MenuElements.h"

// A menu selection layer that scrolls vertically
class MenuSelectorSimpleVertical : public MenuSelector {
private:
	int selectionIndexStart;
	int selectionIndexEnd;
	int titleIndex;
	int xspacing;
	int yspacing;
	enum States {NORMAL = 0, HIGHLIGHTED = 1, DISABLED = 2, BRIGHT = 3};
public:
	MenuSelectorSimpleVertical(rapidjson::Value &a, std::vector<const char *> GFXList) {
		// Fetch layout properties
		std::vector<unsigned int> colors;

		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "disabledcolor", 0xFFCCCCCC));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "brightcolor", 0xFFFFFFFF));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
			xspacing = JSON_GetNum(a["layout"]["properties"], "xspacing", 20);
			yspacing = JSON_GetNum(a["layout"]["properties"], "yspacing", 20);
		}
		else {
			colors.push_back(0xFF666666);
			colors.push_back(0xFF66FF66);
			colors.push_back(0xFFCCCCCC);
			colors.push_back(0xFFFFFFFF);
			x = 20;
			y = 60;
		}

		// Add options
		int _y = y;
		selectionIndexStart = getElemSize();

		for (rapidjson::SizeType i = 0; i < a["options"].Size(); i++) {
			MenuLayer *layer = new MenuLayer();
			int _x = x;
			int ycenter = 0;

			if (a["options"][i].HasMember("icon")) {
				const char *filepath = a["options"][i]["icon"]["filepath"].GetString();
				int tx = JSON_GetNum(a["options"][i]["icon"], "x", 0);
				int ty = JSON_GetNum(a["options"][i]["icon"], "y", 0);
				int tw = JSON_GetNum(a["options"][i]["icon"], "w", 16);
				int th = JSON_GetNum(a["options"][i]["icon"], "h", 16);
				int scale = JSON_GetNum(a["options"][i]["icon"], "scale", 1);
				std::vector<unsigned int> dummycolors = std::vector<unsigned int>(4, 0xFFFFFFFF);

				std::vector<const char *>::iterator iter = std::find_if(
					GFXList.begin(), GFXList.end(),
					[filepath](const char *a) {return (strcmp(a, filepath) == 0); }
				);
				int tpage = std::distance(GFXList.begin(), iter);

				Menu2DElement *icon = new Menu2DElement(
					_x, _y, 500.0, tw*scale, th*scale, tpage, tx, ty, tw, th, dummycolors
				);
				_x += tw*scale + xspacing;
				layer->addElem(icon);
				layer->addState(BRIGHT);

				ycenter = icon->h / 2;
			}

			if (a["options"][i].HasMember("text")) {
				if (ycenter >= 8) { ycenter -= 8; } // 1/2 font height

				MenuText *text = new MenuText(
					a["options"][i]["text"].GetString(), _x, _y + ycenter, colors
				);
				layer->addElem(text);
				layer->addState(BRIGHT);
			}

			if (Menu_OptEnabled(a["options"][i])) {
				states.push_back(NORMAL);
			} else {
				states.push_back(DISABLED);
			}

			addElem(layer);
			_y += layer->h + yspacing;
		}
		selectionIndexEnd = getElemSize() - 1;

		// Add title
		if (a.HasMember("title")) {
			MenuText *text = new MenuText(
				a["title"].GetString(), 0, 0, colors
			);
			text->moveTo(320 - (text->w / 2), 20);
			addElem(text);
			titleIndex = getElemSize() - 1;
		} else {
			titleIndex = -1;
		}
	}

	void draw(unsigned int state) {
		// Options
		for (int i = selectionIndexStart; i <= selectionIndexEnd; i++) {
			if (states[i] != DISABLED) { states[i] = (selection == i); }
			getElem(i)->draw(states[i]);
		}

		// Title
		if (titleIndex != -1) { getElem(titleIndex)->draw(HIGHLIGHTED); }
	}

	void vscroll(int amount) {
		int oldSelection = selection;
		do {
			selection += amount;
			if (selection < selectionIndexStart) { 
				selection = oldSelection; 
				break; 
			} else if (selection > selectionIndexEnd) {
				selection = oldSelection;
				break; 
			}
		} while (states[selection] == DISABLED);
	}
	void hscroll(int amount) { return; }
	bool doAction(rapidjson::Value &action) { return false; }
};

void * MenuSelectorSimpleVertical_Constructor(
	rapidjson::Value &a, std::vector<const char *> GFXList
) {
	return new MenuSelectorSimpleVertical(a, GFXList);
}
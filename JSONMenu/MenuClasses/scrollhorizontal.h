#pragma once
#include "stdafx.h"
#include "../MenuElements.h"

// A menu selection layer that scrolls horizontally
class MenuSelectorScrollHorizontal : public MenuSelector {
private:
	int x_tgt;
	int y_tgt;
	int scrollspeed;
	int spacing;
	enum States { NORMAL = 0, HIGHLIGHTED = 1, DISABLED = 2 };
public:
	MenuSelectorScrollHorizontal(rapidjson::Value &a, std::vector<const char *> GFXList) {
		// Fetch layout properties
		std::vector<unsigned int> colors;
		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "disabledcolor", 0xFFCCCCCC));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
			scrollspeed = JSON_GetNum(a["layout"]["properties"], "scrollspeed", 4);
			spacing = JSON_GetNum(a["layout"]["properties"], "spacing", 20);
		}
		else {
			colors.push_back(0xFF666666);
			colors.push_back(0xFF66FF66);
			colors.push_back(0xFFCCCCCC);
			x = 20;
			y = 60;
			scrollspeed = 4;
			spacing = 20;
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
			if (Menu_OptEnabled(a["options"][i])) {
				states.push_back(NORMAL);
			} else {
				states.push_back(DISABLED);
			}
			addElem(text);
			_x += spacing + text->w;
		}
	}

	void draw(unsigned int state) {
		int oldSign = sign(x_tgt - x);
		x += scrollspeed * oldSign;
		if (sign(x_tgt - x) != oldSign) {
			x = x_tgt;
		}

		for (int i = 0; i < getElemSize(); i++) {
			getElem(i)->moveTo(x, y);
			if (states[i] != DISABLED) { states[i] = (selection == i); }
			getElem(i)->draw(states[i]);
		}
	}

	void vscroll(int amount) { return; }
	void hscroll(int amount) {
		int oldSelection = selection;
		int oldXtgt = x_tgt;
		do {
			selection += amount;
			if (amount < 0 && selection >= 0) {
				x_tgt += getElem(selection)->w + spacing;
			}
			else if (amount > 0 && selection < getElemSize()) {
				x_tgt -= getElem(selection - 1)->w + spacing;
			}
			else {
				selection = oldSelection;
				x_tgt = oldXtgt;
				break;
			}
		} while (states[selection] == DISABLED);
	}
	bool doAction(rapidjson::Value &action) { return false; }
};

void * MenuSelectorScrollHorizontal_Constructor(
	rapidjson::Value &a, std::vector<const char *> GFXList
) {
	return new MenuSelectorScrollHorizontal(a, GFXList);
}
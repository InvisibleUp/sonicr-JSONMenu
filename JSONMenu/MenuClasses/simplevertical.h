#pragma once
#include "stdafx.h"
#include "../MenuElements.h"

// A menu selection layer that scrolls vertically
class MenuSelectorSimpleVertical : public MenuSelector {
public:
	MenuSelectorSimpleVertical(rapidjson::Value &a) {
		// Fetch layout properties
		std::vector<unsigned int> colors;
		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
		}
		else {
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
			addElem(text);
			states.push_back(0);
			_y += 20;
		}
	}

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < getElemSize(); i++) {
			states[i] = (selection == i);
			getElem(i)->draw(states[i]);
		}
	}

	void scroll(int amount) {
		selection += amount;
		if (selection < 0) { selection = 0; }
		else if (selection >(getElemSize() - 1)) { selection = (getElemSize() - 1); }
	}
};
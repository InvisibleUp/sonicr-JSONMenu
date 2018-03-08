#pragma once
#include "stdafx.h"
#include "../MenuElements.h"

// A menu selection layer that scrolls horizontally
class MenuSelectorScrollHorizontal : public MenuSelector {
private:
	int x_tgt;
	int y_tgt;
	int scrollspeed;
public:
	MenuSelectorScrollHorizontal(rapidjson::Value &a) {
		// Fetch layout properties
		std::vector<unsigned int> colors;
		if (a.HasMember("layout") && a["layout"].HasMember("properties")) {
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "basecolor", 0xFF666666));
			colors.push_back(JSON_GetNum(a["layout"]["properties"], "selcolor", 0xFF66FF66));
			x = JSON_GetNum(a["layout"]["properties"], "xoff", 20);
			y = JSON_GetNum(a["layout"]["properties"], "yoff", 60);
			scrollspeed = JSON_GetNum(a["layout"]["properties"], "scrollspeed", 2);
		}
		else {
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
			addElem(text);
			states.push_back(0);
			_x += 20 + text->w;
		}
	}

	void draw(unsigned int state) {
		x += scrollspeed * sign(x_tgt - x);
		for (unsigned int i = 0; i < getElemSize(); i++) {
			getElem(i)->moveTo(x, y);
			states[i] = (selection == i);
			getElem(i)->draw(states[i]);
		}
	}

	void scroll(int amount) {
		selection += amount;
		if (amount < 0 && selection > 0) {
			x_tgt -= getElem(selection)->w;
		} else if (amount > 0 && selection < getElemSize()) {
			x_tgt += getElem(selection)->h;
		}
	}
};
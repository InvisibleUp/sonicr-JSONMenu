#pragma once
#include "stdafx.h"
#include "MenuElements.h"

/*// A menu selection layer that scrolls horizontally
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
	int querySelection() { return selection; }
	int querySubSelection() { return -1; }
	void loadTextures(std::vector<const char *> &GFXList){}
};*/

class MenuSelectorScrollHorizontal : public MenuSelector {
private:
	int x_tgt;
	int y_tgt;
	int scrollspeed;
	int selectionIndexStart;
	int selectionIndexEnd;
	int titleIndex;
	int xspacing;
	int yspacing;
	enum States { NORMAL = 0, HIGHLIGHTED = 1, DISABLED = 2, BRIGHT = 3 };
	std::vector<unsigned int> choiceSizes;
	std::vector<unsigned int> choiceSelections;
public:
	MenuSelectorScrollHorizontal(rapidjson::Value &a, std::vector<const char *> GFXList) {
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
			scrollspeed = JSON_GetNum(a["layout"]["properties"], "scrollspeed", 4);
		}
		else {
			colors.push_back(0xFF666666);
			colors.push_back(0xFF66FF66);
			colors.push_back(0xFFCCCCCC);
			colors.push_back(0xFFFFFFFF);
			x = 20;
			y = 60;
			scrollspeed = 4;
		}

		// Add options
		int _x = x;
		x_tgt = x;
		y_tgt = y;
		selectionIndexStart = getElemSize();

		if (a.HasMember("options")) {
			for (rapidjson::SizeType i = 0; i < a["options"].Size(); i++) {
				MenuLayer *layer = new MenuLayer();
				int _y = y;
				int choiceCount = 1;

				if (a["options"][i].HasMember("icon")) {
					MenuElement *icon = Menu_CreateIcon(
						a["options"][i]["icon"], _x, _y, GFXList
					);
					layer->addElem(icon);
					layer->addState(BRIGHT);
					_y += icon->h + yspacing;
				}

				if (a["options"][i].HasMember("text")) {
					MenuText *text = new MenuText(
						a["options"][i]["text"].GetString(), _x, _y, colors
					);
					layer->addElem(text);
					layer->addState(BRIGHT);
					_y += text->h + yspacing;
				}

				if (Menu_OptEnabled(a["options"][i])) {
					states.push_back(NORMAL);
				}
				else {
					states.push_back(DISABLED);
				}
				addElem(layer);

				// Add in choices
				if (a["options"][i].HasMember("choices")) {
					for (unsigned int j = 0; j < a["options"][i]["choices"].Size(); j++) {
						MenuLayer *layer = new MenuLayer();
						int y_bak = _y;

						if (a["options"][i]["choices"][j].HasMember("icon")) {
							MenuElement *icon = Menu_CreateIcon(a["options"][i]["choices"][j], _x, _y, GFXList);
							layer->addElem(icon);
							layer->addState(BRIGHT);
							_y += icon->h + yspacing;
						}

						if (a["options"][i]["choices"][j].HasMember("text")) {
							//if (ycenter >= 8) { ycenter -= 8; } // 1/2 font height

							MenuText *text = new MenuText(
								a["options"][i]["choices"][j]["text"].GetString(), _x, _y, colors
							);
							layer->addElem(text);
							layer->addState(BRIGHT);
						}
						_y = y_bak;
						choiceCount += 1;

						if (Menu_OptEnabled(a["options"][i]["choices"][j])) {
							states.push_back(NORMAL);
						}
						else {
							states.push_back(DISABLED);
						}
						addElem(layer);
					}
				}

				choiceSizes.push_back(choiceCount);
				if (choiceCount == 1) {
					choiceSelections.push_back(0);
				}
				else {
					choiceSelections.push_back(1);
				}
				_x += layer->w + xspacing;
			}
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
		}
		else {
			titleIndex = -1;
		}
	}

	void draw(unsigned int state) {
		int oldSign = sign(x_tgt - x);
		x += scrollspeed * oldSign;
		if (sign(x_tgt - x) != oldSign) {
			x = x_tgt;
		}

		// Options
		int i = 0, pos = 0;
		while (pos <= selectionIndexEnd) {
			if (states[pos] != DISABLED) { states[pos] = (selection == i); }
			getElem(pos)->moveTo(x, y);
			getElem(pos)->draw(states[pos]);

			int optPos = pos + choiceSelections[i];
			if (choiceSelections[i] != 0) {
				getElem(optPos)->moveTo(x, y);
				getElem(optPos)->draw(states[optPos]);
			}

			pos += choiceSizes[i];
			i += 1;
		}

		// Title
		if (titleIndex != -1) { getElem(titleIndex)->draw(HIGHLIGHTED); }
	}

	void hscroll(int dir) {
		do {
			if (dir < 0) {
				if (selection <= 0) {
					break;
				}
				selection -= 1;

				int pos = 0, i = 0;
				while (pos < selection) {
					int optPos = pos + choiceSelections[i];
					pos += choiceSizes[i];
					i += 1;
				}
				x_tgt += (getElem(pos)->w + xspacing);
			}
			else if (dir > 0) {
				if (selection >= choiceSizes.size() - 1) {
					break;
				}
				selection += 1;

				int pos = 0, i = 0;
				while (pos < selection - 1) {
					int optPos = pos + choiceSelections[i];
					pos += choiceSizes[i];
					i += 1;
				}
				x_tgt -= (getElem(pos)->w + xspacing);
			}
		} while (states[selection] == DISABLED);
	}
	void vscroll(int dir) {
		if (choiceSelections[selection] == 0) { return; }

		if (dir < 0) {
			if (choiceSelections[selection] <= 1) {
				return;
			}
			choiceSelections[selection] -= 1;
		}
		else if (dir > 0) {
			if (choiceSelections[selection] >= choiceSizes[selection] - 1) {
				return;
			}
			choiceSelections[selection] += 1;
		}
	}

	bool doAction(rapidjson::Value &action) { return false; }

	int querySelection() {
		int pos = 0;
		for (int i = 0; i < selection; i++) {
			pos += choiceSizes[i];
		}

		if (states[pos] == DISABLED) {
			return -1;
		}
		else {
			return selection;
		}
	}
	int querySubSelection() {
		int pos = 0;
		for (int i = 0; i < selection; i++) {
			pos += choiceSizes[i];
		}
		int optPos = pos + choiceSelections[selection];

		if (states[optPos] == DISABLED) {
			return -1;
		}
		else {
			return choiceSelections[selection] - 1;
		}
	}
	void loadTextures(std::vector<const char *> &GFXList) {}
};

void * MenuSelectorScrollHorizontal_Constructor(
	rapidjson::Value &a, std::vector<const char *> GFXList
) {
	return new MenuSelectorScrollHorizontal(a, GFXList);
}

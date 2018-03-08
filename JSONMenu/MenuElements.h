#pragma once
#include "stdafx.h"
// element interface
class MenuElement {
public:
	int x;
	int y;
	int w;
	int h;
	virtual void draw(unsigned int state) = 0;
	virtual void moveTo(int x, int y) = 0;
};

class Menu2DElement : public MenuElement {
public:
	int x_tgt;
	int y_tgt;

	float z;
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
		}
		else {
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
class MenuLayer : public MenuElement {
private:
	std::vector<MenuElement *> elems;
public:
	std::vector<unsigned int> states;

	void addElem(MenuElement *elem) {
		elems.push_back(elem);

		// Recalculate width and height
		int xLow = std::numeric_limits<int>::min();
		int xHigh = std::numeric_limits<int>::max();
		int yLow = std::numeric_limits<int>::min();
		int yHigh = std::numeric_limits<int>::max();

		for (int i = 0; i < elems.size(); i++) {
			if (elems[i]->x > xLow) { xLow = elems[i]->x; }
			if ((elems[i]->x + elems[i]->w) < xHigh) { xHigh = (elems[i]->x + elems[i]->w); }
			if (elems[i]->x > yLow) { yLow = elems[i]->y; }
			if ((elems[i]->y + elems[i]->h) < yHigh) { xHigh = (elems[i]->y + elems[i]->h); }
		}

		w = xHigh - xLow;
		h = yHigh - yLow;
	}

	int getElemSize() {
		return elems.size();
	}

	MenuElement *getElem(int i) {
		return elems[i];
	}

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < elems.size(); i++) {
			elems[i]->draw(states[i]);
		}
	}

	void moveTo(int _x, int _y) {
		for (unsigned int i = 0; i < elems.size(); i++) {
			//PrintDebug("\t%d -> %d\n", elems[i]->y, _y - (y - elems[i]->y));
			elems[i]->moveTo(_x - (x - elems[i]->x), _y - (y - elems[i]->y));
		}
		x = _x;
		y = _y;
	}
};

// A whole bunch of 2D elements representing text
class MenuText : public MenuLayer {
public:
	MenuText(
		const char *text,
		int _x, int _y,
		std::vector<unsigned int> _colors
	) {
		int x = _x;

		// TODO: Replace with call to a Font class
		for (unsigned int i = 0; i < strlen(text); i++) {
			int _ty = ((text[i] - 1) >> 4) << 4;
			int _tx = ((text[i] - 1) & 0x0F) << 4;

			Menu2DElement *elem = new Menu2DElement(
				x, _y, 500.0, 16, 16, 7, _tx, _ty, 16, 16, _colors
			);

			addElem(elem);
			states.push_back(0);
			x += 14;
		}

		//h = 14;
		//w = _x - x;
	}

	void draw(unsigned int state) {
		for (unsigned int i = 0; i < getElemSize(); i++) {
			getElem(i)->draw(state);
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
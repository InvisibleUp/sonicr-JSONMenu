#include "stdafx.h"

struct Inputs Pad_GetInputsUsable(int player) {
	DataPointer(unsigned short, rawinput, (0x4A3608 + 2 * player));
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

void RandomizeGPPlayers() {
	std::vector<bool> taken = std::vector<bool>(Char_ModelCount, false);
	taken[P1_Identity] = true;

	for (int i = 1; i < Player_Count; i++) {
		short newChar;
		do {
			newChar = rand() % Char_ModelCount;
		} while (
			taken[newChar] == true ||
			Char_Unlocked[newChar] == 0
		);
		taken[newChar] = true;
		PrintDebug("Player %d is %d\n", i, newChar);

		// Set player identity to newChar
		*(short *)(0x7B747A + i * 0x71C) = newChar;
	}
}
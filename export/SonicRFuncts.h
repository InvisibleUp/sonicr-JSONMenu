#pragma once
#include "stdafx.h"

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

// Memory addresses
DataPointer(int, P1_Identity, 0x7B747A);
DataPointer(int, Char_ModelCount, 0x753504);
DataPointer(int, Player_Count, 0x85DF28);
DataPointer(int, Track_ID, 0x7af13c);
DataArray(int, Char_Unlocked, 0x752BE0, 10);
DataPointer(int, Oval_Speed, 0x5F388c);
DataPointer(int, Oval_Size, 0x7BCB28);

// Helper functions
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
struct Inputs Pad_GetInputsUsable(int player);

void RandomizeGPPlayers();
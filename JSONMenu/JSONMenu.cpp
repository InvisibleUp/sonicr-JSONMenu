// JSONMenu.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SonicRModLoader.h"

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
	(int x, int y, int z, int sx, int sy, int a, int b, int c, int d, int e, int tint), 0x40C270
);
FunctionPointer(void, D3D_ClearTextures, (void), 0x4427B0);

void dump_json() {

}

int Test_Loop() {
	// Create variables
	int Menu_TimeLeft = 300; //Not what it was, but who cares?

	PrintDebug("\nTest Game Loop\n\n");
	//Unk01 = 1;
	
	// Assume D3D, as that's the only thing supported
	D3D_ClearTextures();
	D3D_LoadTexture(0, "general\\player00.raw");
	D3D_LoadWallpaper("bin\\titles\\ttlogo.raw");
	D3D_TexturesToGPU();

	while(Menu_TimeLeft > 0){
		PrintDebug("\r%d frames remaining", Menu_TimeLeft);

		User32_PeekMessageA();
		D3D_BeginScene();

			// Render some, any, 2D texture (works!)
			D3D_Render2DObject(20, 20, 0x447A000, 0x140, 0x40, 0, 20, 20, 0x9F, 0x1F, 0xFFE0E0E0);

			D3D_RenderBG(0); // Renders the background
			D3D_Flush();

		D3D_EndScene();
		D3D_ScreenSwap();

		Menu_TimeLeft -= 1;
		FrameDelay(30);
	}
	return 1;
}

extern "C" {
	__declspec(dllexport) ModInfo SonicRModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init() {
		// Replace Multiplayer menu with our own loop.
		WriteJump((void *)0x444E50, &Test_Loop);
	}
}
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "API.h"
#include "SonicRushMenu.h"

rapidjson::Document dump_json(const char *filepath) {
	FILE* fp;
	char *buf = (char *)malloc(65536);

	fopen_s(&fp, filepath, "rb");
	if (fp == 0) { return rapidjson::Document(); }
	rapidjson::FileReadStream is(fp, buf, sizeof(buf));

	rapidjson::Document d;
	d.ParseStream(is);
	fclose(fp);

	free(buf);
	return d;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: {
		// Load functions from DLL (this is dangerous here, actually, but it works...)
		HMODULE JSONMenu = LoadLibrary(L"mods/JSONMenu/JSONMenu.dll");
		AddMenuClass_Type AddMenuClass_DLL = (AddMenuClass_Type)GetProcAddress(JSONMenu, "AddMenuClass");
		AddMenu_Type AddMenu_DLL = (AddMenu_Type)GetProcAddress(JSONMenu, "AddMenu");

		// Register class
		AddMenuClass_DLL(std::pair<
			const char *, 
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)>(
				"SonicRushTitle", SonicRushTitle_Constructor
			)
		);

		// Replace menus with ours
		rapidjson::Document intro = dump_json("mods/SonicRushMenu/menus/intro.json");
		auto tmp = intro.GetType();
		AddMenu_DLL(intro.GetObject());

		FreeLibrary(JSONMenu);

		break;
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" {
	__declspec(dllexport) ModInfo SonicRModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init() {}
}

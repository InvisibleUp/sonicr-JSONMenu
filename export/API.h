#pragma once
#include "rapidjson\document.h"
#include <vector>

extern "C" {
	__declspec(dllexport) void __cdecl AddMenuClass(
		std::pair<
			std::string,
			void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
		> value
	);
	__declspec(dllexport) bool __cdecl RemoveMenuByID(const char * menuID);
	__declspec(dllexport) bool __cdecl AddMenu(rapidjson::Value menu);
	__declspec(dllexport) bool __cdecl RemoveMenuOption(const char *menuID, const char *optionID);
	__declspec(dllexport) bool __cdecl AddMenuOption(const char * menuID, rapidjson::Value option);
}

// Win32 GetProcAddress helpers
typedef void(*AddMenuClass_Type)(std::pair<
	std::string,
	void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
>);
typedef bool(*RemoveMenuByID_Type)(const char *);
typedef bool(*AddMenu_Type)(rapidjson::Value menu);
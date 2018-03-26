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
	__declspec(dllexport) bool __cdecl AddMenuOption(const char *menuID, rapidjson::Value option);
}

__declspec(dllexport) int __cdecl MajorVersion();
__declspec(dllexport) int __cdecl MinorVersion();

// Win32 GetProcAddress helpers
typedef void(__cdecl *AddMenuClass_Type)(std::pair<
	std::string,
	void *(*) (rapidjson::Value &a, std::vector<const char *> GFXList)
>);
typedef bool(__cdecl *RemoveMenuByID_Type)(const char *);
typedef bool(__cdecl *AddMenu_Type)(rapidjson::Value menu);
typedef bool(__cdecl *RemoveMenuOption_Type)(const char *menuID, const char *optionID);
typedef bool(__cdecl *AddMenuOption_Type)(const char *menuID, rapidjson::Value option);
typedef int(__cdecl *MajorVersion_Type)();
typedef int(__cdecl *MinorVersion_Type)();
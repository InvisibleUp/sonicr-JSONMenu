#include "stdafx.h"
#include "bass_vgmstream.h"

const float volumelevels[] = { 0, 0.002511886f, 0.007079458f, 0.031622777f, 0.079432823f, 0.125892541f, 0.354813389f, 0.630957344f, 1 };
int basschan = 0; // Might conflict with CD playback function!

// DLL import definitions
typedef HSTREAM(WINAPI *BASS_StreamCreateFile_Type)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
typedef HSTREAM(*BASS_VGMSTREAM_StreamCreate_Type)(const char* file, DWORD flags);
typedef BOOL(WINAPI *BASS_ChannelStop_Type)(DWORD handle);
typedef BOOL(WINAPI *BASS_StreamFree_Type)(HSTREAM handle);
typedef BOOL(WINAPI *BASS_ChannelPlay_Type)(DWORD handle, BOOL restart);
typedef BOOL(WINAPI *BASS_ChannelSetAttribute_Type)(DWORD handle, DWORD attrib, float value);
typedef HSYNC(WINAPI *BASS_ChannelSetSync_Type)(DWORD handle, DWORD type, QWORD param, SYNCPROC *proc, void *user);

static void __stdcall onTrackEnd(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	HMODULE BASS = LoadLibrary(L"bass.dll");
	auto BASS_ChannelStop_DLL = (BASS_ChannelStop_Type)GetProcAddress(BASS, "BASS_ChannelStop");
	auto BASS_StreamFree_DLL = (BASS_StreamFree_Type)GetProcAddress(BASS, "BASS_StreamFree");

	BASS_ChannelStop_DLL(channel);
	BASS_StreamFree_DLL(channel);
	CurrentMusicTrack = -1;
	*(int*)0x502F0C = 0;

	FreeLibrary(BASS);
}

void StopMusic() {
	HMODULE BASS = LoadLibrary(L"bass.dll");
	auto BASS_ChannelStop_DLL = (BASS_ChannelStop_Type)GetProcAddress(BASS, "BASS_ChannelStop");
	auto BASS_StreamFree_DLL = (BASS_StreamFree_Type)GetProcAddress(BASS, "BASS_StreamFree");

	if (basschan != 0) {
		BASS_ChannelStop_DLL(basschan);
		BASS_StreamFree_DLL(basschan);
	}

	FreeLibrary(BASS);
}

void PlayMusicFromFile(const char *filepath)
{
	// Load DLL and functions
	HMODULE BASS = LoadLibrary(L"bass.dll");
	HMODULE BASSVGM = LoadLibrary(L"bass_vgmstream.dll");

	auto BASS_StreamCreateFile_DLL = (BASS_StreamCreateFile_Type)GetProcAddress(BASS, "BASS_StreamCreateFile");
	auto BASS_VGMSTREAM_StreamCreate_DLL = (BASS_VGMSTREAM_StreamCreate_Type)GetProcAddress(BASSVGM, "BASS_VGMSTREAM_StreamCreate");
	auto BASS_ChannelStop_DLL = (BASS_ChannelStop_Type)GetProcAddress(BASS, "BASS_ChannelStop");
	auto BASS_StreamFree_DLL = (BASS_StreamFree_Type)GetProcAddress(BASS, "BASS_StreamFree");
	auto BASS_ChannelPlay_DLL = (BASS_ChannelPlay_Type)GetProcAddress(BASS, "BASS_ChannelPlay");
	auto BASS_ChannelSetAttribute_DLL = (BASS_ChannelSetAttribute_Type)GetProcAddress(BASS, "BASS_ChannelSetAttribute");
	auto BASS_ChannelSetSync_DLL = (BASS_ChannelSetSync_Type)GetProcAddress(BASS, "BASS_ChannelSetSync");

	if (basschan != 0) {
		BASS_ChannelStop_DLL(basschan);
		BASS_StreamFree_DLL(basschan);
	}

	basschan = BASS_VGMSTREAM_StreamCreate_DLL(filepath, 0);
	if (basschan == 0) {
		basschan = BASS_StreamCreateFile_DLL(false, filepath, 0, 0, 0);
	}

	if (basschan != 0)
	{
		// Stream opened!
		BASS_ChannelPlay_DLL(basschan, false);
		BASS_ChannelSetAttribute_DLL(basschan, BASS_ATTRIB_VOL, volumelevels[MusicVolume]);
		BASS_ChannelSetSync_DLL(basschan, BASS_SYNC_END, 0, onTrackEnd, nullptr);
		CurrentMusicTrack = -2;
		*(int*)0x502F0C = 1;
	} else {
		CurrentMusicTrack = -1;
		*(int*)0x502F0C = 0;
		PrintDebug("Could not play music file \"%s\".", filepath);
	}

	FreeLibrary(BASS);
}
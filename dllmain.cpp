#include "stdafx.h"
#include "dynamic_lib.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitializeHooks, 0, 0, 0);
		case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

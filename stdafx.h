// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Windows.h>
#include <comutil.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iterator>
#include <memory>
#include <tuple>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <locale>
#include <codecvt>

#include "stdafx.h"
#include "detours.h"
#include "Player.h"
#include "Npch.h"
#include "dynamic_lib.h"

// TODO: reference additional headers your program requires here

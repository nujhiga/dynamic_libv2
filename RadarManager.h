#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

#include "PacketManager.h"
namespace pm = PacketManager;

#include "dynamic_lib.h"

namespace RadarManager {
	BOOL SetupRadar(const std::string& windowTitle);
	BOOL InitRadar(const std::string& windowTitle);
	VOID FinalizeRadar();
	VOID InitBrushes();
	VOID ReleaseBrushes();
	VOID DisposeObjects();
	VOID RadarRenderThread();
}


#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

#include "PacketManager.h"
namespace pm = PacketManager;

#include "dynamic_lib.h"

namespace RadarManager {
	bool SetupRadar(const std::string& windowTitle);
	bool InitRadar(const std::string& windowTitle);
	void FinalizeRadar();
	void InitBrushes();
	void ReleaseBrushes();
	void DisposeObjects();
	void RadarRenderThread();
}


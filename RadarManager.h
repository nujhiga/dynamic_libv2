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
	void RadarRender();
	void RenderPos(const HFONT& font);
	void RenderUser();
	void RenderPlayers();
	RECT GetRectRadar(int left, int top, int right, int bottom, float scale = 1.0f);
}


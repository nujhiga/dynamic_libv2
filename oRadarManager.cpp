#include "stdafx.h"
#include "oRadarManager.h"


namespace oRadarManager {

	std::atomic<bool> runRadar{ false };

	HANDLE radarHandle;
	HWND hWnd;

	HDC hdcWindow;
	HDC hdcCompatible;

	HFONT hFont;

	HBRUSH radarBrush;
	HBRUSH neutralBrush;
	HBRUSH ciudasBrush;
	HBRUSH crimisBrush;
	HBRUSH gmsBrush;
	HBRUSH unknownBrush;
	HBRUSH blackBrush;
	HBRUSH playerBrush;

	const int RADAR_RECT_LEFT = 0;
	const int RADAR_RECT_TOP = 0;
	const int RADAR_RECT_RIGHT = 100;
	const int RADAR_RECT_BOTTOM = 100;

	const int MAP_LIMITS_LEFT = 10;
	const int MAP_LIMITS_TOP = 10;
	const int MAP_LIMITS_RIGHT = 90;
	const int MAP_LIMITS_BOTTOM = 92;

	const int ENTITY_OFFSET = 3;
	const int USER_VISION_YOFFSET = 11;
	const int USER_VISION_XOFFSET = 9;

	const float RADAR_SCALE = 1.5f;
	const int FIX_FONT_SIZE = 6;
	const int FONT_SIZE = static_cast<int>((-MulDiv(FIX_FONT_SIZE, GetDeviceCaps(hdcWindow, LOGPIXELSY), 72)) * RADAR_SCALE);

	//bool InitRadar(const std::string& windowTitle) {
	//	if (!SetupRadar(windowTitle)) return false;
	//	InitBrushes();
	//	runRadar.store(true);
	//	radarHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RadarRender, 0, 0, 0);
	//	return true;
	//}

	bool InitRadar(const std::string& windowTitle) {
		if (!SetupRadar(windowTitle)) return false;

		InitBrushes();
		runRadar.store(true);
		//radarHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RadarRender, 0, 0, 0);

		return true;
	}

	void FinalizeRadar() {

		runRadar.store(false);
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		DisposeObjects();

	}

	//void FinalizeRadar() {
	//	runRadar.store(false);
	//	SendMessage(hWnd, WM_CLOSE, 0, 0);
	//	WaitForSingleObject(radarHandle, INFINITE);
	//	DisposeObjects();
	//	CloseHandle(radarHandle);
	//	radarHandle = nullptr;
	//}

	bool SetupRadar(const std::string& windowTitle) {

		std::wstring wndTitle(windowTitle.begin(), windowTitle.end());
		hWnd = FindWindow(NULL, wndTitle.c_str());

		if (hWnd == NULL) {
			dlg.logData("RadarManager - Failed to find window: " + windowTitle, DlibLogger::LogType::DLIB);
			return false;
		}

		hdcWindow = GetDC(hWnd);
		if (hdcWindow == NULL) {
			dlg.logData("RadarManager - Failed to get DC", DlibLogger::LogType::DLIB);
			return false;
		}

		hdcCompatible = CreateCompatibleDC(hdcWindow);
		if (hdcCompatible == NULL) {
			dlg.logData("RadarManager - Failed to create compatible HDC", DlibLogger::LogType::DLIB);
			ReleaseDC(hWnd, hdcWindow);
			return false;
		}

		return true;
	}

	void InitBrushes() {
		radarBrush = CreateSolidBrush(RGB(0, 255, 0));
		neutralBrush = CreateSolidBrush(RGB(127, 127, 127));
		ciudasBrush = CreateSolidBrush(RGB(0, 162, 232));
		crimisBrush = CreateSolidBrush(RGB(255, 0, 0));
		gmsBrush = CreateSolidBrush(RGB(255, 255, 255));
		unknownBrush = CreateSolidBrush(RGB(255, 242, 0));
		blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		playerBrush = CreateSolidBrush(RGB(0, 0, 255));

		hFont = CreateFontW(FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");
	}

	void ReleaseBrushes() {

		DeleteObject(radarBrush);
		radarBrush = nullptr;

		DeleteObject(neutralBrush);
		neutralBrush = nullptr;

		DeleteObject(ciudasBrush);
		ciudasBrush = nullptr;

		DeleteObject(crimisBrush);
		crimisBrush = nullptr;

		DeleteObject(gmsBrush);
		gmsBrush = nullptr;

		DeleteObject(unknownBrush);
		unknownBrush = nullptr;

		DeleteObject(blackBrush);
		blackBrush = nullptr;

		DeleteObject(playerBrush);
		playerBrush = nullptr;
	}

	void DisposeObjects() {
		ReleaseBrushes();
		DeleteObject(hFont);
		DeleteDC(hdcCompatible);
		ReleaseDC(hWnd, hdcWindow);
	}

	void RadarRender(int userX, int userY, const std::unordered_map<int, std::shared_ptr<Player>>& mapPlayers) {

		RECT radar = GetRectRadar(RADAR_RECT_LEFT, RADAR_RECT_TOP, RADAR_RECT_RIGHT, RADAR_RECT_BOTTOM, RADAR_SCALE);
		RECT mapLimits = GetRectRadar(MAP_LIMITS_LEFT, MAP_LIMITS_TOP, MAP_LIMITS_RIGHT, MAP_LIMITS_BOTTOM, RADAR_SCALE);

		RECT radarWnd;
		GetClientRect(hWnd, &radarWnd);

		while (runRadar.load()) {

			RedrawWindow(hWnd, &radarWnd, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

			FillRect(hdcWindow, &radar, radarBrush);
			FrameRect(hdcWindow, &mapLimits, blackBrush);

			RenderPlayers(mapPlayers);

			RenderPos(userX, userY);
			RenderUser(userX, userY);

			Sleep(100);
		}

	}

	void RenderPos(int userX, int userY) {
		std::wstringstream xyStream;
		xyStream << L"X: " << userX << L" Y: " << userY;

		std::wstring upos = xyStream.str();

		SelectObject(hdcWindow, hFont);
		TextOut(hdcWindow, 1, 1, upos.c_str(), upos.length());
	}

	void RenderUser(int userX, int userY) {
		RECT user = GetRectRadar(userX, userY, userX + ENTITY_OFFSET, userY + ENTITY_OFFSET, RADAR_SCALE);
		FillRect(hdcWindow, &user, playerBrush);

		int heightOffset = USER_VISION_YOFFSET * RADAR_SCALE;
		int widthOffset = USER_VISION_XOFFSET * RADAR_SCALE;

		RECT userVision = GetRectRadar(user.left - widthOffset, user.top - heightOffset,
			user.right + widthOffset, user.bottom + heightOffset);

		FrameRect(hdcWindow, &userVision, blackBrush);
	}

	void RenderPlayers(const std::unordered_map<int, std::shared_ptr<Player>>& mapPlayers) {
		//	std::lock_guard<std::mutex> lock(mtxMapPlayers);
		for (const auto& pl : mapPlayers) {

			int posX = pl.second->posX;
			int posY = pl.second->posY;

			int posXOffset = posX + ENTITY_OFFSET;
			int posYOffset = posY + ENTITY_OFFSET;

			RECT player = GetRectRadar(posX, posY, posXOffset, posYOffset, RADAR_SCALE);

			switch (pl.second->bcr) {
				case 5: case 21:
					FillRect(hdcWindow, &player, neutralBrush);
					break;
				case 2:
					FillRect(hdcWindow, &player, ciudasBrush);
					break;
				case 3: case 23:
					FillRect(hdcWindow, &player, crimisBrush);
					break;
				case 1:
					FillRect(hdcWindow, &player, gmsBrush);
					break;
				default:
					FillRect(hdcWindow, &player, unknownBrush);
			}
		}
	}

	RECT GetRectRadar(int left, int top, int right, int bottom, float scale) {
		RECT rct{};
		rct.left = static_cast<int>(left * scale);
		rct.top = static_cast<int>(top * scale);
		rct.right = static_cast<int>(right * scale);
		rct.bottom = static_cast<int>(bottom * scale);
		return rct;
	}
}



#include "stdafx.h"
#include "RadarManager.h"


namespace RadarManager {
	HANDLE radarHandle;

	HWND hWnd;

	HDC hdcWindow;
	HDC hdcCompatible;

	HBRUSH radarBrush;
	HBRUSH neutralBrush;
	HBRUSH ciudasBrush;
	HBRUSH crimisBrush;
	HBRUSH gmsBrush;
	HBRUSH unknownBrush;
	HBRUSH blackBrush;
	HBRUSH playerBrush;

	std::atomic<bool> runRadar{ false };

	bool InitRadar(const std::string& windowTitle) {
		if (!SetupRadar(windowTitle)) return false;

		InitBrushes();
		radarHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RadarRenderThread, 0, 0, 0);
		runRadar.store(true);

		return true;
	}

	void FinalizeRadar() {
		runRadar.store(false);

		WaitForSingleObject(radarHandle, INFINITE);
		DisposeObjects();
		GetExitCodeThread(radarHandle, 0);
		CloseHandle(radarHandle);
		radarHandle = nullptr;
	}

	//void FinalizeRadar() {
	//	runRadar = false;
	//	Sleep(25);

	//	WaitForSingleObject(radarHandle, INFINITE);
	//	DisposeObjects();
	//	GetExitCodeThread(radarHandle, 0);
	//	CloseHandle(radarHandle);
	//	radarHandle = nullptr;
	//}

	bool SetupRadar(const std::string& windowTitle) {

		std::wstring wndTitle(windowTitle.begin(), windowTitle.end());
		hWnd = FindWindow(NULL, wndTitle.c_str());

		if (hWnd == NULL) {
			pm::writeLog("RadarManager - Failed to find window: " + windowTitle, pm::DlibLogType::DLIB);
			return false;
		}

		hdcWindow = GetDC(hWnd);
		if (hdcWindow == NULL) {
			pm::writeLog("RadarManager - Failed to get DC", pm::DlibLogType::DLIB);
			return false;
		}

		hdcCompatible = CreateCompatibleDC(hdcWindow);
		if (hdcCompatible == NULL) {
			pm::writeLog("RadarManager - Failed to create compatible HDC", pm::DlibLogType::DLIB);
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
		DeleteDC(hdcCompatible);
		ReleaseDC(hWnd, hdcWindow);
	}

	void RadarRenderThread() {
		while (runRadar.load()) {

			RECT radarWnd;
			GetClientRect(hWnd, &radarWnd);

			RedrawWindow(hWnd, &radarWnd, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

			RECT radar = { 100,100,100,100 };
			FillRect(hdcWindow, &radar, radarBrush);

			RECT mapLimits = { 10,10,90,92 };
			FrameRect(hdcWindow, &mapLimits, blackBrush);

			if (user_pos_x > 0 && user_pos_y > 0) {

				RECT user = { user_pos_x, user_pos_y, user_pos_x + 3, user_pos_y + 3 };
				FillRect(hdcWindow, &user, playerBrush);

				RECT userVision = { user.left - 9, user.top - 11, user.right + 9, user.bottom + 11 };
				FrameRect(hdcWindow, &userVision, blackBrush);
			}

			for (const auto& pl : players_in_map) {
				if (!pl.second) continue;

				RECT player = { pl.second->posX, pl.second->posY, pl.second->posX + 3, pl.second->posY + 3 };

				switch (pl.second->faction) {
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

			Sleep(50);
		}
	}

}



#include "stdafx.h"
#include "RadarManager.h"

//public
RadarManager::RadarManager(const std::string& wtitle) :
	wTitle(wtitle)
{
}

RadarManager::~RadarManager()
{
	running.store(false);
	radarThread.join();
	SendMessage(hWnd, WM_CLOSE, 0, 0);
	disposeObjects();
}

bool RadarManager::initRadar() {
	if (!setupRadar(wTitle)) return false;

	initBrushes();
	initFont();
	running.store(true);
	radarThread = std::thread(&RadarManager::render, this);

	return true;
}

void RadarManager::finalizeRadar() {
	running.store(false);
	SendMessage(hWnd, WM_CLOSE, 0, 0);
	disposeObjects();
}

void RadarManager::render() {

	RECT radar = getScaleRect(RADAR_RECT_LEFT, RADAR_RECT_TOP, RADAR_RECT_RIGHT, RADAR_RECT_BOTTOM, RADAR_SCALE);
	RECT mapLimits = getScaleRect(MAP_LIMITS_LEFT, MAP_LIMITS_TOP, MAP_LIMITS_RIGHT, MAP_LIMITS_BOTTOM, RADAR_SCALE);

	while (running.load()) {
		RECT radarWnd;
		GetClientRect(hWnd, &radarWnd);
		RedrawWindow(hWnd, &radarWnd, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

		FillRect(hdcWindow, &radar, radarBrush);
		FrameRect(hdcWindow, &mapLimits, blackBrush);

		renderUserpos();
		renderUser();
		renderPlayersCount(0);
		renderPlayersCount(1);
		renderPlayers();
		Sleep(80);
	}
}

//private
bool RadarManager::setupRadar(const std::string& wtitle) {
	std::wstring wndTitle(wtitle.begin(), wtitle.end());
	hWnd = FindWindow(NULL, wndTitle.c_str());

	if (hWnd == NULL) {
		dlg.logData("RadarManager - Failed to find window: " + wtitle, DlibLogger::LogType::DLIB);
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

void RadarManager::initBrushes() {
	radarBrush = CreateSolidBrush(RGB(0, 255, 0));
	neutralBrush = CreateSolidBrush(RGB(127, 127, 127));
	ciudasBrush = CreateSolidBrush(RGB(0, 162, 232));
	crimisBrush = CreateSolidBrush(RGB(255, 0, 0));
	gmsBrush = CreateSolidBrush(RGB(255, 255, 255));
	unknownBrush = CreateSolidBrush(RGB(255, 242, 0));
	blackBrush = CreateSolidBrush(RGB(0, 0, 0));
	playerBrush = CreateSolidBrush(RGB(0, 0, 255));
}

void RadarManager::initFont() {
	hFont = CreateFontW(FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");
}

void RadarManager::deleteBrushes() {

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

void RadarManager::deleteFont() {
	DeleteObject(hFont);
	hFont = nullptr;
}

void RadarManager::disposeObjects() {
	deleteBrushes();
	deleteFont();
	DeleteDC(hdcCompatible);
	ReleaseDC(hWnd, hdcWindow);
}

void RadarManager::renderUserpos() {
	std::wstringstream xyStream;
	xyStream << L"X: " << userX << L" Y: " << userY;

	std::wstring upos = xyStream.str();

	SelectObject(hdcWindow, hFont);
	TextOut(hdcWindow, 1, 155, upos.c_str(), upos.length());

}

void RadarManager::renderPlayersXY() {
	for (const auto& pl : mapPlayers) {

	}
}

void RadarManager::renderUser() {
	RECT user = getScaleRect(userX, userY, userX + ENTITY_OFFSET, userY + ENTITY_OFFSET, RADAR_SCALE);
	FillRect(hdcWindow, &user, playerBrush);

	int heightOffset = USER_VISION_YOFFSET * RADAR_SCALE;
	int widthOffset = USER_VISION_XOFFSET * RADAR_SCALE;

	RECT userVision = getScaleRect(user.left - widthOffset, user.top - heightOffset,
		user.right + widthOffset, user.bottom + heightOffset);

	FrameRect(hdcWindow, &userVision, blackBrush);
}

void RadarManager::renderPlayersCount(int t) {
	std::wstringstream xyStream;
	int y = 0;
	if (t == 0) {
		y = 170;
		xyStream << L"map players: " << mapPlayers.size();
	}
	else {
		y = 190;
		xyStream << L"rng players: " << rngPlayers.size();
	}

	std::wstring upos = xyStream.str();
	SelectObject(hdcWindow, hFont);
	TextOut(hdcWindow, 1, y, upos.c_str(), upos.length());
}

void RadarManager::renderPlayers() {	
	for (const auto& pl : mapPlayers) {

		int posXOffset = pl.second->posX + ENTITY_OFFSET;
		int posYOffset = pl.second->posY + ENTITY_OFFSET;

		RECT player = getScaleRect(pl.second->posX, pl.second->posY, posXOffset, posYOffset, RADAR_SCALE);

		switch (pl.second->bcr) {
			case 5: case 21:
				FillRect(hdcWindow, &player, neutralBrush);
				break;
			case 2: case 22: case 9: case 15:
				FillRect(hdcWindow, &player, ciudasBrush);
				break;
			case 3: case 23: case 10:
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

RECT RadarManager::getScaleRect(int left, int top, int right, int bottom, float scale) {
	RECT rct{};
	rct.left = static_cast<int>(left * scale);
	rct.top = static_cast<int>(top * scale);
	rct.right = static_cast<int>(right * scale);
	rct.bottom = static_cast<int>(bottom * scale);
	return rct;
}
#pragma once
#include <string>
#include <atomic>
#include <Windows.h>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

class RadarManager {
public:
	RadarManager(const std::string& wtitle);
	~RadarManager();

	bool initRadar();
	void finalizeRadar();
	void render();

	template<typename... Args>
	std::wstring getWstring(Args&&... args);

private:


	template<typename T>
	void formatArgs(std::wostringstream& wss, const std::string& varName, const T& value);

	template<typename T, typename... Args>
	void formatArgs(std::wostringstream& wss, const std::string& varName, const T& value, Args&&... args);


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
	HBRUSH playerDeadBrush;

	const int RADAR_RECT_LEFT = 0;
	const int RADAR_RECT_TOP = 0;
	const int RADAR_RECT_RIGHT = 100;
	const int RADAR_RECT_BOTTOM = 100;

	const int MAP_LIMITS_LEFT = 10;
	const int MAP_LIMITS_TOP = 10;
	const int MAP_LIMITS_RIGHT = 90;
	const int MAP_LIMITS_BOTTOM = 92;

	const int ENTITY_OFFSET = 3;
	const int USER_VISION_YOFFSET = 9;
	const int USER_VISION_XOFFSET = 11;

	const float RADAR_SCALE = 1.5f;
	const int FONT_SIZE = 8;
	int userPosTextY = 0;

	std::string wTitle;
	std::atomic<bool> running;
	std::thread radarThread;
	
	bool setupRadar(const std::string& wtitle);
	void initBrushes();
	void initFont();
	void deleteBrushes();
	void deleteFont();
	void disposeObjects();

	void renderUser();
	void renderPlayers();
	void renderTexts();

	HBRUSH getBrush(bool isDead, int bcr);
	RECT getScaleRect(int left, int top, int right, int bottom, float scale = 1.0f);

};

#include "RadarManager.tpp";

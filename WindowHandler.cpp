#include "stdafx.h"
#include "WindowHandler.h"

HWND hwnd_radar = nullptr;
HWND hwnd_dylib = nullptr;

HBRUSH neutralBrush = CreateSolidBrush(RGB(127, 127, 127));
HBRUSH ciudasBrush = CreateSolidBrush(RGB(0, 162, 232));
HBRUSH crimisBrush = CreateSolidBrush(RGB(255, 0, 0));
HBRUSH gmsBrush = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH unknownBrush = CreateSolidBrush(RGB(255, 242, 0));
HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH playerBrush = CreateSolidBrush(RGB(0, 0, 255));

VOID CreateRadarWindow() {

	const wchar_t CLASS_NAME[] = L"Radar Window Class";

	WNDCLASS wc = {};
	wc.lpfnWndProc = RadarWindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"Failed to register window class", L"Error", MB_ICONERROR);
		return;
	}

	hwnd_radar = CreateWindowEx(
		WS_EX_LAYERED,
		CLASS_NAME,
		L"Radar Window",
		WS_POPUP, 
		CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, 
		NULL, NULL, GetModuleHandle(NULL), NULL
	);

	if (hwnd_radar == NULL) {
		MessageBox(NULL, L"Failed to create window", L"Error", MB_ICONERROR);
		return;
	}

	SetLayeredWindowAttributes(hwnd_radar, RGB(0, 0, 0), 200, LWA_COLORKEY | LWA_ALPHA);

	ShowWindow(hwnd_radar, SW_SHOW);
	UpdateWindow(hwnd_radar);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK RadarWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static POINTS ptInitPos; // Posición inicial cuando se comienza a arrastrar
	static bool isDragging = false; // Indica si se está arrastrando la ventana

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		// Iniciar el arrastre
		ptInitPos = MAKEPOINTS(lParam);
		SetCapture(hwnd);
		isDragging = true;
		return 0;

	case WM_MOUSEMOVE:
		if (isDragging) {
			POINTS ptCurrent = MAKEPOINTS(lParam); // Posición actual del cursor

			RECT rect;
			GetWindowRect(hwnd, &rect);

			int dx = ptCurrent.x - ptInitPos.x;
			int dy = ptCurrent.y - ptInitPos.y;

			SetWindowPos(hwnd, 0,
				rect.left + dx,
				rect.top + dy,
				0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		return 0;

	case WM_LBUTTONUP:
		// Detener el arrastre
		isDragging = false;
		ReleaseCapture();
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
		FillRect(hdc, &rect, greenBrush);
		DeleteObject(greenBrush);

		EndPaint(hwnd, &ps);
	}
	return 0;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void RenderRadar(const int& user_pos_x, const int& user_pos_y, const std::map<int, Player*>& players_in_map) {
	HDC hdc = GetDC(hwnd_radar);

	RECT rectVentana;
	GetClientRect(hwnd_radar, &rectVentana);

	RedrawWindow(hwnd_radar, &rectVentana, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

	RECT map_limit_rect = { 10, 10, 90, 92 };
	FrameRect(hdc, &map_limit_rect, blackBrush);

	if (user_pos_x > 0 && user_pos_y > 0) {

		std::wstringstream ss;
		ss << L"X: " << user_pos_x << L" Y: " << user_pos_y;

		std::wstring upos = ss.str();

		int nuevoTamanoFuente = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72); // Tamaño de fuente aproximado 4

		HFONT hFont = CreateFontW(nuevoTamanoFuente, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");

		SelectObject(hdc, hFont);
		TextOut(hdc, 1, 1, upos.c_str(), upos.length());

		RECT user_rect = { user_pos_x, user_pos_y, user_pos_x + 3, user_pos_y + 3 };
		FillRect(hdc, &user_rect, playerBrush);

		RECT user_vision_rect = { user_rect.left - 9, user_rect.top - 11, user_rect.right + 9, user_rect.bottom + 11 };
		FrameRect(hdc, &user_vision_rect, blackBrush);
	}

	if (players_in_map.size() > 0) {
		for (auto pl : players_in_map) {

			RECT player_rect = { pl.second->posX, pl.second->posY, pl.second->posX + 3, pl.second->posY + 3 };

			switch (pl.second->faction) {
			case 5:
			case 21:
				FillRect(hdc, &player_rect, neutralBrush);
				break;
			case 2:
				FillRect(hdc, &player_rect, ciudasBrush);
				break;
			case 3:
			case 23:
				FillRect(hdc, &player_rect, crimisBrush);
				break;
			case 1:
				FillRect(hdc, &player_rect, gmsBrush);
				break;
			default:
				FillRect(hdc, &player_rect, unknownBrush);
			}
		}
	}

	ReleaseDC(hwnd_radar, hdc);
}

void SetRadarTopMost(bool top_most) {

	HWND hwnd_top_most;

	if (top_most) {
		hwnd_top_most = HWND_TOPMOST;
	}
	else {
		hwnd_top_most = HWND_NOTOPMOST;
	}

	SetWindowPos(hwnd_radar, hwnd_top_most, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

VOID CreateDyLibWindow() {

	const wchar_t CLASS_NAME[] = L"DyLib Window Class";

	WNDCLASS wc = {};
	wc.lpfnWndProc = DyLibWindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"Failed to register window class", L"Error", MB_ICONERROR);
		return;
	}

	hwnd_dylib = CreateWindowEx(
		WS_EX_LAYERED,
		CLASS_NAME,
		L"DyLib Window",
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
		NULL, NULL, GetModuleHandle(NULL), NULL
	);

	if (hwnd_radar == NULL) {
		MessageBox(NULL, L"Failed to create window", L"Error", MB_ICONERROR);
		return;
	}

	SetLayeredWindowAttributes(hwnd_radar, RGB(0, 0, 0), 200, LWA_COLORKEY | LWA_ALPHA);

	ShowWindow(hwnd_radar, SW_SHOW);
	UpdateWindow(hwnd_radar);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK DyLibWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static POINTS ptInitPos; // Posición inicial cuando se comienza a arrastrar
	static bool isDragging = false; // Indica si se está arrastrando la ventana

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		// Iniciar el arrastre
		ptInitPos = MAKEPOINTS(lParam);
		SetCapture(hwnd);
		isDragging = true;
		return 0;

	case WM_MOUSEMOVE:
		if (isDragging) {
			POINTS ptCurrent = MAKEPOINTS(lParam); // Posición actual del cursor

			RECT rect;
			GetWindowRect(hwnd, &rect);

			int dx = ptCurrent.x - ptInitPos.x;
			int dy = ptCurrent.y - ptInitPos.y;

			SetWindowPos(hwnd, 0,
				rect.left + dx,
				rect.top + dy,
				0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		return 0;

	case WM_LBUTTONUP:
		// Detener el arrastre
		isDragging = false;
		ReleaseCapture();
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		RECT rect;
		GetClientRect(hwnd, &rect);
		HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
		FillRect(hdc, &rect, greenBrush);
		DeleteObject(greenBrush);

		EndPaint(hwnd, &ps);
	}
	return 0;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#pragma once

#include <Windows.h>

LRESULT CALLBACK RadarWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DyLibWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HWND hwnd_radar;
extern HWND hwnd_dylib;

VOID CreateRadarWindow();
VOID CreateDyLibWindow();

void RenderRadar(const int& user_pos_x, const int& user_pos_y, const std::map<int, Player*>& players_in_map);
void SetRadarTopMost(bool top_most);
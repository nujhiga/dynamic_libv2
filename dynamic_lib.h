#include <Windows.h>
#include <comutil.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <iostream>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <iterator>
#include <memory>
#include <tuple>
#include <thread>
#include <algorithm>
#include <fstream>
#include <limits>

#include "stdafx.h"
#include "detours.h"
#include "Player.h"
#include "Npch.h"
#include "PlayerRange.h"

void InitializeHooks(void);

VOID WINAPI MyRecvData(BSTR recvData);		
VOID WINAPI MySendData(BSTR* sendData);
int WINAPI MyLoop();						

BOOL StartsWith(BSTR sValue, const WCHAR* pszSubValue);


std::string hexToString(const std::string& hex);


std::tuple<int, int> ReadXY_stoi(BSTR bstr_xy_packet);
std::tuple<string, string> ReadSpellsInfo(BSTR spell_packet);

size_t splitedPacketLen(BSTR packet);

VOID SetManualTarget(const int& posX, const int& posY);
std::tuple<int, int> GetClosestTargetPos(const int& posX, const int& posY);
std::tuple<int, int> GetManualTargetPos();
std::tuple<int, int> GetUserTargetPos();
//VOID SearchAndSetTarget(int pos_x, int y);
VOID MapChanged();
//VOID AutoRegenHpMan();

VOID InterceptSHS(BSTR packet);

VOID Intercept_npch_CR_MP(BSTR packet, 
	const bool& mp_packet, 
	const int& id_index, 
	const int& pos_x_index, 
	const int& pos_y_index);

VOID Intercept_player_MP(BSTR packet);
VOID SendToClient(const std::string& message);
VOID SendToClient(BSTR message);
VOID SendToServer(const std::string& message);
//VOID CastSpell(const int& stype);
VOID AddNpch(const int& nid, const int& posX, const int& posY);
VOID AddPlayer(const std::vector<std::string>& splitVector);

VOID CleanupMapPlayers();
VOID CleanupRangePlayers();

VOID RemoveMapPlayer(const int& pid);
VOID RemoveRangePlayer(const int& pid);
VOID RemoveRangePlayer(std::unordered_map<int, PlayerRange*>::iterator it);

VOID CleanupMapNpcs();
VOID CleanupRangeNpcs();

VOID CheckNewTargets();

VOID RemoveMapNpc(const int& nid);
VOID RemoveRangeNpc(const int& nid);

BOOL IsInRange(const int& posX, const int& posY);
BOOL IsWhiteSpell();

VOID CasterThread();


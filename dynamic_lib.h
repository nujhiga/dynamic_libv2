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
#include "RadarManager.h"
#include "PacketManager.h"
#include "mutex"

//ToDo move to globals file
extern std::unordered_map<int, Player*> players_in_map;
//extern std::mutex mtx_players_in_map;

extern int user_pos_x;
//extern std::mutex mtx_user_pos_x;

extern int user_pos_y;
//extern std::mutex mtx_user_pos_y;
//ToDo move to globals file

void InitializeHooks(void);

VOID WINAPI MyRecvData(BSTR recvData);
VOID WINAPI MySendData(BSTR* sendData);
int WINAPI MyLoop();

BOOL StartsWith(BSTR sValue, const WCHAR* pszSubValue);
BOOL StartsWithAndNot(BSTR sValue, const WCHAR* pszSubValue, const WCHAR* npszSubValue);

std::string hexToString(const std::string& hex);

void HideCheat();
void MapChanged();
//VOID AutoRegenHpMan();

void Intercept_SHS(const std::string& packet);
int GetSpellPosition(const std::string& sname);
bool IsSelectedLH(const std::string& sname);

void Intercept_CR(const std::string& packet);
void AddNpch(int nid, int posX, int posY);
void RemoveMapNpc(int nid);

void Intercept_CC(BSTR& dataRecv, const std::string& packet);
void AddPlayer(int pid, const std::vector<std::string>& pinfo);
void RemoveMapPlayer(int pid);

void Intercept_MP(const std::string& packet);
void UpdateNpcPos(int nid, int posX, int posY, bool inRange);
void UpdatePlayerPos(int pid, int posX, int posY, bool inRange);

void AddPlayerRange(int pid, int posX, int posY);
void RemoveRangePlayer(std::unordered_map<int, PlayerRange*>::iterator it);
void RemoveRangePlayer(int pid);

void RemoveRangeNpc(int nid);

void CleanupMapPlayers();
void CleanupRangePlayers();

void CleanupMapNpcs();
void CleanupRangeNpcs();

void Intercept_PU(const std::string& packet);

void Intercept_LH(const std::string& packet);
void SetManualTarget(int posX, int posY);

void Intercept_V3(BSTR& dataRecv, const string& packet);

bool IsInRange(int posX, int posY);

std::tuple<int, int> GetClosestTargetPos(int posX, int posY);
std::tuple<int, int> GetManualTargetPos();
std::tuple<int, int> GetUserTargetPos();

void CheckNewTargets();

VOID SendToClient(const std::string& message);
VOID SendToClient(BSTR message);
VOID SendToServer(const std::string& message);


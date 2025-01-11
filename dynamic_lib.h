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
#include <unordered_set>
#include <iterator>
#include <memory>
#include <tuple>
#include <thread>
#include <algorithm>
#include <fstream>
#include <limits>

#include "stdafx.h"
#include "detours.h"
#include "RadarManager.h"
#include "PacketManager.h"
#include "DlibLogger.h"
#include "mutex"

#include "Player.h"
#include "Npc.h"

extern std::unordered_map<int, std::shared_ptr<Player>> mapPlayers;

extern int userX;
extern int userY;

extern DlibLogger dlg;

void InitializeHooks(void);

VOID WINAPI MyRecvData(BSTR recvData);
VOID WINAPI MySendData(BSTR* sendData);
int WINAPI MyLoop();

BOOL StartsWith(BSTR sValue, const WCHAR* pszSubValue);
BOOL StartsWith(BSTR sValue, const std::vector<const WCHAR*>& pszSubValue);
BOOL StartsWithAndNot(BSTR sValue, const WCHAR* pszSubValue, const WCHAR* npszSubValue);

std::string hexToString(const std::string& hex);

void HideCheat(bool finalizeRadar);
void Intercept_CM(const std::string& packet);
//VOID AutoRegenHpMan();

void Intercept_SHS(const std::string& packet);
std::string GetSelectedLHName();
int GetSpellPosition(const std::string& sname);
bool IsSelectedLH(const std::string& sname);

void Intercept_CR(const std::string& packet);
void Intercept_QQ(const std::string& packet);
void AddNPC(const std::vector<std::string>& pinfo);
void RemoveMapNpc(int nid);

void Intercept_CC(BSTR& dataRecv, const std::string& packet);
void Intercept_CP(const std::string& packet);
void AddPlayer(int pid, bool detected, const std::vector<std::string>& pinfo);
bool RemoveMapPlayer(int pid);
void RemoveMapPlayer(const std::unordered_map<int, std::shared_ptr<Player>>::iterator& it);

void Intercept_MP(const std::string& packet);

template <typename MapType>
void CleanupMap(MapType& map);

void Intercept_PU(const std::string& packet);

void Intercept_LC(const std::string& packet);
void SetManualTarget(int posX, int posY);

void Intercept_V3(BSTR& dataRecv, const std::string& packet);

void Intercept_M1234(const std::string& packet);

void Intercept_BP(const std::string& packet);

void Intercept_WLC(BSTR* dataSend, const std::string& packet);

void Intercept_CastON(const std::string& packet);

int GetPlayerID(const std::string& pname);

void Intercept_LH(const std::string& packet);

bool IsInRange(int posX, int posY);

bool IsUserPosOutbounds();

void SetUserpos(int posX, int posY);

std::tuple<int, int> GetClosestTargetPos(int posX, int posY);
std::tuple<int, int> GetManualTargetPos();
std::tuple<int, int> GetUserTargetPos();
std::tuple<int, int> GetLastTargetPos();

void CheckPlayerTargets();
void CheckNpcTargets();

void PlayLocalWav(int wav);

VOID SendToClient(const std::string& message);
VOID SendToServer(const std::string& message);


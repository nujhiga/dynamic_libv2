#include "stdafx.h"

namespace pm = PacketManager;
namespace rm = RadarManager;

#pragma comment(lib, "comsuppw.lib")

DWORD oldTTeclas = GetTickCount();
//DWORD ttAutoPot = GetTickCount();

bool isRadarRunning = false;
bool hideCheating = false;
bool writeLogs = true;

int cast_mode = 0;
//int cast_target = 0;

int user_pos_x = 0;
int user_pos_y = 0;

int user_id = 0;
int user_faction = 0;
bool user_paralized = false;
bool user_meditando = false;

std::atomic<bool> lockM1234{ false };

const std::string user_name = "growland";
const std::string del_spell_words = ";1 ";

std::unordered_map<std::string, int> userSpells;

const std::string Remo = "Remover paralisis";
const std::string Apoca = "Apocalipsis";
const std::string Desca = "Descarga eléctrica";
const std::string Inmo = "Inmovilizar";
const std::string Tormi = "Tormenta de fuego";
const std::string Misil = "Misil magico";
const std::string Felect = "Flecha eléctrica";
const std::string Dardo = "Dardo magico";
const std::string Invi = "Invisibilidad";
const std::string Cele = "Celeridad";
const std::string Fue = "Fuerza";
const std::string EleA = "Invocar elemental de agua";

//const std::string CMD_MEDITAR = "meditar";

bool uk_flag = false;
bool lh_flag = false;
bool auto_cast_flag = false;

int selected_player_id = 0;
int selected_npch_id = 0;
int selected_lh = 0;

const int WAV_SCREEN_CAPTURE = 555;
const int WAV_PRC_PPL_PPP = 324;

std::unordered_map<int, Player*> players_in_map;
std::unordered_map<int, PlayerRange*> players_in_range;

std::unordered_map<int, Npch*> npcs_in_map;
std::unordered_map<int, std::pair<int, int>> npcs_in_range;

std::vector<std::string> packets;

const std::string LOG_FILEPATH = "C:\\Users\\joaco\\Documents\\dlib_log.txt";
DlibLogger dlg(LOG_FILEPATH);

typedef VOID(WINAPI* PRecvData)(BSTR data);
PRecvData PFunctionRecv = (PRecvData)0x0084F0E0; //Pointer to where the original HandleData() starts

typedef VOID(WINAPI* PSendData)(BSTR* data);
PSendData PFunctionSend = (PSendData)0x008B1850; //Pointer to where the original SendData() starts

typedef int(WINAPI* PLoop)();
HMODULE dllModule = LoadLibraryA("MSVBVM60.DLL");
PLoop PFunctionLoop = (PLoop)GetProcAddress(dllModule, "rtcDoEvents");

void InitializeHooks()
{
	DetourTransactionBegin();

	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)PFunctionRecv, &MyRecvData);      // Hook DataHandle() to MyRecvData()
	DetourAttach(&(PVOID&)PFunctionSend, &MySendData);      // Hook SendData() to MySendData()
	DetourAttach(&(PVOID&)PFunctionLoop, &MyLoop);          // Hook DoEvents AKA Loop() to MyLoop()

	DetourTransactionCommit();
}

VOID WINAPI MyRecvData(BSTR dataRecv)
{
	__asm PUSHAD;
	__asm PUSHFD;

	if (StartsWith(dataRecv, L"PAIN")) {
		PlayLocalWav(WAV_SCREEN_CAPTURE);
		HideCheat(true);
		PlayLocalWav(WAV_PRC_PPL_PPP);
	}

	if (StartsWith(dataRecv, { L"PRC", L"PPP", L"PPL" })) {
		PlayLocalWav(WAV_PRC_PPL_PPP);
	}

	if (StartsWith(dataRecv, L"LH")) {
		uk_flag = false;
		lh_flag = false;
		auto_cast_flag = false;
	}

	if (StartsWith(dataRecv, L"MEDOK")) {
		user_meditando = !user_meditando;
	}

	if (StartsWith(dataRecv, L"V3")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_V3(dataRecv, packet);
	}

	//ToDo :: if (StartsWith(dataRecv, L"EST")) 
	//ToDo :: if (StartsWith(dataRecv, L"CP")) to verify is player has died

	if (StartsWith(dataRecv, L"BP")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_BP(packet);
	}

	if (StartsWith(dataRecv, L"CM")) {
		MapChanged();
	}

	if (StartsWith(dataRecv, L"CC")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CC(dataRecv, packet);
	}

	if (StartsWithAndNot(dataRecv, L"CR", L"CRA")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CR(packet);
	}

	if (StartsWith(dataRecv, L"MP")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_MP(packet);
	}

	if (StartsWith(dataRecv, L"P9")) {
		user_paralized = true;
	}

	if (StartsWith(dataRecv, L"P8")) {
		user_paralized = false;
	}

	if (StartsWith(dataRecv, L"PU")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_PU(packet);
	}

	if (StartsWith(dataRecv, L"SHS")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 3);
		Intercept_SHS(packet);
	}

	if (writeLogs) dlg.logData(pm::ConvertBSTRToString(dataRecv), DlibLogger::LogType::RECV);

	PFunctionRecv(dataRecv);

	__asm POPFD;
	__asm POPAD;
}

VOID WINAPI MySendData(BSTR* dataSend)
{
	__asm PUSHAD;
	__asm PUSHFD;

	if (StartsWith(*dataSend, L"WLC")) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 3);
		Intercept_WLC(dataSend, packet);
	}

	if (StartsWith(*dataSend, L"UK1")) {
		uk_flag = true;
	}

	if (StartsWith(*dataSend, L"LH")) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 2);
		selected_lh = stoi(packet);
		lh_flag = true;
	}

	if (StartsWith(*dataSend, L"LC")) {
		if (cast_mode == 1) {
			std::string packet = pm::ConvertBSTRPacket(*dataSend, 2);
			Intercept_LC(packet);
		}
	}

	if (StartsWith(*dataSend, { L"M1",L"M2", L"M3", L"M4" })) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 1);
		Intercept_M1234(packet);
	}

	if (writeLogs) dlg.logData(pm::ConvertBSTRToString(*dataSend), DlibLogger::LogType::SEND);

	PFunctionSend(dataSend);

	__asm POPFD;
	__asm POPAD;
}

int WINAPI MyLoop()
{
	__asm PUSHAD;
	__asm PUSHFD;

	DWORD currentTime = GetTickCount();

	if (currentTime - oldTTeclas > 100) {

		if ((GetKeyState(VK_MULTIPLY) & 0x100) != 0) {

			if (!isRadarRunning) {
				isRadarRunning = rm::InitRadar("Untitled - Notepad");
			}
			else {
				rm::FinalizeRadar();
				isRadarRunning = false;
			}

			Sleep(250);
		}

		if ((GetKeyState(VK_END) & 0x100) != 0) {
			HideCheat(true);
		}

		if ((GetKeyState(VK_XBUTTON2) & 0x100) != 0) {
			auto_cast_flag = true;
		}

		if ((GetKeyState(VK_XBUTTON1) & 0x100) != 0) {
			Sleep(30);
			SendToServer(del_spell_words);
			Sleep(10);
		}

		if ((GetKeyState(VK_DELETE) & 0x100) != 0) {

		}

		//if ((GetKeyState(VK_INSERT) & 0x100) != 0) {
		//}

		oldTTeclas = GetTickCount();

		if (packets.size() != 0) {

			for (size_t i = 0; i < packets.size(); ++i)
				SendToServer(packets[i]);

			packets.clear();
		}
	}

	PFunctionLoop();

	__asm POPFD;
	__asm POPAD;
}

void HideCheat(bool finalizeRadar) {
	hideCheating = true;

	if (isRadarRunning) {
		rm::FinalizeRadar();
		isRadarRunning = false;
	}

	Sleep(25);

	for (const auto& player : players_in_map) {
		if (!player.second) continue;

		auto& pl = player.second;

		if (pl->inviDetected) {

			pl->name = pl->orgName;
			pl->isInvisible = true;
			pl->inviDetected = false;

			SendToClient(pm::build_BP(pl->id));
			SendToClient(pm::build_CC(player));
		}
	}
	Sleep(25);
}

void MapChanged() {

	CleanupMap(players_in_map);
	CleanupMap(players_in_range);

	CleanupMap(npcs_in_map);
	CleanupRangeNpcs();

	selected_npch_id = 0;
	selected_player_id = 0;
}

void Intercept_SHS(const std::string& packet) {
	auto sinfo = pm::read_SHS(packet);
	userSpells.emplace(sinfo);
}

int GetSpellPosition(const std::string& sname) {
	auto& its = userSpells.find(sname);
	if (its != userSpells.end())
		return its->second;

	return -1;
}

bool IsSelectedLH(const std::string& sname) {
	return GetSpellPosition(sname) == selected_lh;
}

void Intercept_CR(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	if (stoi(pinfo[1]) > 0) return;

	int nid = stoi(pinfo[3]);
	int posX = stoi(pinfo[4]);
	int posY = stoi(pinfo[5]);

	AddNpch(nid, posX, posY);
}

void AddNpch(int nid, int posX, int posY) {

	auto itn = npcs_in_map.find(nid);
	if (itn != npcs_in_map.end()) return;

	Npch* npc = new Npch();
	npc->id = nid;
	npc->posX = posX;
	npc->posY = posY;

	npcs_in_map.emplace(nid, npc);

	dlg.logDebug("AddNpch: npcs_in_map.emplace: ", "npcs_in_map.size()",
		npcs_in_map.size(), "nid", nid);

	if (IsInRange(posX, posY)) {
		npcs_in_range.emplace(nid, std::make_pair(posX, posY));
		dlg.logDebug("AddNpch: npcs_in_range.emplace: ", "npcs_in_range.size()",
			npcs_in_range.size(), "nid", nid);
	}
}

//void AddNpch(int nid, int posX, int posY) {
//
//	auto itn = npcs_in_map.find(nid);
//
//	if (itn == npcs_in_map.end()) {
//
//		Npch* npc = new Npch();
//		npc->id = nid;
//		npc->posX = posX;
//		npc->posY = posY;
//
//		npcs_in_map.emplace(nid, npc);
//
//		if (IsInRange(posX, posY)) {
//			npcs_in_range.emplace(nid, std::make_pair(posX, posY));
//		}
//	}
//}

void RemoveMapNpc(int nid) {
	auto itn = npcs_in_map.find(nid);

	if (itn != npcs_in_map.end()) {
		if (itn->second) {
			delete itn->second;
			itn->second = nullptr;
		}
		npcs_in_map.erase(itn);
		//RemoveRangeNpc(nid);
	}
}

void Intercept_CC(BSTR& dataRecv, const std::string& packet) {

	auto pinfo = pm::split(packet, ',');
	int pid = stoi(pinfo[3]);

	if (pinfo[11] == user_name) {

		user_id = pid;
		SetUserpos(stoi(pinfo[4]), stoi(pinfo[5]));
		user_faction = stoi(pinfo[12]);
	}
	else {

		if (!hideCheating && pinfo[13] == "1")
		{
			pinfo[11] += " [I]";
			pinfo[13] = "0";
			SysFreeString(dataRecv);
			dataRecv = pm::ConvertStringToBSTR(pm::build_CC(pinfo));
		}

		AddPlayer(pid, pinfo);
	}
}

void AddPlayer(int pid, const std::vector<std::string>& pinfo) {

	auto it = players_in_map.find(pid);
	bool inviDetected = !hideCheating && stoi(pinfo[13]) == 1;

	if (it == players_in_map.end()) {
		players_in_map.emplace(pid, new Player{ pinfo });
		dlg.logDebug("AddPlayer: players_in_map.emplace: ", "players_in_map.size()",
			players_in_map.size(), "pid", pid);
	}
	else {
		it->second->inviDetected = inviDetected;
	}
}

bool RemoveMapPlayer(int pid) {

	auto itp = players_in_map.find(pid);

	if (itp != players_in_map.end()) {

		if (itp->second) {
			delete itp->second;
			itp->second = nullptr;
		}

		players_in_map.erase(itp);
		dlg.logDebug("RemoveMapPlayer: players_in_map.erase: ", "players_in_map.size()",
			players_in_map.size(), "pid", pid);
		//RemoveRangePlayer(pid);

		return true;
	}

	return false;
}

void Intercept_MP(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	int id = stoi(pinfo[0]);
	int posX = stoi(pinfo[1]);
	int posY = stoi(pinfo[2]);
	bool inRange = IsInRange(posX, posY);

	//if (pinfo[3] == "") {
	//	dlg.logDebug("Intercept_MP: pinfo.size() == 3: ", "inRange", inRange, "id", id);
	if (!UpdatePlayerPos(id, posX, posY, inRange))
		UpdateNpcPos(id, posX, posY, inRange);
	//}
	//else {
	//	dlg.logDebug("Intercept_MP: pinfo.size() != 3: ", "inRange", inRange, "id", id);
	//}

	//if (pinfo[3] == "") {
	//	dlg.logDebug("Intercept_MP: pinfo.size() == 3: ", "inRange", inRange, "id", id);
	//	UpdateNpcPos(id, posX, posY, inRange);
	//}
	//else {
	//	dlg.logDebug("Intercept_MP: pinfo.size() != 3: ", "inRange", inRange, "id", id);
	//	UpdatePlayerPos(id, posX, posY, inRange);
	//}
}

void UpdateNpcPos(int nid, int posX, int posY, bool inRange) {

	auto itm = npcs_in_map.find(nid);
	auto itr = npcs_in_range.find(nid);

	if (itm != npcs_in_map.end() && itm->second) {
		itm->second->posX = posX;
		itm->second->posY = posY;
		dlg.logDebug("UpdateNpcPos: npcs_in_map.end()->posupd: ", "nid", nid);
	}

	if (itr != npcs_in_range.end()) {
		if (inRange) {
			itr->second = std::make_pair(posX, posY);
			dlg.logDebug("UpdateNpcPos: itr != npcs_in_range.end()->posupd: ", "inRange", inRange, "nid", nid);
		}
		else {
			npcs_in_range.erase(itr);
			dlg.logDebug("UpdateNpcPos: itr != npcs_in_range.end()->.erase: ",
				"npcs_in_range.size()", npcs_in_range.size(), "inRange", inRange, "nid", nid);
		}
	}
	else if (inRange) {
		npcs_in_range.emplace(nid, std::make_pair(posX, posY));
		dlg.logDebug("UpdateNpcPos: itr == npcs_in_range.end()->.emplace: ",
			"npcs_in_range.size()", npcs_in_range.size(), "inRange", inRange, "nid", nid);
	}
}

bool UpdatePlayerPos(int pid, int posX, int posY, bool inRange) {

	auto itp = players_in_map.find(pid);
	auto itr = players_in_range.find(pid);
	bool updated = false;

	if (itp != players_in_map.end() && itp->second) {
		itp->second->posX = posX;
		itp->second->posY = posY;
		updated = true;
		dlg.logDebug("UpdatePlayerPos: players_in_map.end()->posupd: ", "pid", pid);
	}

	if (itr != players_in_range.end()) {
		if (inRange) {
			itr->second->posX = posX;
			itr->second->posY = posY;
			dlg.logDebug("UpdatePlayerPos: itr != players_in_range.end()->posupd: ",
				"players_in_range.size()", players_in_range.size(), "inRange", inRange, "pid", pid);
		}
		else {
			RemoveRangePlayer(itr);
		}
	}
	else if (inRange) {
		AddPlayerRange(pid, posX, posY);
	}

	return updated;
}

void AddPlayerRange(int pid, int posX, int posY) {
	PlayerRange* pr = new PlayerRange();

	pr->ID = pid;
	pr->posX = posX;
	pr->posY = posY;

	players_in_range.emplace(pid, pr);
}

void RemoveRangePlayer(std::unordered_map<int, PlayerRange*>::iterator& it) {

	if (it->second) {
		delete it->second;
		it->second = nullptr;
	}

	players_in_range.erase(it);
	dlg.logDebug("RemoveRangePlayer: players_in_range.erase by it: ", "players_in_range.size()", players_in_range.size());
}

void RemoveRangePlayer(int pid) {
	auto itr = players_in_range.find(pid);

	if (itr != players_in_range.end()) {

		if (itr->second) {
			delete itr->second;
			itr->second = nullptr;
		}
		players_in_range.erase(itr);
	}
	dlg.logDebug("RemoveRangePlayer: players_in_range.erase> ",
		"players_in_range.size()", players_in_range.size(), "pid", pid);

}

void RemoveRangeNpc(int nid) {
	npcs_in_range.erase(nid);
}

template <typename MapType>
void CleanupMap(MapType& map) {

	for (auto& element : map) {
		if (!element.second) continue;
		delete element.second;
		element.second = nullptr;
	}

	map.clear();
}

void CleanupRangeNpcs() {
	npcs_in_range.clear();
}

void Intercept_PU(const std::string& packet) {
	auto xy = pm::read_PU(packet);
	int x = std::get<0>(xy);
	int y = std::get<1>(xy);
	SetUserpos(x, y);
}

void Intercept_LC(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	int posX = stoi(pinfo[0]);
	int posY = stoi(pinfo[1]);

	SetManualTarget(posX, posY);
}

void SetManualTarget(int posX, int posY) {

	for (const auto& pr : players_in_range) {
		if (!pr.second) continue;

		if (pr.second->posX == posX && pr.second->posY == posY) {
			selected_player_id = pr.first;
			return;
		}
	}

	for (const auto& nr : npcs_in_range) {
		if (nr.second.first == posX && nr.second.second == posY) {
			selected_npch_id = nr.first;
			break;
		}
	}
}

void Intercept_V3(BSTR& dataRecv, const std::string& packet) {
	if (hideCheating) return;

	std::string dcpt_packet = pm::decrypt_packet(packet);
	auto pinfo = pm::split(dcpt_packet, ',');

	int pid = stoi(pinfo[1]);

	if (pid == user_id) return;

	auto itp = players_in_map.find(pid);

	if (itp != players_in_map.end() && itp->second) {
		auto& pl = itp->second;
		bool invi = pinfo[4] == "1";
		pl->isInvisible = invi;

		if (invi) {

			pl->name += " [I]";
			pl->isInvisible = false;
			pl->inviDetected = true;

			SendToClient(pm::build_BP(pid));
			SendToClient(pm::build_CC(*pl));

			pinfo[4] = "0";
			SysFreeString(dataRecv);
			dataRecv = pm::ConvertStringToBSTR(pm::build_V3(pinfo));
		}
		else {
			pl->name = pl->orgName;
			pl->inviDetected = false;

			SendToClient(pm::build_BP(pid));
			SendToClient(pm::build_CC(*pl));
		}
	}
}

void Intercept_M1234(const std::string& packet) {

	if (lockM1234.load()) return;
	if (user_paralized || user_meditando) return;

	int dir = stoi(packet);
	switch (dir)
	{
		case 1:
			if (user_pos_y - 1 >= 11)
				user_pos_y--;
			break;
		case 2:
			if (user_pos_x + 1 <= 88)
				user_pos_x++;
			break;
		case 3:
			if (user_pos_y + 1 <= 90)
				user_pos_y++;
			break;
		case 4:
			if (user_pos_x - 1 >= 13)
				user_pos_x--;
			break;
		default:
			break;
	}

	CheckPlayerTargets();
	CheckNpcTargets();
}

void Intercept_BP(const std::string& packet) {
	int eid = stoi(packet);
	if (eid == user_id) return;
	if (RemoveMapPlayer(eid)) return;
	RemoveMapNpc(eid);
}

void Intercept_WLC(BSTR* dataSend, const std::string packet) {

	auto pinfo = pm::split(packet, ',');

	int posX = stoi(pinfo[0]);
	int posY = stoi(pinfo[1]);

	if (pinfo[2] == "1" && !IsSelectedLH(EleA)) {

		int _posX = -1, _posY = -1;
		std::tuple<int, int> xy = { -1, -1 };

		if (auto_cast_flag) {
			xy = GetUserTargetPos();
		}
		else if (cast_mode == 0) {
			xy = GetClosestTargetPos(posX, posY);
		}
		else if (cast_mode == 1 && selected_player_id > 0) {
			xy = GetManualTargetPos();
		}

		_posX = std::get<0>(xy);
		_posY = std::get<1>(xy);

		if (_posX == -1 || _posY == -1) {
			_posX = posX;
			_posY = posY;
		}

		SysFreeString(*dataSend);
		*dataSend = pm::ConvertStringToBSTR(pm::build_WLC(_posX, _posY));
	}
}

bool IsInRange(int posX, int posY) {
	bool xrange = std::abs(posX - user_pos_x) <= 11;
	bool yrange = std::abs(posY - user_pos_y) <= 9;
	return xrange && yrange;
}

bool IsUserPosOutbounds() {
	return user_pos_x > 88 || user_pos_y > 90 || user_pos_x < 13 || user_pos_y < 11;
}

void SetUserpos(int posX, int posY) {
	lockM1234.store(true);
	user_pos_x = posX;
	user_pos_y = posY;
	lockM1234.store(false);
}

std::tuple<int, int> GetManualTargetPos() {
	auto itp = players_in_range.find(selected_player_id);

	if (itp != players_in_range.end())
		return { itp->second->posX, itp->second->posY };

	return { -1, -1 };
}

std::tuple<int, int> GetUserTargetPos() {
	if (IsUserPosOutbounds())
		return { -1, -1 };

	return { user_pos_x, user_pos_y };
}

std::tuple<int, int> GetClosestTargetPos(int posX, int posY) {

	int closestX = -1, closestY = -1;
	double minDistance = 9;

	auto updateClosest = [&](int targetX, int targetY) {
		double distance = std::sqrt(std::pow(posX - targetX, 2) + std::pow(posY - targetY, 2));
		if (distance < minDistance) {
			minDistance = distance;
			closestX = targetX;
			closestY = targetY;
		}
	};

	for (const auto& pr : players_in_range) {
		if (!pr.second) continue;
		updateClosest(pr.second->posX, pr.second->posY);
	}

	//if (minDistance < 8 && !players_in_range.empty() && (closestX == -1 || closestY == -1)) {
	//	dlg.logDebug("GetClosestTargetPos: Player should be finded",
	//		"posX", posX, "posY", posY, "minDistance", minDistance);
	//}

	if (closestX > -1 && closestY > -1)
		return { closestX, closestY };

	for (const auto& nr : npcs_in_range) {
		updateClosest(nr.second.first, nr.second.second);
	}

	//if (minDistance < 8 && !npcs_in_range.empty() && (closestX == -1 || closestY == -1)) {
	//	dlg.logDebug("GetClosestTargetPos: Npc should be finded",
	//		"posX", posX, "posY", posY, "minDistance", minDistance);
	//}

	if (closestX == -1 || closestY == -1) {
		return { posX, posY };
	}

	return { closestX, closestY };
}

//ToDo review cheknewtarg
void CheckPlayerTargets() {
	//dlg.logDebug("CheckPlayerTargets:");

	for (const auto& pm : players_in_map) {
		if (!pm.second) continue;

		int pid = pm.second->id;
		int pmX = pm.second->posX;
		int pmY = pm.second->posY;

		auto itr = players_in_range.find(pid);
		bool inRange = IsInRange(pmX, pmY);

		if (itr != players_in_range.end()) {
			if (!inRange) {
				RemoveRangePlayer(itr);
				continue;
			}

			//if (itr->second->posX != pmX && itr->second->posY != pmY) {
			itr->second->posX = pmX;
			itr->second->posY = pmY;
			//}
		}
		else if (inRange) {
			AddPlayerRange(pid, pmX, pmY);
		}
	}
}

void CheckNpcTargets() {
	//dlg.logDebug("CheckNpcTargets:");
	for (const auto& nm : npcs_in_map) {
		if (!nm.second) continue;

		int nid = nm.second->id;
		int nmX = nm.second->posX;
		int nmY = nm.second->posY;

		auto itr = npcs_in_range.find(nid);
		bool inRange = IsInRange(nmX, nmY);

		if (itr != npcs_in_range.end()) {
			if (!inRange) {
				RemoveRangeNpc(nid);
				continue;
			}

			//if (itr->second.first != nmX && itr->second.second != nmY) {
			itr->second.first = nmX;
			itr->second.second = nmY;
			//}
		}
		else if (inRange) {
			npcs_in_range.emplace(nid, std::make_pair(nmX, nmY));
		}
	}
}

//void CheckNewTargets() {
//	// Process players in the map
//	for (const auto& pm : players_in_map) {
//		auto& pl = pm.second;
//		
//		int playerId = pm.second->id;
//		auto itp = players_in_range.find(playerId);
//		bool inRange = IsInRange(pm.second->posX, pm.second->posY);
//
//		if (inRange) {
//			if (itp == players_in_range.end()) {
//				// Add player to the range if not already present
//				PlayerRange* pr = new PlayerRange();
//				pr->ID = playerId;
//				pr->posX = pm.second->posX;
//				pr->posY = pm.second->posY;
//				pr->faction = pm.second->faction;
//				players_in_range.emplace(playerId, std::move(pr));
//
//				dlg.logDebug("CheckNewTargets", "Player emplaced",
//					"posX", posX, "posY", posY, "minDistance", minDistance);
//			}
//		}
//
//		//	else if (itp != players_in_range.end()) {
//				// Remove player from the range
//		//		RemoveRangePlayer(itp);
//		//	}
//	}
//
//	// Process NPCs in the map
//	for (const auto& nm : npcs_in_map) {
//		int npcId = nm.second->id;
//		auto itn = npcs_in_range.find(npcId);
//		bool inRange = IsInRange(nm.second->posX, nm.second->posY);
//
//		if (inRange) {
//			if (itn == npcs_in_range.end()) {
//				// Add NPC to the range if not already present
//				npcs_in_range.emplace(npcId, std::make_pair(nm.second->posX, nm.second->posY));
//			}
//		}
//		//else if (itn != npcsh_in_range.end()) {
//			// Remove NPC from the range
//		//	RemoveRangeNpc(npcId);
//		//}
//	}
//}

void PlayLocalWav(int wav) {
	std::string packet = pm::build_TW(wav, user_pos_x, user_pos_y);
	SendToClient(packet);
}

/*
VOID AutoRegenHpMan()
{
	try
	{
		DWORD* hpMaxAddress = (DWORD*)(0x00a35c6c);
		DWORD* hpActAddress = (DWORD*)(0x00a35c68);

		int* HPMAX = (int*)hpMaxAddress;
		int* HPACT = (int*)hpActAddress;

		while (true)
		{
			if (*HPACT != 0)
			{
				if (*HPACT != *HPMAX)
				{
					string message = "USEd>?";
					packets.push_back(message);
					message = "USAT<C";
					packets.push_back(message);
				}
			}
			Sleep(200);
		}
	}
	catch (int e)
	{
		OutputDebugStringW(ConvertStringToBSTR("ERROR-> Place: AutoPotas()  Id: " + std::to_string(e)));
	}
}*/

VOID SendToClient(const std::string& packet)
{
	try
	{
		if (writeLogs) dlg.logData(packet, DlibLogger::LogType::LOCAL_RECV);

		BSTR recvPacket = pm::ConvertStringToBSTR(packet);

		PFunctionRecv(recvPacket);

		SysFreeString(recvPacket);
	}
	catch (const std::exception& e)
	{
		std::ostringstream oss;
		oss << "At: SendToClient(args) | "
			<< "Packet: " << packet << " | "
			<< "Ex details: " << e.what();

		dlg.logData(oss.str(), DlibLogger::LogType::DLIB_EX);
	}
}

VOID SendToServer(const std::string& packet)
{
	try
	{
		if (writeLogs) dlg.logData(packet, DlibLogger::LogType::LOCAL_SEND);

		BSTR convertedSend = pm::ConvertStringToBSTR(packet);

		PFunctionSend(&convertedSend);

		SysFreeString(convertedSend);
	}
	catch (const std::exception& e)
	{
		std::ostringstream oss;
		oss << "At: SendToServer(args) | "
			<< "Packet: " << packet << " | "
			<< "Ex details: " << e.what();

		dlg.logData(oss.str(), DlibLogger::LogType::DLIB_EX);
	}
}

std::string hexToString(const std::string& hex) {
	std::string result;
	result.reserve(hex.length() / 2);

	for (size_t i = 0; i < hex.length(); i += 2) {
		if (!std::isxdigit(hex[i]) || !std::isxdigit(hex[i + 1])) {
			//throw std::invalid_argument("Invalid hex character in input string.");
		}

		char high = std::tolower(hex[i]);
		char low = std::tolower(hex[i + 1]);

		// Convert hex characters to a byte
		char byte = (high >= 'a' ? (high - 'a' + 10) : (high - '0')) * 16 +
			(low >= 'a' ? (low - 'a' + 10) : (low - '0'));
		result.push_back(byte);
	}

	return result;
}

BOOL StartsWith(BSTR sValue, const WCHAR* pszSubValue) {
	if (!sValue || !pszSubValue) {
		return FALSE; // Handle null input
	}

	size_t subValueLen = wcslen(pszSubValue);

	// Check if the substring length exceeds the main string length
	if (SysStringLen(sValue) < subValueLen) {
		return FALSE;
	}

	return wcsncmp(sValue, pszSubValue, subValueLen) == 0;
}

BOOL StartsWith(BSTR sValue, const std::vector<const WCHAR*>& pszSubValue) {

	if (!sValue) return FALSE;

	for (const auto& subValue : pszSubValue) {

		if (!subValue) return FALSE;

		size_t subValueLen = wcslen(subValue);
		if (SysStringLen(sValue) < subValueLen) return FALSE;

		if (wcsncmp(sValue, subValue, subValueLen) == 0)
			return TRUE;
	}

	return FALSE;
}

BOOL StartsWithAndNot(BSTR sValue, const WCHAR* pszSubValue, const WCHAR* npszSubValue) {
	if (!sValue || !pszSubValue || !npszSubValue) {
		return FALSE; // Handle null input
	}

	size_t subValueLen = wcslen(pszSubValue);
	size_t nsubValueLen = wcslen(npszSubValue);

	// Check if the substring length exceeds the main string length
	if (SysStringLen(sValue) < subValueLen) {
		return FALSE;
	}

	if (SysStringLen(sValue) < nsubValueLen) {
		return FALSE;
	}

	return wcsncmp(sValue, pszSubValue, subValueLen) == 0 && wcsncmp(sValue, npszSubValue, nsubValueLen) != 0;
}




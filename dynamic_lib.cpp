#include "stdafx.h"

namespace pm = PacketManager;

#pragma comment(lib, "comsuppw.lib")

DWORD oldTTeclas = GetTickCount();
//DWORD ttAutoPot = GetTickCount();

bool isRadarRunning = false;
bool hideCheating = false;
bool writeLogs = true;
bool avoidPacketLog = false;

int cast_mode = 0;

int userX = 0;
int userY = 0;

int userPID = 0;
int userBCR = 0;
bool userParalized = false;
bool userMeditando = false;
bool insecureMap = false;

std::atomic<bool> lockM1234{ false };

const std::string userName = "growland";
const std::string del_spell_words = ";1 ";

std::unordered_map<int, std::string> userSpells;

std::unordered_set<std::string> whiteSpells = {
	"Remover paralisis",
	"Invisibilidad",
	"Celeridad",
	"Fuerza",
	"Curar veneno"
};

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

std::unordered_set<int> npcsBodyToAvoid = {
	154, //ele agua
	15, //guardia impe
	268, //guardia caos
	19, //sastre
	33, //bebida?
	17, //bardo ?
	16, //herrero?
	34, //pociones
	37, //banco
};

bool uk_flag = false;
bool lh_flag = false;
bool autoCast = false;
bool toogleAutoCast = false;
bool directCast = false;
bool avoidInviCast = false;
bool lockLastTarget = false;
bool flagLastTarget = false;
bool isSelLhWhite = false;

int selectedPid = 0;
int selectedNid = 0;
int selectedLH = 0;
int lastTargetPID = 0;

const int WAV_SCREEN_CAPTURE = 555;
const int WAV_PRC_PPL_PPP = 324;
const int WAV_FEATURE_ON = 194;
const int WAV_FEATURE_OFF = 199;
const int WAV_CC_TRIGGER = 100;
const int WAV_BP_TRIGGER = 101;

const int BODY_DEAD1 = 8;
const int BODY_DEAD2 = 145;

std::unordered_map<int, std::shared_ptr<Npc>> rngNpcs;
std::unordered_map<int, std::shared_ptr<Npc>> mapNpcs;

std::unordered_map<int, std::shared_ptr<Player>> rngPlayers;
std::unordered_map<int, std::shared_ptr<Player>> mapPlayers;

std::vector<std::string> packets;

const std::string LOG_FILEPATH = "C:\\Users\\joaco\\Documents\\dlib_log.txt";
DlibLogger dlg(LOG_FILEPATH);
RadarManager radar("Untitled - Notepad");

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
	avoidPacketLog = false;

	if (StartsWith(dataRecv, L"PAIN")) {
		PlayLocalWav(WAV_SCREEN_CAPTURE);
		HideCheat(true);
		PlayLocalWav(WAV_PRC_PPL_PPP);
		dlg.logDebug("MyRecvData - CAUTION! PAIN", "hidecheat", hideCheating, "radar run", isRadarRunning);
	}

	if (StartsWith(dataRecv, { L"PRC", L"PPP", L"PPL" })) {
		PlayLocalWav(WAV_PRC_PPL_PPP);
		dlg.logDebug("MyRecvData - CAUTION! PRC PPP PPL", "hidecheat", hideCheating, "radar run", isRadarRunning);
	}

	if (StartsWith(dataRecv, L"LH")) {
		uk_flag = false;
		lh_flag = false;
		isSelLhWhite = false;
		autoCast = false;
		//flagLastTarget = false;
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"MEDOK")) {
		userMeditando = !userMeditando;
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"V3")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_V3(dataRecv, packet);
		avoidPacketLog = true;
	}

	//ToDo :: if (StartsWith(dataRecv, L"EST")) 

	if (StartsWith(dataRecv, L"BP")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_BP(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"CM")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CM(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"CC")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CC(dataRecv, packet);
		//avoidPacketLog = true;
	}

	//ToDo :: if (StartsWith(dataRecv, L"CP")) to verify is player has died

	if (StartsWith(dataRecv, L"CP")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CP(packet);
		avoidPacketLog = true;
	}

	if (StartsWithAndNot(dataRecv, L"CR", L"CRA")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_CR(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"QQ")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_QQ(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, { L"MP", L"LP" })) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_MP(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"P9")) {
		userParalized = true;
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"P8")) {
		userParalized = false;
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"PU")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 2);
		Intercept_PU(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"SHS")) {
		std::string packet = pm::ConvertBSTRPacket(dataRecv, 3);
		Intercept_SHS(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, L"||Has lanzado") && lockLastTarget) {
		if (!flagLastTarget || lastTargetPID == 0) {
			std::string packet = pm::ConvertBSTRPacket(dataRecv, 0);
			Intercept_CastON(packet);
		}
		avoidPacketLog = true;
	}

	if (StartsWith(dataRecv, { L"AA", L"TW",L"EX",L"SHIV", L"SHII",
		L"HO", L"OTII", L"OTIV", L"RTR", L"BQ" })) {
		avoidPacketLog = true;
	}

	if (writeLogs && !avoidPacketLog) dlg.logData(pm::ConvertBSTRToString(dataRecv), DlibLogger::LogType::RECV);

	PFunctionRecv(dataRecv);

	__asm POPFD;
	__asm POPAD;
}

VOID WINAPI MySendData(BSTR* dataSend)
{
	__asm PUSHAD;
	__asm PUSHFD;
	avoidPacketLog = false;

	if (StartsWith(*dataSend, L"WLC")) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 3);
		Intercept_WLC(dataSend, packet);
		avoidPacketLog = true;
	}

	if (StartsWith(*dataSend, L"UK1")) {
		uk_flag = true;
		avoidPacketLog = true;
	}

	if (StartsWith(*dataSend, L"LH")) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 2);
		Intercept_LH(packet);
		avoidPacketLog = true;
	}

	if (StartsWith(*dataSend, L"LC")) {
		if (cast_mode == 1) {
			std::string packet = pm::ConvertBSTRPacket(*dataSend, 2);
			Intercept_LC(packet);
		}
		avoidPacketLog = true;
	}

	if (StartsWith(*dataSend, L"RC")) {
		if (lockLastTarget) {
			lastTargetPID = 0;
			flagLastTarget = false;
		}
		avoidPacketLog = true;
	}

	if (StartsWith(*dataSend, { L"M1",L"M2", L"M3", L"M4" })) {
		std::string packet = pm::ConvertBSTRPacket(*dataSend, 1);
		Intercept_M1234(packet);
		avoidPacketLog = true;
	}

	if (writeLogs && !avoidPacketLog) dlg.logData(pm::ConvertBSTRToString(*dataSend), DlibLogger::LogType::SEND);

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
				isRadarRunning = radar.initRadar();
			}
			else {
				radar.finalizeRadar();
				isRadarRunning = false;
			}

			Sleep(250);
		}

		if ((GetKeyState(VK_END) & 0x100) != 0) {
			HideCheat(true);
		}

		if ((GetKeyState(VK_F1) & 0x100) != 0) {
			autoCast = false;
			toogleAutoCast = false;
			directCast = !directCast;

			if (directCast)
				PlayLocalWav(WAV_FEATURE_ON);
			else
				PlayLocalWav(WAV_FEATURE_OFF);

			Sleep(15);
		}

		if ((GetKeyState(VK_F2) & 0x100) != 0) {
			autoCast = false;
			directCast = false;
			toogleAutoCast = !toogleAutoCast;

			if (toogleAutoCast)
				PlayLocalWav(WAV_FEATURE_ON);
			else
				PlayLocalWav(WAV_FEATURE_OFF);

			Sleep(15);
		}

		if ((GetKeyState(VK_F3) & 0x100) != 0) {
			avoidInviCast = !avoidInviCast;

			if (avoidInviCast)
				PlayLocalWav(WAV_FEATURE_ON);
			else
				PlayLocalWav(WAV_FEATURE_OFF);

			Sleep(15);
		}

		//if ((GetKeyState(VK_F3) & 0x100) != 0) {
			//lockLastTarget = !lockLastTarget;

			//if (lockLastTarget)
			//	PlayLocalWav(WAV_FEATURE_ON);
			//else
			//	PlayLocalWav(WAV_FEATURE_OFF);

			//Sleep(15);
		//}

		if ((GetKeyState(VK_XBUTTON2) & 0x100) != 0) {
			toogleAutoCast = false;
			directCast = false;
			autoCast = true;
		}

		if ((GetKeyState(VK_XBUTTON1) & 0x100) != 0) {
			Sleep(30);
			SendToServer(del_spell_words);
			Sleep(10);
		}

		if ((GetKeyState(VK_DELETE) & 0x100) != 0) {

		}

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
		radar.finalizeRadar();
		isRadarRunning = false;
	}

	for (const auto& player : mapPlayers) {

		auto& pl = player.second;

		if (pl->inviDetected) {

			if (pl->name.find(" [I]") != std::string::npos)
				pl->name.erase(pl->name.length() - 4, 4);

			pl->invi = 1;
			pl->inviDetected = 0;

			SendToClient(pm::build_BP(pl->id));
			SendToClient(pm::build_CC(pl));
		}
	}

	Sleep(35);
}

void Intercept_CM(const std::string& packet) {
	mapPlayers.clear();
	rngPlayers.clear();

	mapNpcs.clear();
	rngNpcs.clear();

	selectedNid = 0;
	selectedPid = 0;

	auto pinfo = pm::split(packet, ',');
	insecureMap = pinfo[1] != "0";
}

void Intercept_SHS(const std::string& packet) {
	auto sinfo = pm::read_SHS(packet);
	userSpells.emplace(sinfo);
}

bool IsSelectedLH(const std::string& sname) {
	if (selectedLH <= 0) return false;
	return selectedLH == GetSpellPosition(sname);
}

int GetSpellPosition(const std::string& sname) {
	for (const auto& spl : userSpells) {
		if (spl.second == sname)
			return spl.first;
	}

	return -1;
}

std::string GetSelectedLHName() {
	if (selectedLH <= 0) return "";
	auto spl = userSpells.find(selectedLH);
	if (spl != userSpells.end())
		return spl->second;

	return "";
}

void Intercept_CR(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	if (pinfo[1] != "0") return;
	if (npcsBodyToAvoid.find(stoi(pinfo[0])) != npcsBodyToAvoid.end()) return;

	AddNPC(pinfo);
}

void Intercept_QQ(const std::string& packet) {
	auto pinfo = pm::split(packet, '@');
	int nid = stoi(pinfo[0]);
	RemoveMapNpc(nid);
}

void AddNPC(const std::vector<std::string>& pinfo) {
	int nid = stoi(pinfo[3]);
	if (mapPlayers.count(nid) > 0) return;

	auto npc = std::make_shared<Npc>(pinfo, false);
	mapNpcs.emplace(nid, npc);

	if (IsInRange(npc->posX, npc->posY)) {
		rngNpcs.emplace(nid, npc);
	}
}

void RemoveMapNpc(int nid) {
	rngNpcs.erase(nid);
	mapNpcs.erase(nid);
}

void Intercept_CC(BSTR& dataRecv, const std::string& packet) {

	auto pinfo = pm::split(packet, ',');
	int pid = stoi(pinfo[3]);

	if (mapPlayers.count(pid) > 0) return;

	if (pinfo[11] == userName) {

		userPID = pid;
		SetUserpos(stoi(pinfo[4]), stoi(pinfo[5]));
		userBCR = stoi(pinfo[12]);
	}
	else {

		bool detected = !hideCheating && pinfo[13] == "1";

		if (detected)
		{
			pinfo[11] += " [I]";
			pinfo[13] = "0";
			SysFreeString(dataRecv);
			dataRecv = pm::ConvertStringToBSTR(pm::build_CC(pinfo));
		}

		AddPlayer(pid, detected, pinfo);
	}
}

void Intercept_CP(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');
	int pid = stoi(pinfo[0]);
	if (pid == userPID) return;

	auto itpm = mapPlayers.find(pid);
	if (itpm != mapPlayers.end()) {
		int body = stoi(pinfo[1]);
		int head = stoi(pinfo[2]);

		if (itpm->second->body != body)
			itpm->second->body = body;

		if (itpm->second->head != head)
			itpm->second->head = head;

		itpm->second->isDead = itpm->second->body == BODY_DEAD1 || itpm->second->body == BODY_DEAD2;
	}
}

void AddPlayer(int pid, bool detected, const std::vector<std::string>& pinfo) {
	//if (mapPlayers.count(pid) > 0) return;

	auto pl = std::make_shared<Player>(pinfo, detected);
	pl->isDead = pl->body == BODY_DEAD1 || pl->body == BODY_DEAD2;

	mapPlayers.emplace(pid, pl);

	if (IsInRange(pl->posX, pl->posY)) {
		rngPlayers.emplace(pid, pl);
	}

	if (insecureMap) PlayLocalWav(WAV_CC_TRIGGER);
}

bool RemoveMapPlayer(int pid) {
	int hits = 0;
	hits += rngPlayers.erase(pid);
	hits += mapPlayers.erase(pid);

	return hits > 0;
}

void RemoveMapPlayer(const std::unordered_map<int, std::shared_ptr<Player>>::iterator& it) {
	rngPlayers.erase(it);
	mapPlayers.erase(it);
}

void Intercept_MP(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	int eid = stoi(pinfo[0]);
	int posX = stoi(pinfo[1]);
	int posY = stoi(pinfo[2]);
	bool inRange = IsInRange(posX, posY);

	auto itpm = mapPlayers.find(eid);
	if (itpm != mapPlayers.end()) {
		itpm->second->posX = posX;
		itpm->second->posY = posY;

		auto itpr = rngPlayers.find(eid);
		if (itpr != rngPlayers.end()) {
			if (!inRange) {
				rngPlayers.erase(itpr);
			}
		}
		else if (inRange) {
			rngPlayers.emplace(eid, itpm->second);
		}
	}

	auto itnm = mapNpcs.find(eid);
	if (itnm != mapNpcs.end()) {
		itnm->second->posX = posX;
		itnm->second->posY = posY;

		auto itnr = rngNpcs.find(eid);
		if (itnr != rngNpcs.end()) {
			if (!inRange) {
				rngNpcs.erase(itnr);
			}
		}
		else if (inRange) {
			rngNpcs.emplace(eid, itnm->second);
		}
	}
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

void Intercept_PU(const std::string& packet) {
	auto xy = pm::read_PU(packet);
	SetUserpos(std::get<0>(xy), std::get<1>(xy));
}

void Intercept_LC(const std::string& packet) {
	auto pinfo = pm::split(packet, ',');

	int posX = stoi(pinfo[0]);
	int posY = stoi(pinfo[1]);

	SetManualTarget(posX, posY);
}

void SetManualTarget(int posX, int posY) {

	for (const auto& pr : rngPlayers) {
		if (pr.second->posX == posX && pr.second->posY == posY) {
			selectedPid = pr.first;
			return;
		}
	}

	for (const auto& nr : rngNpcs) {
		if (nr.second->posX == posX && nr.second->posY == posY) {
			selectedNid = nr.first;
			break;
		}
	}
}

void Intercept_V3(BSTR& dataRecv, const std::string& packet) {
	if (hideCheating) return;

	std::string dcpt_packet = pm::decrypt_packet(packet);
	auto pinfo = pm::split(dcpt_packet, ',');

	int pid = stoi(pinfo[1]);

	auto itp = mapPlayers.find(pid);

	if (itp != mapPlayers.end()) {
		auto& pl = itp->second;
		//if (pl->name == userName) return;

		bool invi = pinfo[4] == "1";

		if (invi) {
			pl->name += " [I]";
			pl->invi = 0;
			pl->inviDetected = 1;
			pinfo[4] = "0";

			SendToClient(pm::build_BP(pid));
			Sleep(8);
			SendToClient(pm::build_CC(pl));

			SysFreeString(dataRecv);
			dataRecv = pm::ConvertStringToBSTR(pm::build_V3(pinfo));
		}
		else {
			if (pl->name.find(" [I]") != std::string::npos)
				pl->name.erase(pl->name.length() - 4, 4);

			pl->inviDetected = 0;

			SendToClient(pm::build_BP(pid));
			Sleep(8);
			SendToClient(pm::build_CC(pl));
		}
	}
}

void Intercept_M1234(const std::string& packet) {

	if (lockM1234.load()) return;
	if (userParalized || userMeditando) return;

	int dir = stoi(packet);
	switch (dir)
	{
		case 1:
			if (userY - 1 >= 11)
				userY--;
			break;
		case 2:
			if (userX + 1 <= 88)
				userX++;
			break;
		case 3:
			if (userY + 1 <= 90)
				userY++;
			break;
		case 4:
			if (userX - 1 >= 13)
				userX--;
			break;
		default:
			break;
	}

	CheckPlayerTargets();
	CheckNpcTargets();
}

void Intercept_BP(const std::string& packet) {
	int eid = stoi(packet);
	if (RemoveMapPlayer(eid)) {
		if (insecureMap)
			PlayLocalWav(WAV_BP_TRIGGER);

		return;
	}

	RemoveMapNpc(eid);
}

void Intercept_WLC(BSTR* dataSend, const std::string& packet) {
	if (directCast) return;

	auto pinfo = pm::split(packet, ',');

	int posX = stoi(pinfo[0]);
	int posY = stoi(pinfo[1]);

	if (pinfo[2] == "1" && !IsSelectedLH(EleA)) {

		int _posX = -1, _posY = -1;
		std::tuple<int, int> xy = { -1, -1 };

		if (lockLastTarget) {
			xy = lastTargetPID > 0 ? GetLastTargetPos() : GetClosestTargetPos(posX, posY);
		}
		else if (toogleAutoCast && isSelLhWhite) {
			xy = GetUserTargetPos();
		}
		else if (autoCast) {
			xy = GetUserTargetPos();
		}
		else if (cast_mode == 0) {
			xy = GetClosestTargetPos(posX, posY);
		}
		else if (cast_mode == 1 && selectedPid > 0) {
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

void Intercept_CastON(const std::string& packet) {


	auto pinfo_0 = pm::split(packet, ' ');
	size_t p0size = pinfo_0.size();
	std::string pinfo_0last = pinfo_0[p0size - 1];
	auto pinfo_1 = pm::split(pinfo_0last, '~');
	std::string pname = pinfo_1[0];

	int pid = GetPlayerID(pname);
	if (pid < 0) return;

	lastTargetPID = pid;


	//if (pinfo0[pinfo0.size() - 2] == "la")
	//auto pinfo1 = pm::split(pinfo0[pinfo0.size() - 1], '~');
	//std::string targetname = pinfo0[1];	
}

int GetPlayerID(const std::string& pname) {
	for (const auto& pl : mapPlayers) {
		if (pl.second->isDead) continue;
		if (pl.second->bcr == userBCR) continue;

		if (pl.second->name.find(pname) != std::string::npos) {
			return pl.first;
		}
	}

	return -1;
}

void Intercept_LH(const std::string& packet) {
	selectedLH = stoi(packet);
	lh_flag = true;
	isSelLhWhite = false;

	auto uspl = userSpells.find(selectedLH);
	if (uspl != userSpells.end()) {
		isSelLhWhite = uspl->second == Remo || uspl->second == Invi ||
			uspl->second == Cele || uspl->second == Fue;
	}
}

bool IsInRange(int posX, int posY) {
	if (userX == 0 || userY == 0) return false;
	bool xrange = std::abs(posX - userX) <= 11;
	bool yrange = std::abs(posY - userY) <= 9;
	return xrange && yrange;
}

bool IsUserPosOutbounds() {
	return userX > 88 || userY > 90 || userX < 13 || userY < 11;
}

void SetUserpos(int posX, int posY) {
	lockM1234.store(true);
	userX = posX;
	userY = posY;
	lockM1234.store(false);
}

std::tuple<int, int> GetManualTargetPos() {
	auto itp = rngPlayers.find(selectedPid);

	if (itp != rngPlayers.end()) {
		//flagLastTarget = true;
		return { itp->second->posX, itp->second->posY };
	}

	return { -1, -1 };
}

std::tuple<int, int> GetUserTargetPos() {
	if (IsUserPosOutbounds())
		return { -1, -1 };

	return { userX, userY };
}

std::tuple<int, int> GetLastTargetPos() {
	auto itrg = rngPlayers.find(lastTargetPID);
	if (itrg != rngPlayers.end()) {

		if (avoidInviCast && itrg->second->inviDetected)
			return { -1, -1 };

		return { itrg->second->posX, itrg->second->posY };
	}

	return { -1, -1 };
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

	auto isValidTarget = [&](const std::shared_ptr<Player>& pl) -> bool {
		if (pl->bcr == 1) return false;
		if (pl->isDead) return false;
		if (!isSelLhWhite && pl->bcr == userBCR) return false;
		if (avoidInviCast && pl->inviDetected) return false;

		return true;
	};

	for (const auto& pr : rngPlayers) {
		if (!isValidTarget(pr.second)) continue;
		updateClosest(pr.second->posX, pr.second->posY);
	}

	if (closestX > -1 && closestY > -1) {
		flagLastTarget = lockLastTarget ? true : false;
		return { closestX, closestY };
	}

	for (const auto& nr : rngNpcs) {
		updateClosest(nr.second->posX, nr.second->posY);
	}

	if (closestX == -1 || closestY == -1) {
		return { posX, posY };
	}

	return { closestX, closestY };
}

void CheckPlayerTargets() {
	for (const auto& pm : mapPlayers) {

		int pid = pm.second->id;
		bool inRange = IsInRange(pm.second->posX, pm.second->posY);

		auto itpr = rngPlayers.find(pid);
		if (itpr != rngPlayers.end()) {
			if (!inRange) {
				rngPlayers.erase(itpr);
			}
		}
		else if (inRange) {
			rngPlayers.emplace(pid, pm.second);
		}
	}
}

void CheckNpcTargets() {
	for (const auto& nm : mapNpcs) {
		int nid = nm.second->id;

		bool inRange = IsInRange(nm.second->posX, nm.second->posY);

		auto itnr = rngNpcs.find(nid);
		if (itnr != rngNpcs.end()) {
			if (!inRange) {
				rngNpcs.erase(itnr);
			}
		}
		else if (inRange) {
			rngNpcs.emplace(nid, nm.second);
		}
	}
}

void PlayLocalWav(int wav) {
	std::string packet = pm::build_TW(wav, userX, userY);
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
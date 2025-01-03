#include "stdafx.h"

namespace pm = PacketManager;
namespace rm = RadarManager;

#pragma comment(lib, "comsuppw.lib")

DWORD oldTTeclas = GetTickCount();
//DWORD ttAutoPot = GetTickCount();

HANDLE keyPressHandle;

int sleepRadarThread = 0;
bool playersMapRadarFlag = false;

BOOL isRadarRunning = false;
bool isKeyPressRunning = false;
bool hideCheating = false;
bool writeLogs = true;
bool recv_tempWriteLogs = false;
bool send_tempWriteLogs = false;

int cast_mode = 0;
int cast_target = 0;

int user_id = 0;
int user_pos_x = 0;
int user_pos_y = 0;
int user_faction = 0;
bool user_paralized = false;
bool user_meditando = false;

const std::string user_name = "growland";
const std::string del_spell_words = ";1 ";

std::string pos_apoca;
std::string pos_remove;
std::string pos_inmovilize;
std::string pos_descarga;
std::string pos_tormenta;
std::string pos_misil;
std::string pos_flecha;
std::string pos_dardo;
std::string pos_invi;
std::string pos_cele;
std::string pos_fue;
std::string pos_curv;
std::string pos_elea;

bool uk_flag = false;
bool lh_flag = false;
bool auto_cast_flag = false;

int selected_player_id = 0;
int selected_npch_id = 0;
int selected_lh = 0;

int spell_casting_delay = 800;
int loop_caster_delay = 55;

const int spell_casting_index = 50;
const int loop_caster_index = 5;

const int min_casting_delay = 950;
const int min_loop_caster_delay = 55;

std::unordered_map<int, Player*> players_in_map;
std::unordered_map<int, PlayerRange*> players_in_range;

std::unordered_map<int, Npch*> npcsh_in_map;
std::unordered_map<int, std::pair<int, int>> npcsh_in_range;

std::string team_mate_name;

std::vector<std::string> packets;

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

	recv_tempWriteLogs = true;

	if (StartsWith(dataRecv, L"LH")) {
		uk_flag = false;
		lh_flag = false;
		auto_cast_flag = false;
		///selected_lh = 0;
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"PAIN")) {
		rm::FinalizeRadar();
		isRadarRunning = false;
		HideCheat();
	}

	if (StartsWith(dataRecv, L"MEDOK")) {
		user_meditando = !user_meditando;
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"V3")) {

		std::string packet_str = pm::ConvertBSTRToString(dataRecv).substr(2);
		std::string decripted = pm::decrypt_packet(packet_str);

		std::vector<std::string> player_info = pm::split(decripted, ',');

		int player_id = stoi(player_info[1]);

		if (player_id != user_id) {
			int player_invisible = stoi(player_info[4]);
			auto player = players_in_map.find(player_id);

			if (player != players_in_map.end()) {

				player->second->isInvisible = player_invisible == 1;

				if (!hideCheating && player_invisible == 1) {

					player->second->name += " [I]";
					player->second->isInvisible = false;
					player->second->inviDetected = true;

					SendToClient(pm::build_bp_packet(player_id));
					SendToClient(pm::build_cc_packet(player));
					SysFreeString(dataRecv);
					dataRecv = pm::build_v3_packet(player_info);
				}
				else {
					player->second->name = player->second->orgName;
					player->second->inviDetected = false;

					SendToClient(pm::build_bp_packet(player_id));
					SendToClient(pm::build_cc_packet(player));
				}
			}
		}

		recv_tempWriteLogs = false;
	}

	//if (StartsWith(dataRecv, L"EST")) {

		//packet_str = ConvertBSTRToString(dataRecv).substr(3);
		//writePacketLog(packet_str, DLIB);
		//writePacketLog("EST dec> " + decrypt_packet(packet_str), DLIB);
	//}

	if (StartsWith(dataRecv, L"BP")) {
		std::string packet_str = pm::ConvertBSTRToString(dataRecv).substr(2);

		int id = stoi(packet_str);

		RemoveMapPlayer(id);
		RemoveRangePlayer(id);

		RemoveMapNpc(id);
		RemoveRangeNpc(id);
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"CM")) {
		MapChanged();
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"CC")) {

		std::vector<std::string> player_info = pm::packet_split(dataRecv, 2);
		AddPlayer(player_info);

		int player_invisible = stoi(player_info[13]);
		std::string player_name = player_info[11];

		if (player_name != user_name) {
			if (!hideCheating && player_invisible == 1) {

				player_info[11] += " [I]";
				player_info[13] = "0";

				SysFreeString(dataRecv);
				dataRecv = pm::build_cc_packet(player_info);
			}
		}

		//playersMapRadarFlag = !players_in_map.empty();
		//sleepRadarThread = 50;
		recv_tempWriteLogs = false;
	}

	//if (StartsWith(dataRecv, L"CR")) {
	//	if (splitedPacketLen(dataRecv) > 3) {
	//		Intercept_npch_CR_MP(dataRecv, false, 3, 4, 5);
	//	}
	//}

	if (StartsWith(dataRecv, L"CR")) {
		Intercept_CR(dataRecv);
		recv_tempWriteLogs = false;
	}

	///if (StartsWith(dataRecv, L"QQ")) {
	//	int nid = stoi(pm::ConvertBSTRToString(dataRecv).substr(2));
	//	RemoveMapNpc(nid);
	//	RemoveRangeNpc(nid);
	//}

	if (StartsWith(dataRecv, L"MP")) {
		Intercept_MP(dataRecv);
		recv_tempWriteLogs = false;
	}

	//if (StartsWith(dataRecv, L"MP")) {
	//	if (splitedPacketLen(dataRecv) == 3) {
	//		Intercept_npch_CR_MP(dataRecv, true, 0, 1, 2);
	//	}
	//	else {
	//		Intercept_player_MP(dataRecv);
	//	}
	//}

	if (wcsstr(dataRecv, L"P9")) {
		user_paralized = true;
		recv_tempWriteLogs = false;
	}

	if (wcsstr(dataRecv, L"P8")) {
		user_paralized = false;
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"PU"))
	{
		auto xy = ReadXY_stoi(dataRecv);
		user_pos_x = std::get<0>(xy);
		user_pos_y = std::get<1>(xy);
		recv_tempWriteLogs = false;
	}

	if (StartsWith(dataRecv, L"SHS"))
	{
		InterceptSHS(dataRecv);
		recv_tempWriteLogs = false;
	}

	//if (writeLogs) pm::writeLog(pm::ConvertBSTRToString(dataRecv), pm::DlibLogType::RECV);

	if (recv_tempWriteLogs) pm::writeLog(pm::ConvertBSTRToString(dataRecv), pm::DlibLogType::RECV);

	PFunctionRecv(dataRecv);

	__asm POPFD;
	__asm POPAD;
}

VOID WINAPI MySendData(BSTR* dataSend)
{
	__asm PUSHAD;
	__asm PUSHFD;

	//string packet_str;

	send_tempWriteLogs = true;

	if (StartsWith(*dataSend, L"WLC")) {

		std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(*dataSend).substr(3), ',');

		int posX = 0, posY = 0;
		posX = stoi(splitVector[0]);
		posY = stoi(splitVector[1]);

		if (splitVector[2] == "1" && selected_lh != stoi(pos_elea)) {

			if (uk_flag && lh_flag) {
				int _posX = -1, _posY = -1;
				std::tuple<int, int> xy = std::make_tuple(-1, -1);

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
				*dataSend = pm::build_wlc_packet(_posX, _posY);
				cast_target = 0;
			}
			else {
				SysFreeString(*dataSend);
				*dataSend = pm::build_lac_packet(user_pos_x, user_pos_y);
			}
		}
		send_tempWriteLogs = false;
	}

	if (StartsWith(*dataSend, L"UK1")) {
		uk_flag = true;
		send_tempWriteLogs = false;
	}

	if (StartsWith(*dataSend, L"LH")) {
		std::string packet_str = pm::ConvertBSTRToString(*dataSend);
		selected_lh = stoi(packet_str.substr(2));
		lh_flag = true;
		send_tempWriteLogs = false;
	}

	if (StartsWith(*dataSend, L"LC")) {
		if (cast_mode == 1) {
			std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(*dataSend), ',');
			SetManualTarget(stoi(splitVector[0].substr(2)), stoi(splitVector[1]));
		}
		send_tempWriteLogs = false;
	}

	if (wcsstr(*dataSend, L"M1") != NULL) {
		user_pos_y--;
		CheckNewTargets();
		send_tempWriteLogs = false;
	}

	if (wcsstr(*dataSend, L"M2") != NULL) {
		user_pos_x++;
		CheckNewTargets();
		send_tempWriteLogs = false;
	}

	if (wcsstr(*dataSend, L"M3") != NULL) {
		user_pos_y++;
		CheckNewTargets();
		send_tempWriteLogs = false;
	}

	if (wcsstr(*dataSend, L"M4") != NULL) {
		user_pos_x--;
		CheckNewTargets();
		send_tempWriteLogs = false;
	}

	//if (writeLogs) pm::writeLog(pm::ConvertBSTRToString(*dataSend), pm::DlibLogType::SEND);
	if (send_tempWriteLogs) pm::writeLog(pm::ConvertBSTRToString(*dataSend), pm::DlibLogType::RECV);

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


		if ((GetKeyState(VK_DIVIDE) & 0x100) != 0) {

			if (!isRadarRunning) {
				isRadarRunning = rm::InitRadar("Untitled - Notepad");
			}
			else {
				rm::FinalizeRadar();
				isRadarRunning = false;
			}

			Sleep(500);
		}

		if ((GetKeyState(VK_END) & 0x100) != 0) {
			hideCheating = !hideCheating;

			if (hideCheating) {
				if (isRadarRunning)
					rm::FinalizeRadar();

				HideCheat();
			}

			Sleep(250);
		}

		if ((GetKeyState(VK_MULTIPLY) & 0x100) != 0) {
			if (!isKeyPressRunning) {
				keyPressHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)KeyPressThread, 0, 0, 0);
				isKeyPressRunning = true;
			}
			else {

				WaitForSingleObject(keyPressHandle, INFINITE);
				GetExitCodeThread(keyPressHandle, 0);
				CloseHandle(keyPressHandle);
				keyPressHandle = nullptr;

				//TerminateThread(keyPressHandle, 0);
				isKeyPressRunning = false;
			}

			//	SendToClient(pm::build_console_packet("KeyPressThread: " + std::to_string(isKeyPressRunning)));

			Sleep(150);
		}

		if ((GetKeyState(VK_INSERT) & 0x100) != 0) {

		}

		oldTTeclas = GetTickCount();

		if (packets.size() != 0) {

			for (size_t i = 0; i < packets.size(); ++i)
				SendToServer(packets[i]);

			packets.clear();
			//uk_flag = false;
			//lh_flag = false;
			//auto_cast_flag = false;
		}
	}

	PFunctionLoop();

	__asm POPFD;
	__asm POPAD;
}

VOID KeyPressThread() {

	while (1) {


		if ((GetKeyState(VK_HOME) & 0x100) != 0) {
			writeLogs = !writeLogs;
			Sleep(250);
		}

		//if ((GetKeyState(VK_END) & 0x100) != 0) {
		//	hideCheating = !hideCheating;

		//	if (hideCheating) {
		//		if (isRadarRunning)
		//			rm::FinalizeRadar();

		//		HideCheat();
		//	}

		//	Sleep(250);
		//}



		if ((GetKeyState(VK_DELETE) & 0x100) != 0) {
			if (cast_mode == 0)
				cast_mode = 1;
			else
				cast_mode = 0;
			Sleep(250);
		}


		if ((GetKeyState(VK_XBUTTON2) & 0x100) != 0) {
			auto_cast_flag = true;
			//Sleep(50);
		}

		if ((GetKeyState(VK_XBUTTON1) & 0x100) != 0) {
			packets.push_back(del_spell_words);
			Sleep(400);
		}

		Sleep(loop_caster_delay);
	}
}

VOID HideCheat() {

	//pm::writeLog("players_in_map size > " + std::to_string(players_in_map.size()), pm::DlibLogType::DLIB);
	//int count = 0;
	for (const auto& pl : players_in_map) {
		if (!pl.second) continue;
		//if (!pl.second->isInvisible && !pl.second->inviDetected) continue;
		//pm::writeLog("index count > " + std::to_string(count), pm::DlibLogType::DLIB);

		if (pl.second->inviDetected) {

			//pm::writeLog("pl.second id: " + std::to_string(pl.first) + " inviDetected = true", pm::DlibLogType::DLIB);

			pl.second->name = pl.second->orgName;
			pl.second->isInvisible = true;
			pl.second->inviDetected = false;

			SendToClient(pm::build_bp_packet(pl.first));
			SendToClient(pm::build_cc_packet(pl));

			/*			std::string cc;

			std::ostringstream packetStream;
			packetStream << "CC"
				<< pl.second->inf0 << ","
				<< pl.second->inf1 << ","
				<< pl.second->inf2 << ","
				<< pl.second->id << ","
				<< pl.second->posX << ","
				<< pl.second->posY << ","
				<< pl.second->inf6 << ","
				<< pl.second->inf7 << ","
				<< pl.second->inf8 << ","
				<< pl.second->inf9 << ","
				<< pl.second->inf10 << ","
				<< pl.second->name << ","
				<< pl.second->faction << ","
				<< pl.second->isInvisible << ","
				<< pl.second->inf14 << ","
				<< pl.second->inf15;

			cc = packetStream.str();

			pm::writeLog("packetStream.str() = " + cc, pm::DlibLogType::DLIB);*/



			//	SendToClient(cc);

		}

		//	count++;
	}
}

VOID MapChanged() {

	//playersMapRadarFlag = false;
	//sleepRadarThread = 250;

	CleanupMapNpcs();
	CleanupRangeNpcs();

	CleanupMapPlayers();
	CleanupRangePlayers();

	selected_npch_id = 0;
	selected_player_id = 0;
}

VOID InterceptSHS(BSTR packet) {
	auto spell_info = ReadSpellsInfo(packet);

	std::string spell_pos = std::get<0>(spell_info);
	std::string spell_name = std::get<1>(spell_info);

	if (spell_name == "Remover paralisis") {
		pos_remove = spell_pos;
	}
	else if (spell_name == "Apocalipsis") {
		pos_apoca = spell_pos;
	}
	else if (spell_name == "Descarga eléctrica") {
		pos_descarga = spell_pos;
	}
	else if (spell_name == "Inmovilizar") {
		pos_inmovilize = spell_pos;
	}
	else if (spell_name == "Tormenta de fuego") {
		pos_tormenta = spell_pos;
	}
	else if (spell_name == "Misil magico") {
		pos_misil = spell_pos;
	}
	else if (spell_name == "Flecha eléctrica") {
		pos_flecha = spell_pos;
	}
	else if (spell_name == "Dardo magico") {
		pos_dardo = spell_pos;
	}
	else if (spell_name == "Invisibilidad") {
		pos_invi = spell_pos;
	}
	else if (spell_name == "Celeridad") {
		pos_cele = spell_pos;
	}
	else if (spell_name == "Fuerza") {
		pos_fue = spell_pos;
	}
	else if (spell_name == "Invocar elemental de agua") {
		pos_elea = spell_pos;
	}
	//else if (spell_name == "Curar veneno") {
	//	pos_curv = spell_pos;
	//}
}

VOID Intercept_player_MP(BSTR packet) {

	std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(packet), ',');

	if (splitVector.size() != 0)
	{
		int pid = stoi(splitVector[0].substr(2));
		int posX = stoi(splitVector[1]);
		int posY = stoi(splitVector[2]);

		auto& itm = players_in_map.find(pid);

		if (itm != players_in_map.end()) {
			itm->second->posX = posX;
			itm->second->posY = posY;
		}

		auto& itr = players_in_range.find(pid);

		if (itr != players_in_range.end()) {
			if (IsInRange(posX, posY)) {
				itr->second->posX = posX;
				itr->second->posY = posY;
			}
			else {
				RemoveRangePlayer(itr);
			}
		}
		else {
			if (IsInRange(posX, posY)) {

				int itp_faction = 0;
				auto itp = players_in_map.find(pid);
				if (itp != players_in_map.end()) {
					itp_faction = itp->second->faction;
				}

				PlayerRange* pr = new PlayerRange();

				pr->ID = pid;
				pr->posX = posX;
				pr->posY = posY;
				pr->faction = itp_faction;

				players_in_range.emplace(pid, pr);
			}
		}
	}
}

VOID Intercept_CR(BSTR packet) {

	if (StartsWith(packet, L"CRA")) return;

	std::vector<std::string> pinfo = pm::split(pm::ConvertBSTRToString(packet).substr(2), ',');

	//if (pinfo.size() != 6) return;
	if (stoi(pinfo[1]) > 0) return;

	int nid = stoi(pinfo[3]);
	int posX = stoi(pinfo[4]);
	int posY = stoi(pinfo[5]);

	AddNpch(nid, posX, posY);
}

VOID UpdateNpcPos(const int& nid, const int& posX, const int& posY) {

	auto itr = npcsh_in_range.find(nid);
	//auto itm = npcsh_in_map.find(nid);

	//if (itm != npcsh_in_map.end() && itm->second->body != 154) {
	if (itr != npcsh_in_range.end()) {
		if (IsInRange(posX, posY))
			itr->second = std::make_pair(posX, posY);
		else
			npcsh_in_range.erase(nid);
	}
	else {
		if (IsInRange(posX, posY)) {
			npcsh_in_range.emplace(nid, std::make_pair(posX, posY));
		}
	}
	//}
}

VOID UpdatePlayerPos(const int& pid, const int& posX, const int& posY) {

	int faction = 0;
	auto itm = players_in_map.find(pid);

	if (itm != players_in_map.end()) {
		itm->second->posX = posX;
		itm->second->posY = posY;
		faction = itm->second->faction;
	}

	auto itr = players_in_range.find(pid);

	if (itr != players_in_range.end()) {
		if (IsInRange(posX, posY)) {
			itr->second->posX = posX;
			itr->second->posY = posY;
		}
		else {
			RemoveRangePlayer(itr);
		}
	}
	else {
		if (IsInRange(posX, posY)) {
			AddPlayerRange(pid, posX, posY, faction);
		}
	}
}

VOID Intercept_MP(BSTR packet) {

	std::vector<std::string> pinfo = pm::split(pm::ConvertBSTRToString(packet).substr(2), ',');

	int id = stoi(pinfo[0]);
	int posX = stoi(pinfo[1]);
	int posY = stoi(pinfo[2]);

	if (pinfo.size() == 3) {
		UpdateNpcPos(id, posX, posY);
	}
	else {
		UpdatePlayerPos(id, posX, posY);
	}
}

VOID Intercept_npch_CR_MP(BSTR packet, const bool& mp_packet,
	const int& id_index, const int& pos_x_index, const int& pos_y_index) {

	std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(packet), ',');

	if (splitVector.size() != 0) {

		int nid = stoi((mp_packet) ? splitVector[id_index].substr(2) : splitVector[id_index]);
		int posX = stoi(splitVector[pos_x_index]);
		int posY = stoi(splitVector[pos_y_index]);

		if (StartsWith(packet, L"CR")) {
			//	AddNpch(nid, posX, posY);
		}
		else {

			auto itr = npcsh_in_range.find(nid);

			if (itr != npcsh_in_range.end()) {
				if (IsInRange(posX, posY))
					itr->second = std::make_pair(posX, posY);
				else
					npcsh_in_range.erase(nid);
			}
			else {
				if (IsInRange(posX, posY))
					npcsh_in_range.emplace(nid, std::make_pair(posX, posY));
			}
		}
	}
}

VOID AddNpch(const int& nid, const int& posX, const int& posY) {

	auto itn = npcsh_in_map.find(nid);

	if (itn == npcsh_in_map.end()) {

		Npch* npc = new Npch();
		npc->id = nid;
		npc->posX = posX;
		npc->posY = posY;

		npcsh_in_map.emplace(nid, npc);

		if (IsInRange(posX, posY)) {
			npcsh_in_range.emplace(nid, std::make_pair(posX, posY));
		}
	}
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

VOID SendToClient(const std::string& message)
{
	try
	{
		//
		//// Create Packet
		//
		BSTR recvPacket = pm::ConvertStringToBSTR(message);
		//OutputDebugStringW(ConvertStringToBSTR("SendToClient: " + ConvertBSTRToString(recvPacket)));
		//
		//// Send Packet
		//
		//writelrFile(recvPacket);

		if (writeLogs) pm::writeLog(pm::ConvertBSTRToString(recvPacket), pm::DlibLogType::LOCAL_RECV);

		PFunctionRecv(recvPacket);
		//
		//// Free Packet
		//
		SysFreeString(recvPacket);
	}
	catch (int e)
	{
	}
}

VOID SendToClient(BSTR recvPacket)
{
	try
	{
		//
		//// Create Packet
		//
		//OutputDebugStringW(ConvertStringToBSTR("SendToClient: " + ConvertBSTRToString(recvPacket)));
		//
		//// Send Packet
		//
		//writelrFile(recvPacket);

		if (writeLogs) pm::writeLog(pm::ConvertBSTRToString(recvPacket), pm::DlibLogType::LOCAL_RECV);

		PFunctionRecv(recvPacket);
		//
		//// Free Packet
		//
		SysFreeString(recvPacket);
	}
	catch (int e)
	{
	}
}

VOID SendToServer(const std::string& message)
{
	try
	{
		//
		//// Create Packet
		//
		BSTR convertedSend = pm::ConvertStringToBSTR(message);
		//OutputDebugStringW(ConvertStringToBSTR("SendToServer: " + ConvertBSTRToString(convertedSend)));
		//
		//// Send Packet
		//
		PFunctionSend(&convertedSend);
		//
		//// Free Packet
		//
		SysFreeString(convertedSend);
	}
	catch (int e)
	{
	}
}

/*5, 21 = neutral
	2 = ciuda
	3 = crimi
	1 = gm*/

VOID AddPlayer(const std::vector<std::string>& splitVector) {

	int player_id = stoi(splitVector[3]);
	std::string player_name = splitVector[11];
	int posX = stoi(splitVector[4]);
	int posY = stoi(splitVector[5]);
	int faction = stoi(splitVector[12]);
	bool isInvi = stoi(splitVector[13]) == 1;

	if (player_name == user_name) {
		user_pos_x = posX;
		user_pos_y = posY;
		user_faction = faction;
		user_id = player_id;
	}
	else {

		auto it = players_in_map.find(player_id);

		bool inviDetected = !hideCheating && isInvi;

		if (it == players_in_map.end()) {
			Player* player = new Player();

			player->id = player_id;
			player->posX = posX;
			player->posY = posY;
			player->name = player_name;
			player->orgName = player_name;
			player->isInvisible = isInvi;
			player->faction = faction;

			player->inf0 = stoi(splitVector[0]);
			player->inf1 = stoi(splitVector[1]);
			player->inf2 = stoi(splitVector[2]);

			player->inf6 = stoi(splitVector[6]);
			player->inf7 = stoi(splitVector[7]);
			player->inf8 = stoi(splitVector[8]);
			player->inf9 = stoi(splitVector[9]);
			player->inf10 = stoi(splitVector[10]);

			player->inf14 = stoi(splitVector[14]);
			player->inf15 = stoi(splitVector[15]);

			player->inviDetected = inviDetected;

			players_in_map.emplace(player_id, player);
		}
		else {
			it->second->inviDetected = inviDetected;
		}

		//pm::writeLog("players_in_map size >" + std::to_string(players_in_map.size()), pm::DlibLogType::DLIB);
	}
}

VOID AddPlayerRange(const int& pid, const int& posX, const int& posY, const int& faction) {
	PlayerRange* pr = new PlayerRange();

	pr->ID = pid;
	pr->posX = posX;
	pr->posY = posY;
	pr->faction = faction;

	players_in_range.emplace(pid, pr);
}

VOID CleanupMapPlayers() {
	for (auto& player : players_in_map) {
		if (player.second) {
			delete player.second;  // Delete each Player object
			player.second = nullptr;
		}// Delete each Player object
	}
	players_in_map.clear();  // Clear the map after deleting the objects
}

VOID CleanupRangePlayers() {
	for (auto& player : players_in_range) {
		if (player.second) {
			delete player.second;  // Delete each Player object
			player.second = nullptr;
		}
	}
	players_in_range.clear();  // Clear the map after deleting the objects
}

VOID RemoveMapPlayer(const int& pid) {
	auto itp = players_in_map.find(pid);

	if (itp != players_in_map.end()) {

		if (itp->second) {
			delete itp->second;
			itp->second = nullptr;
		}

		players_in_map.erase(itp);
	}

	//pm::writeLog("players_in_map size >" + std::to_string(players_in_map.size()), pm::DlibLogType::DLIB);
}

VOID RemoveRangePlayer(std::unordered_map<int, PlayerRange*>::iterator it) {
	if (it->second) {
		delete it->second;
		it->second = nullptr;
	}
	players_in_range.erase(it);
}

VOID RemoveRangePlayer(const int& pid) {
	auto itr = players_in_range.find(pid);

	if (itr != players_in_range.end()) {
		if (itr->second) {
			delete itr->second;
			itr->second = nullptr;
		}
		players_in_range.erase(itr);
	}

	//pm::writeLog("players_in_range size >" + std::to_string(players_in_range.size()), pm::DlibLogType::DLIB);
}

VOID CleanupMapNpcs() {
	for (auto& npc : npcsh_in_map) {
		if (npc.second) {
			delete npc.second;  // Delete each Player object
			npc.second = nullptr;
		}
	}
	npcsh_in_map.clear();  // Clear the map after deleting the objects
}

VOID CleanupRangeNpcs() {
	npcsh_in_range.clear();  // Clear the map after deleting the objects
}

VOID RemoveMapNpc(const int& nid) {
	auto itn = npcsh_in_map.find(nid);

	if (itn != npcsh_in_map.end()) {
		if (itn->second) {
			delete itn->second;
			itn->second = nullptr;
		}
		npcsh_in_map.erase(itn);
	}
}

VOID RemoveRangeNpc(const int& nid) {
	npcsh_in_range.erase(nid);
}

BOOL IsInRange(const int& posX, const int& posY) {
	bool xrange = std::abs(posX - user_pos_x) <= 11;
	bool yrange = std::abs(posY - user_pos_y) <= 9;
	return xrange && yrange;
}

bool IsWhiteSpell() {
	return selected_lh > 0 &&
		(selected_lh == stoi(pos_remove) ||
			selected_lh == stoi(pos_invi) ||
			selected_lh == stoi(pos_cele) ||
			selected_lh == stoi(pos_fue));
	//||
//	selected_lh == stoi(pos_curv);
}

VOID SetManualTarget(const int& posX, const int& posY) {

	for (const auto& pr : players_in_range) {
		if (pr.second->posX == posX && pr.second->posY == posY) {
			selected_player_id = pr.first;
			return;
		}
	}

	for (const auto& nr : npcsh_in_range) {
		if (nr.second.first == posX && nr.second.second == posY) {
			selected_npch_id = nr.first;
			break;
		}
	}
}

std::tuple<int, int> GetManualTargetPos() {
	auto itp = players_in_range.find(selected_player_id);
	if (itp != players_in_range.end()) {
		return { itp->second->posX, itp->second->posY };
	}
	else {
		return { -1, -1 };
	}
}

std::tuple<int, int> GetUserTargetPos() {
	if (user_pos_x > 0 && user_pos_y > 0) {
		return { user_pos_x, user_pos_y };
	}
	else {
		return { -1, -1 };
	}
}

//std::tuple<int, int> GetClosestTargetPos(const int& posX, const int& posY) {
//
//	int closestX = -1, closestY = -1;
//	double minDistance = 12;
//
//	auto updateClosest = [&](int targetX, int targetY) {
//		double distance = std::sqrt(std::pow(posX - targetX, 2) + std::pow(posY - targetY, 2));
//		if (distance < minDistance) {
//			minDistance = distance;
//			closestX = targetX;
//			closestY = targetY;
//		}
//	};
//
//	for (const auto& pr : players_in_range) {
//		if (!pr.second) continue;
//		//if (pr.second->faction == user_faction && !IsWhiteSpell())
//		//	continue;
//
//		updateClosest(pr.second->posX, pr.second->posY);
//	}
//
//	if (closestX > -1 && closestY > -1)
//		return { closestX, closestY };
//	
//	for (const auto& nr : npcsh_in_range) {
//		
//		updateClosest(nr.second.first, nr.second.second);
//	}
//
//	// If no NPCs are found, return the current position
//	if (closestX == -1 || closestY == -1) {
//		return { posX, posY };
//	}
//
//	return { closestX, closestY };
//}

std::tuple<int, int> GetClosestTargetPos(const int& posX, const int& posY) {

	int closestX = -1, closestY = -1;

	//const double maxDistance = 8;
	double minDistance = 8;

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

	if (closestX > -1 && closestY > -1)
		return { closestX, closestY };

	for (const auto& nr : npcsh_in_range) {
		updateClosest(nr.second.first, nr.second.second);
	}

	if (closestX == -1 || closestY == -1) {
		return { posX, posY };
	}

	return { closestX, closestY };
}

void CheckNewTargets() {
	// Process players in the map
	for (const auto& pm : players_in_map) {
		int playerId = pm.second->id;
		auto itp = players_in_range.find(playerId);
		bool inRange = IsInRange(pm.second->posX, pm.second->posY);

		if (inRange) {
			if (itp == players_in_range.end()) {
				// Add player to the range if not already present
				PlayerRange* pr = new PlayerRange();
				pr->ID = playerId;
				pr->posX = pm.second->posX;
				pr->posY = pm.second->posY;
				pr->faction = pm.second->faction;
				players_in_range.emplace(playerId, std::move(pr));
			}
		}

		//	else if (itp != players_in_range.end()) {
				// Remove player from the range
		//		RemoveRangePlayer(itp);
		//	}
	}

	// Process NPCs in the map
	for (const auto& nm : npcsh_in_map) {
		int npcId = nm.second->id;
		auto itn = npcsh_in_range.find(npcId);
		bool inRange = IsInRange(nm.second->posX, nm.second->posY);

		if (inRange) {
			if (itn == npcsh_in_range.end()) {
				// Add NPC to the range if not already present
				npcsh_in_range.emplace(npcId, std::make_pair(nm.second->posX, nm.second->posY));
			}
		}
		//else if (itn != npcsh_in_range.end()) {
			// Remove NPC from the range
		//	RemoveRangeNpc(npcId);
		//}
	}
}

//VOID CheckNewTargets() {
//
//	for (const auto& pm : players_in_map) {
//		auto itp = players_in_range.find(pm.second->id);
//		bool inRange = IsInRange(pm.second->posX, pm.second->posY);
//
//		if (inRange && itp == players_in_range.end())
//		{
//			PlayerRange* pr = new PlayerRange();
//
//			pr->ID = pm.second->id;
//			pr->posX = pm.second->posX;
//			pr->posY = pm.second->posY;
//			pr->faction = pm.second->faction;
//
//			players_in_range.emplace(pm.second->id, pr);
//		}
//		else if (!inRange && itp != players_in_range.end()) {
//			//RemoveRangePlayer(pm.second->id);
//			RemoveRangePlayer(itp);
//		}
//	}
//
//	for (const auto& nm : npcsh_in_map) {
//		
//		auto itn = npcsh_in_range.find(nm.second->id);
//		bool inRange = IsInRange(nm.second->posX, nm.second->posY);
//
//		if (inRange && itn == npcsh_in_range.end())
//		{
//			auto xy = std::make_pair(nm.second->posX, nm.second->posY);
//			npcsh_in_range.emplace(nm.second->id, xy);
//		}
//		else if (!inRange && itn != npcsh_in_range.end()) {
//			RemoveRangeNpc(nm.second->id);
//		}
//	}
//}

std::tuple<std::string, std::string> ReadSpellsInfo(BSTR spell_packet) {
	// Convert BSTR to std::wstring and then to std::string
	std::wstring wSpellPacket = spell_packet;
	std::string spellPacket = std::string(wSpellPacket.begin(), wSpellPacket.end());

	// Remove the first 3 characters
	if (spellPacket.length() > 3) {
		spellPacket = spellPacket.substr(3);
	}

	// Find the first delimiter 'C'
	size_t firstC = spellPacket.find('C');
	std::string firstPart = hexToString(spellPacket.substr(0, firstC));

	// Remove last character from firstPart (if needed)
	if (!firstPart.empty()) {
		firstPart.pop_back();  // This is the equivalent of substr(0, length - 1)
	}

	// Find the second delimiter 'C'
	size_t secondC = spellPacket.find('C', firstC + 1);
	std::string secondPart = hexToString(spellPacket.substr(firstC + 1, secondC - firstC - 1));

	// Remove last character from secondPart (if needed)
	if (!secondPart.empty()) {
		secondPart.pop_back();  // Same as substr(0, length - 1)
	}

	// Convert the remaining part to the third part
	std::string thirdPart = hexToString(spellPacket.substr(secondC + 1));

	// Return as a tuple
	return std::make_tuple(firstPart, thirdPart);
}

//std::tuple<int, int> ReadXY_stoi(BSTR bstr_xy_packet) {
//
//	std::wstring convertedRecv = bstr_xy_packet;
//	string paquete_hex_org(convertedRecv.begin(), convertedRecv.end());
//
//	std::string paquete_hex = paquete_hex_org.substr(2, paquete_hex_org.length());
//
//	size_t paquete_hex_0_len = paquete_hex.find('C', 1);
//	std::string paquete_hex_0 = hexToString(paquete_hex.substr(0, paquete_hex_0_len));
//	paquete_hex_0 = paquete_hex_0.substr(0, paquete_hex_0.length() - 1);
//
//	size_t paquete_hex_1_len = paquete_hex.find('C', paquete_hex_0_len + 1);
//	std::string paquete_hex_1 = hexToString(paquete_hex.substr(paquete_hex_0_len + 1, paquete_hex_1_len - paquete_hex_0_len - 1));
//	paquete_hex_1 = paquete_hex_1.substr(0, paquete_hex_1.length());
//
//	int posY = stoi(paquete_hex_1);
//	int posX = stoi(paquete_hex_0);
//
//	return std::make_tuple(posX, posY);
//}

std::tuple<int, int> ReadXY_stoi(BSTR bstr_xy_packet) {
	// Convert BSTR to std::wstring
	std::wstring convertedRecv(bstr_xy_packet);

	// Remove the first 2 characters (assuming it's a prefix)
	std::wstring paquete_hex = convertedRecv.substr(2);

	// Find the position of 'C' that separates the two parts
	size_t paquete_hex_0_len = paquete_hex.find(L'C');

	// Convert wstring to string for the first part (paquete_hex_0)
	std::string paquete_hex_0(paquete_hex.begin(), paquete_hex.begin() + paquete_hex_0_len);

	// Convert wstring to string for the second part (paquete_hex_1)
	std::string paquete_hex_1(paquete_hex.begin() + paquete_hex_0_len + 1, paquete_hex.end());

	// Convert the hex strings to regular strings
	std::string hex_str_0 = hexToString(paquete_hex_0);
	std::string hex_str_1 = hexToString(paquete_hex_1);

	// Convert hex strings to integers
	int posX = std::stoi(hex_str_0);
	int posY = std::stoi(hex_str_1);

	return std::make_tuple(posX, posY);
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

size_t splitedPacketLen(BSTR packet) {

	std::wstring convertedRecv = packet;
	std::string str(convertedRecv.begin(), convertedRecv.end());
	std::vector<string> splitVector = pm::split(str, ',');

	return splitVector.size();
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





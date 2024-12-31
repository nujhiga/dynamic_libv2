#include "stdafx.h"
#include "WindowHandler.h"

#include "PacketManager.h"
namespace pm = PacketManager;

#pragma comment(lib, "comsuppw.lib")

DWORD oldTTeclas = GetTickCount();
//DWORD ttAutoPot = GetTickCount();

HANDLE handle_caster;
HANDLE handle_radar;
HANDLE handle_dylib;

bool is_code_running = false;
bool print_packets = true;

int cast_mode = 0;

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

	if (StartsWith(dataRecv, L"LH")) {
		uk_flag = false;
		lh_flag = false;
		auto_cast_flag = false;
	}

	if (StartsWith(dataRecv, L"MEDOK")) {
		user_meditando = !user_meditando;
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

				if (player_invisible == 1) {

					player->second->name += " [I]";
					player->second->isInvisible = false;

					SendToClient(pm::build_bp_packet(player_id));
					SendToClient(pm::build_cc_packet(player));
					SysFreeString(dataRecv);
					dataRecv = pm::build_v3_packet(player_info);
				}
				else {
					player->second->name = player->second->orgName;

					SendToClient(pm::build_bp_packet(player_id));
					SendToClient(pm::build_cc_packet(player));
				}
			}
		}
	}

	if (StartsWith(dataRecv, L"EST")) {

		//packet_str = ConvertBSTRToString(dataRecv).substr(3);
		//writePacketLog(packet_str, DLIB);
		//writePacketLog("EST dec> " + decrypt_packet(packet_str), DLIB);
	}
	
	if (StartsWith(dataRecv, L"BP")) {

		std::string packet_str = pm::ConvertBSTRToString(dataRecv).substr(2);

		int player_id = stoi(packet_str);

		RemoveRangePlayer(player_id);
		RemoveMapPlayer(player_id);
	}

	if (StartsWith(dataRecv, L"CM")) {
		MapChanged();
	}

	if (StartsWith(dataRecv, L"CC")) {

		std::vector<std::string> player_info = pm::packet_split(dataRecv, 2);
		AddPlayer(player_info);

		int player_invisible = stoi(player_info[13]);
		std::string player_name = player_info[11];

		if (player_name != user_name) {
			if (player_invisible == 1) {

				player_info[11] += " [I]";
				player_info[13] = "0";

				SysFreeString(dataRecv);
				dataRecv = pm::build_cc_packet(player_info);
			}
		}
	}

	if (StartsWith(dataRecv, L"CR")) {
		if (splitedPacketLen(dataRecv) > 3) {
			Intercept_npch_CR_MP(dataRecv, false, 3, 4, 5);
		}
	}

	if (StartsWith(dataRecv, L"MP")) {
		if (splitedPacketLen(dataRecv) == 3) {
			Intercept_npch_CR_MP(dataRecv, true, 0, 1, 2);
		}
		else {
			Intercept_player_MP(dataRecv);

		}
	}

	if (wcsstr(dataRecv, L"P9") != NULL) user_paralized = true;
	if (wcsstr(dataRecv, L"P8") != NULL) user_paralized = false;

	if (StartsWith(dataRecv, L"PU"))
	{
		auto xy = ReadXY_stoi(dataRecv);
		user_pos_x = std::get<0>(xy);
		user_pos_y = std::get<1>(xy);
	}

	if (StartsWith(dataRecv, L"SHS")) InterceptSHS(dataRecv);

	pm::writePacketLog(pm::ConvertBSTRToString(dataRecv), pm::DlibLogType::RECV);

	PFunctionRecv(dataRecv);

	__asm POPFD;
	__asm POPAD;
}

VOID WINAPI MySendData(BSTR* dataSend)
{
	__asm PUSHAD;
	__asm PUSHFD;

	//string packet_str;

	if (StartsWith(*dataSend, L"WLC")) {

		std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(*dataSend).substr(3), ',');

		int posX = 0, posY = 0;
		posX = stoi(splitVector[0]);
		posY = stoi(splitVector[1]);

		if (splitVector[2] == "1") {
			if (uk_flag && lh_flag) {
				int _posX = -1, _posY = -1;
				std::tuple<int, int> xy;

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
			}
			else {
				SysFreeString(*dataSend);
				*dataSend = pm::build_lac_packet(user_pos_x, user_pos_y);
			}
		}
	}

	if (StartsWith(*dataSend, L"UK1")) {
		uk_flag = true;
	}

	if (StartsWith(*dataSend, L"LH")) {
		std::string packet_str = pm::ConvertBSTRToString(*dataSend);
		selected_lh = stoi(packet_str.substr(2));
		lh_flag = true;
	}

	if (StartsWith(*dataSend, L"LC")) {
		if (cast_mode == 1) {
			std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(*dataSend), ',');
			SetManualTarget(stoi(splitVector[0].substr(2)), stoi(splitVector[1]));
		}
	}

	if (wcsstr(*dataSend, L"M1") != NULL) user_pos_y--;
	if (wcsstr(*dataSend, L"M2") != NULL) user_pos_x++;
	if (wcsstr(*dataSend, L"M3") != NULL) user_pos_y++;
	if (wcsstr(*dataSend, L"M4") != NULL) user_pos_x--;

	pm::writePacketLog(pm::ConvertBSTRToString(*dataSend), pm::DlibLogType::SEND);

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
			if (!is_code_running) {
				handle_caster = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CasterThread, 0, 0, 0);
				is_code_running = true;
			}
			else {
				TerminateThread(handle_caster, 0);
				is_code_running = false;
			}
		}

		oldTTeclas = GetTickCount();

		if (packets.size() != 0) {

			for (int i = 0; i < packets.size(); ++i)
				SendToServer(packets[i]);

			packets.clear();

			uk_flag = false;
			lh_flag = false;
			auto_cast_flag = false;
		}
	}

	PFunctionLoop();

	__asm POPFD;
	__asm POPAD;
}

//static VOID CastSpell(CastSpellType stype) {
//
//	//if (!spell_dardo_magico_flag) return;
//	std::string vlc = "empty";
//	writePacketLog("hit CastSpell", DLIB);
//
//	switch (stype)
//	{
//	case SELF:
//		writePacketLog("self " + std::to_string(user_pos_x) + "," + std::to_string(user_pos_y - 1), DLIB);
//
//		vlc = "WLC" + std::to_string(user_pos_x) + "," + std::to_string(user_pos_y - 1) + ",1";
//		break;
//	case NPC:
//		break;
//	case USER:
//		break;
//	case TEAM_MATE:
//		break;
//	default:
//		break;
//	}
//
//	if (stype == SELF)
//	{
//	/*	writePacketLog("self " + std::to_string(user_pos_x) + "," + std::to_string(user_pos_y - 1), DLIB);
//
//		vlc = "WLC" + std::to_string(user_pos_x) + "," + std::to_string(user_pos_y - 1) + ",1";*/
//	}
//	else if (stype == TEAM_MATE && team_mate_name != "") {
//		std::string posX, posY;
//
//		for (auto& player : players_in_range) {
//
//			if (player.second->name == team_mate_name) {
//				posX = std::to_string(player.second->posX);
//				posY = std::to_string(player.second->posY);
//				break;
//			}
//		}
//
//		vlc = "WLC" + posX + "," + posY + ",1";
//	}
//	else if (stype == NPC && selected_npch_id > 0)
//	{
//		auto it = npcsh_in_range.find(selected_npch_id);
//
//		if (it != npcsh_in_range.end()) {
//
//			Npch* target = it->second;
//			vlc = "WLC" + std::to_string(target->posX) + "," + std::to_string(target->posY) + ",1";
//		}
//	}
//	else if (stype == USER && selected_player_id > 0) {
//
//		auto it = players_in_range.find(selected_player_id);
//
//		if (it != players_in_range.end()) {
//			Player* target = it->second;
//			vlc = "WLC" + std::to_string(target->posX) + "," + std::to_string(target->posY) + ",1";
//		}
//	}
//
//	writePacketLog(vlc, DLIB);
//
//	if (vlc != "empty") packets.push_back(vlc);
//
//	uk_flag = false;
//	lh_flag = false;
//}

VOID CasterThread() {

	while (1) {

		if ((GetKeyState(VK_END) & 0x100) != 0 && (uk_flag && lh_flag)) {
			selected_player_id = 0;
			selected_npch_id = 0;
		}

		if ((GetKeyState(VK_DELETE) & 0x100) != 0) {
			if (cast_mode == 0)
				cast_mode = 1;
			else
				cast_mode = 0;
		}

		if ((GetKeyState(VK_XBUTTON2) & 0x100) != 0 && !auto_cast_flag) {
			auto_cast_flag = !auto_cast_flag;
		}

		if ((GetKeyState(VK_XBUTTON1) & 0x100) != 0) {
			packets.push_back(del_spell_words);
			Sleep(375);
		}

		Sleep(loop_caster_delay);
	}
}

VOID MapChanged() {
	CleanupMapNpcs();
	CleanupRangeNpcs();

	CleanupMapPlayers();
	CleanupRangePlayers();

	selected_npch_id = 0;
	selected_player_id = 0;

	uk_flag = false;
	lh_flag = false;
}

VOID InterceptSHS(BSTR packet) {
	auto spell_info = ReadSpellsInfo(packet);

	std::string spell_pos = std::get<0>(spell_info);
	std::string spell_name = std::get<1>(spell_info);

	//writePacketLog(spell_name + ", " + spell_pos, DLIB);

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
}

VOID Intercept_player_MP(BSTR packet) {

	std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(packet), ',');

	if (splitVector.size() != 0)
	{
		int pid = stoi(splitVector[0].substr(2));
		int posX = stoi(splitVector[1]);
		int posY = stoi(splitVector[2]);

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

VOID Intercept_npch_CR_MP(BSTR packet, const bool& mp_packet,
	const int& id_index, const int& pos_x_index, const int& pos_y_index) {

	std::vector<string> splitVector = pm::split(pm::ConvertBSTRToString(packet), ',');

	if (splitVector.size() != 0) {

		int nid = stoi((mp_packet) ? splitVector[id_index].substr(2) : splitVector[id_index]);
		int posX = stoi(splitVector[pos_x_index]);
		int posY = stoi(splitVector[pos_y_index]);

		if (StartsWith(packet, L"CR")) {
			AddNpch(nid, posX, posY);
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

		pm::writePacketLog(pm::ConvertBSTRToString(recvPacket), pm::DlibLogType::LOCAL_RECV);

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

		pm::writePacketLog(pm::ConvertBSTRToString(recvPacket), pm::DlibLogType::LOCAL_RECV);

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

	if (player_name == user_name) {
		user_pos_x = posX;
		user_pos_y = posY;
		user_faction = faction;
		user_id = player_id;
	}
	else {

		auto it = players_in_map.find(player_id);

		if (it == players_in_map.end()) {

			Player* newPlayer = new Player();
			newPlayer->id = player_id;
			newPlayer->posX = posX;
			newPlayer->posY = posY;
			newPlayer->name = player_name;
			newPlayer->orgName = player_name;
			newPlayer->isInvisible = (stoi(splitVector[13]) == 1);
			newPlayer->faction = faction;

			newPlayer->inf0 = stoi(splitVector[0]);
			newPlayer->inf1 = stoi(splitVector[1]);
			newPlayer->inf2 = stoi(splitVector[2]);

			newPlayer->inf6 = stoi(splitVector[6]);
			newPlayer->inf7 = stoi(splitVector[7]);
			newPlayer->inf8 = stoi(splitVector[8]);
			newPlayer->inf9 = stoi(splitVector[9]);
			newPlayer->inf10 = stoi(splitVector[10]);

			newPlayer->inf14 = stoi(splitVector[14]);
			newPlayer->inf15 = stoi(splitVector[15]);

			players_in_map.emplace(player_id, newPlayer);
		}

		pm::writePacketLog("players_in_map size >" + std::to_string(players_in_map.size()), pm::DlibLogType::DLIB);

	}
}

VOID CleanupMapPlayers() {
	for (auto& player : players_in_map) {
		delete player.second;  // Delete each Player object
	}
	players_in_map.clear();  // Clear the map after deleting the objects
}

VOID CleanupRangePlayers() {
	for (auto& player : players_in_range) {
		delete player.second;  // Delete each Player object
	}
	players_in_range.clear();  // Clear the map after deleting the objects
}

VOID RemoveMapPlayer(const int& pid) {
	auto itp = players_in_map.find(pid);

	if (itp != players_in_map.end()) {
		delete itp->second;
		players_in_map.erase(itp);
	}

	pm::writePacketLog("players_in_map size >" + std::to_string(players_in_map.size()), pm::DlibLogType::DLIB);

}

VOID RemoveRangePlayer(std::unordered_map<int, PlayerRange*>::iterator it) {
	delete it->second;
	players_in_range.erase(it);
}

VOID RemoveRangePlayer(const int& pid) {
	auto itr = players_in_range.find(pid);

	if (itr != players_in_range.end()) {
		delete itr->second;
		players_in_range.erase(itr);
	}

	pm::writePacketLog("players_in_range size >" + std::to_string(players_in_range.size()), pm::DlibLogType::DLIB);
}

VOID CleanupMapNpcs() {
	for (auto& npc : npcsh_in_map) {
		delete npc.second;  // Delete each Player object
	}
	npcsh_in_map.clear();  // Clear the map after deleting the objects
}

VOID CleanupRangeNpcs() {
	npcsh_in_range.clear();  // Clear the map after deleting the objects
}

VOID RemoveMapNpc(const int& nid) {
	auto itn = npcsh_in_map.find(nid);

	if (itn != npcsh_in_map.end()) {
		delete itn->second;
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

BOOL IsWhiteSpell() {
	return selected_lh == stoi(pos_remove) ||
		selected_lh == stoi(pos_invi) ||
		selected_lh == stoi(pos_cele) ||
		selected_lh == stoi(pos_fue);
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

std::tuple<int, int> GetClosestTargetPos(const int& posX, const int& posY) {
	int closestX = -1, closestY = -1;
	double minDistance = 10;

	for (const auto& pr : players_in_range) {

		if (pr.second->faction == user_faction && !IsWhiteSpell())
			continue;

		int userX = pr.second->posX;
		int userY = pr.second->posY;

		// Calculate Euclidean distance
		double distance = std::sqrt(std::pow(posX - userX, 2) + std::pow(posY - userY, 2));

		// Update closest NPC if this one is closer
		if (distance < minDistance) {
			minDistance = distance;
			closestX = userX;
			closestY = userY;
		}
	}

	if (closestX > -1 && closestY > -1)
		return { closestX, closestY };

	for (const auto& nr : npcsh_in_range) {
		int npcX = nr.second.first;
		int npcY = nr.second.second;

		// Calculate Euclidean distance
		double distance = std::sqrt(std::pow(posX - npcX, 2) + std::pow(posY - npcY, 2));

		// Update closest NPC if this one is closer
		if (distance < minDistance) {
			minDistance = distance;
			closestX = npcX;
			closestY = npcY;
		}
	}

	// If no NPCs are found, return the current position
	if (closestX == -1 || closestY == -1) {
		return { posX, posY };
	}

	return { closestX, closestY };
}

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

std::tuple<int, int> ReadXY_stoi(BSTR bstr_xy_packet) {

	std::wstring convertedRecv = bstr_xy_packet;
	string paquete_hex_org(convertedRecv.begin(), convertedRecv.end());

	std::string paquete_hex = paquete_hex_org.substr(2, paquete_hex_org.length());

	size_t paquete_hex_0_len = paquete_hex.find('C', 1);
	std::string paquete_hex_0 = hexToString(paquete_hex.substr(0, paquete_hex_0_len));
	paquete_hex_0 = paquete_hex_0.substr(0, paquete_hex_0.length() - 1);

	size_t paquete_hex_1_len = paquete_hex.find('C', paquete_hex_0_len + 1);
	std::string paquete_hex_1 = hexToString(paquete_hex.substr(paquete_hex_0_len + 1, paquete_hex_1_len - paquete_hex_0_len - 1));
	paquete_hex_1 = paquete_hex_1.substr(0, paquete_hex_1.length());

	int posY = stoi(paquete_hex_1);
	int posX = stoi(paquete_hex_0);

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





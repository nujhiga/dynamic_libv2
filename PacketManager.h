#pragma once
#include "unordered_map"
#include <sstream>
#include "Player.h"
#include <string>
#include <fstream>
#include <comutil.h>


namespace PacketManager {

	enum class ConsoleMessageType {
		ON,
		OFF,
		WARNING,
		INFO
	};

	enum class DlibLogType {
		SEND,
		RECV,
		LOCAL_SEND,
		LOCAL_RECV,
		DLIB
	};

	BSTR build_cc_packet(std::unordered_map<int, Player*>::iterator player);
	BSTR build_cc_packet(const std::vector<std::string>& player_info);
	BSTR build_v3_packet(const std::vector<std::string>& packet_data);
	BSTR build_lac_packet(const int& posX, const int& posY);
	BSTR build_wlc_packet(const int& posX, const int& posY);
	BSTR build_qdl_packet(const int& pid);
	BSTR build_console_packet(const std::string& message, ConsoleMessageType mtype = ConsoleMessageType::INFO);
	BSTR build_bp_packet(const int& pid);
	std::vector<std::string> packet_split(BSTR packet, int subStrOffset = 0, int subStrCharCount = 0, char delim = ',');
	BSTR ConvertStringToBSTR(const std::string& str);
	std::string ConvertBSTRToString(BSTR bstr);
	std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
	std::string decrypt_packet(const std::string& message);
	void writePacketLog(const string& packet, DlibLogType ltype);
	std::vector<std::string> split(const std::string& s, char delim);
}
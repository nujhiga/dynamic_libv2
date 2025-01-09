#pragma once
#include "unordered_map"
#include <sstream>
#include <string>
#include <fstream>
#include <comutil.h>
#include <iostream>
#include <array>
#include <memory>
#include "Player.h"

namespace PacketManager {

	enum class ConsoleMessageType {
		ON,
		OFF,
		WARNING,
		INFO
	};

	enum class OPCode {
		CC,
		V3,
		BP,
		TW,
		LC,
		COUNT
	};

	constexpr std::array<const char*, static_cast<int>(OPCode::COUNT)>
		OPCodeNames = { "CC", "V3", "BP", "TW", "LC" };

	std::string OPCodeStr(const OPCode& ocode);

	std::string BuildPCK(const OPCode& ocode, const std::vector<int>& args);

	std::string build_CC(const std::shared_ptr<Player>& player);

	std::string build_CC(const std::vector<std::string>& player_info);

	std::string build_V3(const std::vector<std::string>& packet_data);

	std::string build_BP(int pid);

	std::string build_TW(int wav, int posX, int posY);

	std::string build_CMD(const std::string& command);

	std::string build_LC(int posX, int posY);
	std::string build_WLC(int posX, int posY, int wlct = 1);

	BSTR build_console_packet(const std::string& message, ConsoleMessageType mtype = ConsoleMessageType::INFO);

	std::vector<std::string> packet_split(BSTR packet, int subStrOffset = 0, int subStrCharCount = 0, char delim = ',');
	BSTR ConvertStringToBSTR(const std::string& str);
	std::string ConvertBSTRToString(BSTR bstr);
	std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
	std::string decrypt_packet(const std::string& message);
	std::vector<std::string> split(const std::string& s, char delim);
	std::pair<int, std::string> read_SHS(const std::string& packet);
	std::tuple<int, int> read_PU(const std::string& packet);
	std::string ConvertBSTRPacket(BSTR packet, int opCodeLength = 0);
}
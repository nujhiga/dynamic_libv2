#include "stdafx.h"
#include "PacketManager.h"
#include "Utils.h"

namespace PacketManager {

	std::string OPCodeStr(const OPCode& ocode) {
		int idx = static_cast<int>(ocode);

		return (idx >= 0 && idx < static_cast<int>(OPCode::COUNT)) ?
			OPCodeNames[idx] : "Unknown";
	}

	std::string BuildPCK(const OPCode& ocode, const std::vector<int>& args) {
		std::ostringstream packet;
		packet << OPCodeStr(ocode);

		for (size_t i = 0; i < args.size(); ++i) {
			if (i > 0) 	packet << ",";
			packet << args[i];
		}

		return packet.str();
	}

	std::string build_CC(const std::shared_ptr<Player>& player) {
		std::ostringstream packetStream;
		packetStream << "CC"
			<< player->body << ","
			<< player->head << ","
			<< player->heading << ","
			<< player->id << ","
			<< player->posX << ","
			<< player->posY << ","
			<< player->wanim << ","
			<< player->sanim << ","
			<< player->fx << ","
			<< player->unk1 << ","
			<< player->canim << ","
			<< player->name << ","
			<< player->bcr << ","
			<< player->invi << ","
			<< player->unk2 << ","
			<< player->unk3;

		return packetStream.str();
	}

	std::string build_CC(const std::vector<std::string>& player_info) {
		std::ostringstream packet;
		packet << "CC";

		for (size_t i = 0; i < player_info.size(); ++i) {
			if (i > 0) packet << ",";  // Add comma separator
			packet << player_info[i];
		}

		return packet.str();
	}

	std::string build_V3(const std::vector<std::string>& packet_data) {
		std::ostringstream packetStream;
		packetStream << "V3"
			<< packet_data[0] << ","
			<< packet_data[1] << ","
			<< packet_data[2] << ","
			<< packet_data[3] << ","
			<< packet_data[4];

		return packetStream.str();
	}

	std::string build_BP(int pid) {
		std::ostringstream packet;
		packet << "BP" << pid;
		return packet.str();
	}

	std::string build_LC(int posX, int posY) {
		std::ostringstream packet;
		packet << "LC" << posX << "," << posY;
		return packet.str();
	}

	std::string build_WLC(int posX, int posY, int wlct) {
		std::ostringstream packet;
		packet << "WLC" << posX << "," << posY << "," << wlct;
		return packet.str();
	}

	BSTR build_console_packet(const std::string& message, ConsoleMessageType mtype) {

		auto get_message_color = [](ConsoleMessageType mtype) {
			switch (mtype)
			{
				case ConsoleMessageType::ON:
					return "~0~255~0~1~0";
				case ConsoleMessageType::OFF:
					return "~255~0~0~1~0";
				case ConsoleMessageType::WARNING:
					return "~255~242~0~1~0";
				case ConsoleMessageType::INFO:
				default:
					return "~255~255~255~1~0";
			}
		};

		if (message.empty()) return nullptr;

		return ConvertStringToBSTR(std::string().append("||DLIB>> ").append(message).append(get_message_color(mtype)));
	}

	std::string build_TW(int wav, int posX, int posY) {
		std::ostringstream packet;
		packet << "TW" << wav << "," << posX << "," << posY;
		return packet.str();
	}

	std::string build_CMD(const std::string& command) {
		std::ostringstream packet;
		packet << "/" << command;
		return packet.str();
	}

	std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
	{
		int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

		std::string dblstr(len, '\0');
		::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, &dblstr[0], len, NULL, NULL);

		return dblstr;
	}

	std::string ConvertBSTRToString(BSTR bstr)
	{
		int wslen = ::SysStringLen(bstr);
		return ConvertWCSToMBS(bstr, wslen);
	}

	BSTR ConvertStringToBSTR(const std::string& str)
	{
		int wslen = ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
			str.data(), str.length(),
			NULL, 0);

		BSTR wsdata = ::SysAllocStringLen(NULL, wslen);
		::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
			str.data(), str.length(),
			wsdata, wslen);
		return wsdata;
	}

	std::vector<std::string> packet_split(BSTR packet, int subStrOffset, int subStrCharCount, char delim) {
		std::string packet_str = ConvertBSTRToString(packet);

		if (subStrOffset > 0) {
			packet_str = packet_str.substr(subStrOffset);
		}
		else if (subStrOffset > 0 && subStrCharCount > 0) {
			packet_str = packet_str.substr(subStrOffset, subStrCharCount);
		}

		return split(packet_str, delim);
	}

	std::string decrypt_packet(const std::string& message)
	{
		auto token1 = message[message.length() - 1] - 0xA;
		auto token2 = message[message.length() - 2] - 0xA;

		int key = stoi(std::string(1, static_cast<char>(token2)) + static_cast<char>(token1));

		std::string decryptedPacket;
		decryptedPacket.reserve(message.length() - 2);

		for (size_t i = 0; i < message.length() - 2; ++i)
			decryptedPacket += static_cast<char>(message[i] - key);

		return decryptedPacket;
	}

	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> elems;
		size_t start = 0;
		size_t end = 0;

		while ((end = s.find(delim, start)) != std::string::npos) {
			elems.push_back(s.substr(start, end - start));
			start = end + 1;
		}

		elems.push_back(s.substr(start));  // Add the last substring

		return elems;
	}

	std::pair<int, std::string> read_SHS(const std::string& packet) {

		size_t firstC = packet.find('C');
		std::string firstPart = hexToString(packet.substr(0, firstC));

		if (!firstPart.empty()) {
			firstPart.pop_back();
		}

		size_t secondC = packet.find('C', firstC + 1);
		std::string secondPart = hexToString(packet.substr(firstC + 1, secondC - firstC - 1));

		if (!secondPart.empty()) {
			secondPart.pop_back();
		}

		std::string thirdPart = hexToString(packet.substr(secondC + 1));
		return { stoi(firstPart),thirdPart };

		//return { thirdPart, stoi(firstPart) };
	}

	std::tuple<int, int> read_PU(const std::string& packet) {

		// Find the position of 'C' that separates the two parts
		size_t paquete_hex_0_len = packet.find(L'C');

		// Convert wstring to string for the first part (paquete_hex_0)
		std::string paquete_hex_0(packet.begin(), packet.begin() + paquete_hex_0_len);

		// Convert wstring to string for the second part (paquete_hex_1)
		std::string paquete_hex_1(packet.begin() + paquete_hex_0_len + 1, packet.end());

		// Convert the hex strings to regular strings
		std::string hex_str_0 = hexToString(paquete_hex_0);
		std::string hex_str_1 = hexToString(paquete_hex_1);

		// Convert hex strings to integers
		int posX = std::stoi(hex_str_0);
		int posY = std::stoi(hex_str_1);

		return std::make_tuple(posX, posY);
	}

	std::string ConvertBSTRPacket(BSTR packet, int opCodeLength) {
		std::string strPacket = ConvertBSTRToString(packet);

		if (opCodeLength > 0) {
			strPacket.erase(0, opCodeLength);
		}

		return strPacket;
	}

}
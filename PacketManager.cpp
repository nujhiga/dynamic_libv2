#include "stdafx.h"
#include "PacketManager.h"
#include "Utils.h"

namespace PacketManager {

	BSTR build_cc_packet(const std::pair<const int, Player*>& pair) {

		std::ostringstream packetStream;
		packetStream << "CC"
			<< pair.second->inf0 << ","
			<< pair.second->inf1 << ","
			<< pair.second->inf2 << ","
			<< pair.second->id << ","
			<< pair.second->posX << ","
			<< pair.second->posY << ","
			<< pair.second->inf6 << ","
			<< pair.second->inf7 << ","
			<< pair.second->inf8 << ","
			<< pair.second->inf9 << ","
			<< pair.second->inf10 << ","
			<< pair.second->name << ","
			<< pair.second->faction << ","
			<< pair.second->isInvisible << ","
			<< pair.second->inf14 << ","
			<< pair.second->inf15;

		return ConvertStringToBSTR(packetStream.str());

	}

	BSTR build_cc_packet(std::unordered_map<int, Player*>::iterator player) {
		std::ostringstream packetStream;
		packetStream << "CC"
			<< player->second->inf0 << ","
			<< player->second->inf1 << ","
			<< player->second->inf2 << ","
			<< player->second->id << ","
			<< player->second->posX << ","
			<< player->second->posY << ","
			<< player->second->inf6 << ","
			<< player->second->inf7 << ","
			<< player->second->inf8 << ","
			<< player->second->inf9 << ","
			<< player->second->inf10 << ","
			<< player->second->name << ","
			<< player->second->faction << ","
			<< player->second->isInvisible << ","
			<< player->second->inf14 << ","
			<< player->second->inf15;

		return ConvertStringToBSTR(packetStream.str());
	}

	BSTR build_cc_packet(const std::vector<std::string>& player_info) {
		std::ostringstream packet;
		packet << "CC";

		for (size_t i = 0; i < player_info.size(); ++i) {
			if (i > 0) packet << ",";  // Add comma separator
			packet << player_info[i];
		}

		// Convert to BSTR and return
		return ConvertStringToBSTR(packet.str());
	}

	BSTR build_v3_packet(const std::vector<std::string>& packet_data) {

		std::ostringstream packetStream;
		packetStream << "V3"
			<< packet_data[0] << ","
			<< packet_data[1] << ","
			<< packet_data[2] << ","
			<< packet_data[3] << ",0";

		return ConvertStringToBSTR(packetStream.str());
	}

	BSTR build_lac_packet(const int& posX, const int& posY) {
		std::ostringstream packet;
		packet << "LC" << std::to_string(posX) << "," << std::to_string(posY);
		return ConvertStringToBSTR(packet.str());
	}

	BSTR build_wlc_packet(const int& posX, const int& posY) {
		std::ostringstream packet;
		packet << "WLC" << posX << "," << posY << ",1";
		return ConvertStringToBSTR(packet.str());
	}

	BSTR build_qdl_packet(const int& pid) {
		std::ostringstream packet;
		packet << "QDL" << pid;
		return ConvertStringToBSTR(packet.str());
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

	BSTR build_bp_packet(const int& pid) {
		std::ostringstream packet;
		packet << "BP" << pid;
		return ConvertStringToBSTR(packet.str());
	}

	string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
	{
		int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

		std::string dblstr(len, '\0');
		::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, &dblstr[0], len, NULL, NULL);

		return dblstr;
	}

	string ConvertBSTRToString(BSTR bstr)
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

	void writeLog(const string& packet, DlibLogType ltype) {

		std::string packetType;

		switch (ltype)
		{
			case DlibLogType::SEND:
				packetType = "SEND > ";
				break;
			case DlibLogType::RECV:
				packetType = "RECV < ";
				break;
			case DlibLogType::LOCAL_SEND:
				packetType = "L_SEND > ";
				break;
			case DlibLogType::LOCAL_RECV:
				packetType = "L_RECV < ";
				break;
			case DlibLogType::DLIB:
				packetType = "DLYB > ";
				break;
			default:
				break;
		}

		//std::string filePath = Utils::GetDllDirectory() + "\\dlib_log.txt";
		std::string filePath = "C:\\Users\\joaco\\Documents\\dynamic_lib-master\\dlib_log.txt";

		std::ofstream file(filePath, std::ios_base::app);

		if (file.is_open()) {
			file << packetType << packet + "\n";
			file.close();
		}
	}

	string decrypt_packet(const std::string& message)
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
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim)) {
			elems.push_back(std::move(item));
		}

		return elems;
	}

}
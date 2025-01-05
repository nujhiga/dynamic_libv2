#include "DlibLogger.h"
#include "stdafx.h"
#include <iostream>

//public
DlibLogger::DlibLogger(const std::string& flName) :
	logFile(flName, std::ios::app),
	logging(true) {
	logThread = std::thread(&DlibLogger::processLogs, this);
	fileName = flName;
}

DlibLogger::~DlibLogger() {
	{
		std::lock_guard<std::mutex> lock(queueMtx);
		logging.store(false);
	}
	cond.notify_all();
	logThread.join();
}

void DlibLogger::startLogging() {
	if (!logging.load())
		logging.store(true);

	if (!logFile.is_open())
		logFile.open(fileName, std::ios::app);

	if (!logThread.joinable())
		logThread = std::thread(&DlibLogger::processLogs, this);
}

void DlibLogger::stopLogging() {
	logging.store(false);

	if (logThread.joinable())
		logThread.join();
}

void DlibLogger::logData(const std::string& data, const LogType& ltype) {
	std::lock_guard<std::mutex> lock(queueMtx);
	logQueue.push(LogTypeStr(ltype) + " " + data);
	cond.notify_one();
}

//private
const std::array<const char*, static_cast<int>(DlibLogger::LogType::COUNT)> DlibLogger::LogTypes = {
	"SEND >", "RECV <",	"LOCAL_SEND >", "LOCAL_RECV <",	"DLIB :", "!DLIB_EX :", "DEBUG :"
};

std::string DlibLogger::LogTypeStr(const DlibLogger::LogType& ltype) {
	int idx = static_cast<int>(ltype);
	return (idx >= 0 && idx < static_cast<int>(DlibLogger::LogType::COUNT)) ?
		DlibLogger::LogTypes[idx] : "Unknown";
}

void DlibLogger::processLogs() {
	while (logging.load()) {
		std::unique_lock<std::mutex> lock(queueMtx);
		cond.wait(lock, [this]() { return !logQueue.empty() || !logging.load(); });

		if (!logFile.is_open()) break;

		while (!logQueue.empty()) {
			logFile << logQueue.front() << "\n";
			logQueue.pop();
		}

		if (!logging.load() && logQueue.empty()) break;
	}

	if (logFile.is_open())
		logFile.close();
}
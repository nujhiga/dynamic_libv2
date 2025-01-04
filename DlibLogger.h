#pragma once

#include <fstream>
#include <queue>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <comutil.h>

class DlibLogger {
public:
	enum class LogType {
		SEND,
		RECV,
		LOCAL_SEND,
		LOCAL_RECV,
		DLIB,
		DLIB_EX,
		DEBUG,
		COUNT
	};

	DlibLogger(const std::string& flName);
	~DlibLogger();

	void startLogging();
	void stopLogging();

	void logData(const std::string& data, const LogType& ltype);

private:

	static const std::array<const char*, static_cast<int>(LogType::COUNT)> LogTypes;
	static std::string LogTypeStr(const LogType& ltype);

	std::string fileName;
	std::ofstream logFile;
	std::queue<std::string> logQueue;
	std::mutex queueMtx;
	std::condition_variable cond;
	std::atomic<bool> logging;
	std::thread logThread;

	void processLogs();
};

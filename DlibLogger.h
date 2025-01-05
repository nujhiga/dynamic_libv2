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

	template<typename... Args>
	void logDebug(const std::string& methodName, Args&&... args);

private:

	template<typename T>
	void formatArgs(std::ostringstream& oss, const std::string& varName, const T& value);

	template<typename T, typename... Args>
	void formatArgs(std::ostringstream& oss, const std::string& varName, const T& value, Args&&... args);

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

#include "DlibLogger.tpp"
#include "DlibLogger.h"

template<typename... Args>
void DlibLogger::logDebug(const std::string& methodName, Args&&... args) {
    std::ostringstream oss;
    oss << "Method: " << methodName << "\n";

    formatArgs(oss, std::forward<Args>(args)...);

    logData(oss.str(), DlibLogger::LogType::DEBUG);
}

template<typename T>
void DlibLogger::formatArgs(std::ostringstream& oss, const std::string& varName, const T& value) {
    oss << varName << " = " << value << "\n";
}

template<typename T, typename... Args>
void DlibLogger::formatArgs(std::ostringstream& oss, const std::string& varName, const T& value, Args&&... args) {
    formatArgs(oss, varName, value);
    formatArgs(oss, std::forward<Args>(args)...);
}


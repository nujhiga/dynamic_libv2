#include "RadarManager.h"

template<typename... Args>
std::wstring RadarManager::getWstring(Args&&... args) {
    std::wostringstream woss;
    formatArgs(woss, std::forward<Args>(args)...);
    return woss.str();
}


template<typename T>
void RadarManager::formatArgs(std::wostringstream& oss, const std::string& varName, const T& value) {
    oss << std::wstring(varName.begin(), varName.end()) << value;
}

template<typename T, typename... Args>
void RadarManager::formatArgs(std::wostringstream& oss, const std::string& varName, const T& value, Args&&... args) {
    formatArgs(oss, varName, value);
    formatArgs(oss, std::forward<Args>(args)...);
}
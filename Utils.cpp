#include "Utils.h"
#include "stdafx.h"

namespace Utils {

    std::string GetDllDirectory() {
        // Buffer to store the full path of the DLL
        char path[MAX_PATH];

        // Get the handle to the current module (DLL)
        HMODULE hModule = GetModuleHandle(NULL);  // NULL means the current executable or DLL

        if (hModule == NULL) {
       //     std::cerr << "Failed to get module handle." << std::endl;
            return ""; // If GetModuleHandle failed, return an empty string
        }

        // Get the full path to the DLL (or executable if NULL)
        DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
        if (len == 0) {
        //    std::cerr << "Failed to get the file name. Error: " << GetLastError() << std::endl;
            return ""; // If GetModuleFileNameA failed, return an empty string
        }

        // Print the full path to debug
        //std::cout << "Full path: " << path << std::endl;

        // Extract the directory from the full path
        std::string fullPath(path);
        size_t pos = fullPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            fullPath = fullPath.substr(0, pos); // Get directory part only
            return fullPath;
        }

       // std::cerr << "Failed to extract the directory from the path." << std::endl;
        return "C:\\";
    }



}

#pragma once

#include "registry.h"

bool ExportRegistryKey(const std::string& keyPath, const std::string& exportPath) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key. Error: " << result << std::endl;
        return false;
    }

    DWORD bufferSize = 0;
    result = RegSaveKeyA(hKey, exportPath.c_str(), NULL);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to save registry key. Error: " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool ImportRegistryKey(const std::string& importPath) {
    LONG result = RegRestoreKeyA(HKEY_CURRENT_USER, importPath.c_str(), REG_FORCE_RESTORE);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to restore registry key. Error: " << result << std::endl;
        return false;
    }
    return true;
}
#pragma once

#include <windows.h>
#include <iostream>
#include <fstream>

#define KEY_PATH "SOFTWARE/Microsoft/Windows/CurrentVersion/Run"
#define EXPORT_PATH "C:/MarauderMap"

bool ExportRegistryKey(const std::string& keyPath, const std::string& exportPath);

bool ImportRegistryKey(const std::string& importPath);
#pragma once

#include <windows.h>
#include <iostream>
#include <fstream>

#define KEY_PATH "Software/Microsoft/Windows/CurrentVersion/Run"
#define EXPORT_PATH "C:/RansomTestSuite/Registry/auto_run_backup.reg"

bool ExportRegistryKey(const std::string& keyPath, const std::string& exportPath);

bool ImportRegistryKey(const std::string& importPath);
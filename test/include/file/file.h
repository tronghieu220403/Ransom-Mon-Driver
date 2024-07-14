#pragma once
#include <windows.h>
#include <fltUser.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>
#include <queue>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_set>

#define PORT_NAME L"\\mf"
#define BUFFER_SIZE 1024

// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")
namespace fs = std::filesystem;
using namespace std;

bool VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);
wstring GetDosPath(const wstring* wstr);
string getRandomFile(const std::string& directoryPath);
string copyFile(const std::string& sourcePath, const std::string& destinationPath);
void copyDirectory(const fs::path& source, const fs::path& destination, long long total_size);
void moveFile(const fs::path& sourcePath, const fs::path& destinationDir);
std::string getFileNameWithoutExtension(const std::string& filePath);

struct COMPORT_MESSAGE
{
    FILTER_MESSAGE_HEADER header;
    WCHAR data[BUFFER_SIZE];
};


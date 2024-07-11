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

#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <ctime>

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
void copyDirectory(const fs::path& source, const fs::path& destination);


struct COMPORT_MESSAGE
{
    FILTER_MESSAGE_HEADER header;
    WCHAR data[BUFFER_SIZE];
};


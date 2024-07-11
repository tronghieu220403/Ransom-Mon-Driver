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

#define PORT_NAME L"\\mf"
#define BUFFER_SIZE 1024

// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")

using namespace std;

bool VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);

struct COMPORT_MESSAGE
{
    FILTER_MESSAGE_HEADER header;
    WCHAR data[BUFFER_SIZE];
};


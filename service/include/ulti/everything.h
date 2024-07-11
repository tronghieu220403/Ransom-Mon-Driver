#ifdef _VISUAL_STUDIO_WORKSPACE
#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef PROCESSMONITORING_ULTI_EVERYTHING_H_
#define PROCESSMONITORING_ULTI_EVERYTHING_H_

#ifndef _VISUAL_STUDIO_WORKSPACE
#define UNICODE
#define _UNICODE
#else
#pragma warning( disable : 4244)
#endif

#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <WinSock2.h>
    #include <iphlpapi.h>
	#include <WS2tcpip.h>
	#include <TlHelp32.h>
    #include <Psapi.h>
    #include <direct.h>
    #include <stdio.h>
    #include <conio.h>
    #include <strsafe.h>
    #include <tchar.h>
    #include <locale.h>
    #include <Windows.h>
    #include <Shlobj.h>
    #include <Pdh.h>
    #include <PdhMsg.h>
    #include <wmistr.h>
    #include <evntrace.h>
    #include <evntcons.h>
    #include <comutil.h>
    #include <wbemidl.h>
    #include <aclapi.h>    

    #include "debug.h"

#elif __linux__

	#include <sys/wait.h>
	#include <dirent.h>
    #include <unistd.h>
    #include <fcntl.h> 
    #include <sys/stat.h> 
    #include <sys/types.h> 
    #include <semaphore.h> 
    #include <errno.h>
#endif

#include <memory>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <thread>
#include <functional>
#include <chrono>
#include <deque>
#include <mutex>

#include <syncstream>
#include <filesystem>
#include <fstream>

#ifdef __linux__

#include "nlohmann/json.hpp"

#endif // __linux__

#ifdef _VISUAL_STUDIO_WORKSPACE
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Shell32.lib")  // For IsUserAnAdmin function
#pragma comment(lib, "comsupp.lib")  // For _bstr_t class
#pragma comment(lib,"Pdh.lib")
#pragma comment(lib,"Advapi32.lib")
#endif


#ifdef _WIN32
    #define ZeroMem(data,size) ZeroMemory(data,size)
#elif __linux__
    #define ZeroMem(data,size) memset(data, 0, size)
#endif

#ifdef _WIN32
    
#elif __linux__
    #define Sleep(ms) usleep(ms*1000)
#endif

#ifdef _WIN32
    #define GetCurrentDir _getcwd
#else
    #define GetCurrentDir getcwd
 #endif

namespace rm
{
    inline std::mutex mt;
    inline void WriteDebug(std::string s)
    {
    #ifdef _DEBUG
        mt.lock();
        std::ofstream outfile("C:\\debug.txt", std::ios_base::app);
        outfile << s << "\n";
        outfile.close();
        mt.unlock();
    #endif // DEBUG
    }

    inline std::string WStrToStr(const std::wstring& w_str)
    {
        return std::string(w_str.begin(), w_str.end());
    }

    inline std::wstring StrToWStr(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    inline std::string CharVectorToString(std::vector<char> v)
    {
        return std::string(v.begin(), v.end());
    }

    inline std::vector<char> StringToVectorChar(std::string s)
    {
        return std::vector<char>(s.begin(), s.end());
    }

    #ifdef __linux__
        inline std::string GetCurrentUserPath()
        {
            std::string path;
            path.resize(10000);
            int sz = readlink("/proc/self/exe", path.data(), path.size());
            path.resize(sz);
            int pos = path.find("/", path.find("/", path.find("/") + 1) + 1);
            return path.substr(0, pos);
        }

        inline bool CreateFolder(std::string path)
        {
            struct stat st;
            if(stat(path.data(),&st) != 0 || st.st_mode & S_IFDIR == 0)
            {
                if (mkdir(path.data(),0777) != 0)
                {
                    return false;
                }
            }
            return true;
        }
    #endif
}

#endif

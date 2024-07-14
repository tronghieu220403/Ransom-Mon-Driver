// Receiver.cpp
#include "named-pipe.h"
#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

bool runExe(const std::string& exePath) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    BOOL result = CreateProcessA(
        exePath.c_str(),
        NULL,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!result) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return false;
    }
    cout << pi.dwProcessId << endl;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int main() {
    NamedPipe pipe("\\\\.\\pipe\\MyPipe", false);

    while (true)
    {
        if (!pipe.connect()) {
            Sleep(1000);
            continue;
        }

        while (true)
        {
            std::string exePath;
            if (!pipe.receive(exePath)) {
                break;
            }

            cout << "Received: " << exePath << endl;

            bool success = runExe(exePath);
            if (!pipe.send(success ? "true" : "false")) {
                break;
            }
        }
    }
}

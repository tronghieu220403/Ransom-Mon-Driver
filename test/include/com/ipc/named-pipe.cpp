// NamedPipe.cpp
#include "named-pipe.h"

NamedPipe::NamedPipe(const std::string& pipeName, bool isServer)
    : pipeName(pipeName), hPipe(INVALID_HANDLE_VALUE), isServer(isServer) {}

NamedPipe::~NamedPipe() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }
}

bool NamedPipe::connect() {
    if (isServer) {
        hPipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096,
            4096,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe failed. Error: " << GetLastError() << std::endl;
            return false;
        }

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected) {
            std::cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << std::endl;
            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
            return false;
        }
    }
    else {
        while (true) {
            hPipe = CreateFileA(
                pipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (hPipe != INVALID_HANDLE_VALUE)
                break;

            if (GetLastError() != ERROR_PIPE_BUSY) {
                std::cerr << "Could not open pipe. Error: " << GetLastError() << std::endl;
                return false;
            }

            if (!WaitNamedPipeA(pipeName.c_str(), 20000)) {
                std::cerr << "Could not open pipe: 20 second wait timed out." << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool NamedPipe::send(const std::string& message) {
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(
        hPipe,
        message.c_str(),
        message.size() + 1, // include null-terminator
        &bytesWritten,
        NULL
    );

    if (!result) {
        std::cerr << "WriteFile failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool NamedPipe::receive(std::string& message) {
    char buffer[4096];
    DWORD bytesRead = 0;

    BOOL result = ReadFile(
        hPipe,
        buffer,
        sizeof(buffer),
        &bytesRead,
        NULL
    );

    if (!result || bytesRead == 0) {
        std::cerr << "ReadFile failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    message = std::string(buffer);
    return true;
}

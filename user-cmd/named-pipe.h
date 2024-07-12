#pragma once

#ifndef NAMEDPIPE_H
#define NAMEDPIPE_H

#include <windows.h>
#include <iostream>
#include <string>
#include <sddl.h>

using namespace std;

class NamedPipe {
public:
    NamedPipe(const std::string& pipeName, bool isServer);
    ~NamedPipe();
    bool connect();
    bool send(const std::string& message);
    bool receive(std::string& message);

private:
    std::string pipeName;
    HANDLE hPipe;
    bool isServer;
};

#endif // NAMEDPIPE_H

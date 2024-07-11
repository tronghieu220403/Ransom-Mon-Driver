#ifndef RANSOMTESTER_RANSOMTESTER
#define RANSOMTESTER_RANSOMTESTER

#include "include/service/tester-serv.h"
#include "include/service/servicecontrol.h"
#include "include/service/service.h"
#include "include/file/file.h"
#include "include/com/ioctl/ioctl.h"
#include "include/registry/registry.h"
#include "include/com/ipc/named-pipe.h"

LPVOID ServiceMainWorker()
{
    ExportRegistryKey(KEY_PATH, EXPORT_PATH);
    NamedPipe pipe("\\\\.\\pipe\\MyPipe", false);
    pipe.connect();
    Ioctl ioctl;
    ioctl.Create();
    while (true)
    {
        // Send signal to kernel to start monitor
        vector<unsigned char> reply = ioctl.StartMonitor();
        // Get response from kernel to see if kernel has started
        IOCTL_CMD* cmd = (IOCTL_CMD*)&reply[0];
        if (cmd->cmd_class != kTestEnableRansom || cmd->data_len != sizeof(bool) || *((bool*)(&cmd->data[0])) != true)
        {
            Sleep(100);
            continue;
        }

        // Select a file from list of ransomware
        string file_name = getRandomFile("C:/MarauderMap/Volumes/T7Shield1T/230701-Win32-EXE-all-7802");
        // Copy it to C:/Users/hieu/Downloads]
        string exe_path = copyFile(file_name, "C:/Users/hieu/Downloads");
        
        // Send file_name to runner user-mode process
        bool success = false;
        while (true)
        {
            std::string response;
            if (pipe.send(exe_path) && pipe.receive(response))
            {
                success = response == "true";
                std::cout << "Received response: " << (success ? "Success" : "Failure") << std::endl;
                break;
            }
            Sleep(1000);
        }

        // Get response from runner user-mode process to see if ransomware is running
        if (success == false)
        {
            std::cout << "Run : " << exe_path << " failed" << std::endl;
        }
        Sleep(20000); // sleep 20s

        // Send signal to kernel to stop mornitor
        reply = ioctl.StopMonitor();

        // Get response from kernel to see if kernel monitor has stop and killed all ransomware
        if (cmd->cmd_class != kTestDisableRansom || *((bool*)(&cmd->data[0])) != true)
        {
            Sleep(100);
            continue;
        }
        
        // Log ransomware: name, success

        // Do something with the reply contain data about ransom


        // Restore registry
        ImportRegistryKey(EXPORT_PATH);

        // Move that file out of list of ransomware to runned ransomware
        moveFile(file_name, "C:/MarauderMap/Volumes/Tested/");

        // Restore file system
        //copyDirectory("C:/Users/hieu-copy", "C:/Users/hieu");
    }

    ioctl.Close();
    return nullptr;
}

int main()
{
    /*
    rm::ServiceControl service_control(L"RansomTester");

    std::wstring w_path;
    w_path.resize(1000);
    GetModuleFileNameW(nullptr, &w_path[0], 1000);
    w_path.resize(wcslen(&w_path[0]));
    service_control.SetPath(w_path);
    if (service_control.Create() == true)
    {
        service_control.Run();
    }
    else
    {
        return 0;
    }

    rm::RansomTesterService::SetMainWorkerFunction(ServiceMainWorker);
    rm::Service sc;
    sc.SetName(L"RansomTester");
    sc.SetServiceMainFunc((LPSERVICE_MAIN_FUNCTION)rm::RansomTesterService::RansomTesterServiceMain);
    sc.Start();
    */
    ServiceMainWorker();
    return 0;
}

#endif


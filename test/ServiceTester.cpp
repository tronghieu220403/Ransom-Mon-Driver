#ifndef RANSOMTESTER_RANSOMTESTER
#define RANSOMTESTER_RANSOMTESTER

#include "include/service/tester-serv.h"
#include "include/service/servicecontrol.h"
#include "include/service/service.h"
#include "include/file/file.h"
#include "include/com/ioctl/ioctl.h"
#include "include/registry/registry.h"


LPVOID ServiceMainWorker()
{
    Ioctl ioctl;
    ExportRegistryKey(KEY_PATH, EXPORT_PATH);
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
        string file_name = getRandomFile("C:/Ransomware");
        // Copy it to C:/Users/hieu/Downloads]
        string new_copy_file = copyFile(file_name, "C:/Users/hieu/Downloads");
        
        // Send file_name to runner user-mode process


        // Get response from runner user-mode process to see if ransomware is running

        Sleep(20000); // sleep 20s

        // Send signal to kernel to stop mornitor
        vector<unsigned char> reply = ioctl.StopMonitor();

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
   
        // Restore file system

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


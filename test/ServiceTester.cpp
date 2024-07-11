#ifndef RANSOMTESTER_RANSOMTESTER
#define RANSOMTESTER_RANSOMTESTER

#include "include/service/tester-serv.h"
#include "include/service/servicecontrol.h"
#include "include/service/service.h"
#include "include/file/file.h"

LPVOID ServiceMainWorker()
{
    while (true)
    {
        // Send signal to kernel to start monitor
        
        // Get response from kernel to see if kernel has started
        
        // Select a file from list of ransomware
        
        // Copy it to somewhere in C:/Users/hieu
        
        wstring file_name;
        // Send file_name to runner user-mode process

        // Get response from runner user-mode process to see if ransomware is running

        Sleep(20000); // sleep 20s

        // Send signal to kernel to stop mornitor

        // Get response from kernel to see if kernel monitor has stop and killed all ransomware

        // Log ransomware: name, success

        // Restore registry

        // Move that file out of list of ransomware to runned ransomware
   
        // Restore file system

    }
    return nullptr;
}

int main()
{
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

    ServiceMainWorker();
    return 0;
}

#endif


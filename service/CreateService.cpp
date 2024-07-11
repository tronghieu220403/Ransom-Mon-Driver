#ifndef RANSOMMON_RANSOMMON
#define RANSOMMON_RANSOMMON

#include "include/ulti/everything.h"
#include "include/etw/wmieventclass.h"
#include "include/etw/event.h"
#include "include/etw/consumer.h"
#include "include/etw/provider.h"
#include "include/service/ransom-serv.h"
#include "include/service/servicecontrol.h"
#include "include/service/service.h"

bool provider_oke = false;
bool comsumer_oke = false;

void SetUpProvider()
{
    
    ULONG status;
    rm::KernelProvider kp(
        EVENT_TRACE_FLAG_NO_SYSCONFIG
        | EVENT_TRACE_FLAG_DISK_IO
        | EVENT_TRACE_FLAG_THREAD
        | EVENT_TRACE_FLAG_FILE_IO_INIT
        | EVENT_TRACE_FLAG_FILE_IO
        | EVENT_TRACE_FLAG_DISK_FILE_IO
        | EVENT_TRACE_FLAG_PROCESS
        );
    status = kp.BeginTrace();
    if (status != ERROR_SUCCESS && status != ERROR_ALREADY_EXISTS)
    {
        rm::WriteDebug("Can not set up provider");
        return;
    }
    provider_oke = true;

    Sleep(30*1000);

    std::cout << "End trace" << std::endl;

    kp.CloseTrace();
}

void SetUpComsumer()
{
    while (provider_oke == false)
    {
        Sleep(50);
    }
    
    std::vector<rm::IoInfo> temp;
    rm::KernelConsumer kc;
    
    if (kc.Open() != ERROR_SUCCESS)
    {
        rm::WriteDebug("Can not set up consummer kc.Open()");
        return;
    }

    comsumer_oke = true;

    std::cout << "Consumer oke";

    if (kc.Process() != ERROR_SUCCESS)
    {

    }
    
    return;
}

LPVOID ServiceMainWorker()
{
    /*
    std::jthread provider_thread(&SetUpProvider);
    std::jthread comsumer_thread(&SetUpComsumer);
    provider_thread.join();
    comsumer_thread.join();
    */

    return nullptr;
}

int main()
{
    rm::ServiceControl service_control(L"aaaRanMon");

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

    rm::RansomwareMonitoringService::SetMainWorkerFunction(ServiceMainWorker);
    rm::Service sc;
    sc.SetName(L"aaaRanMon");
    sc.SetServiceMainFunc((LPSERVICE_MAIN_FUNCTION)rm::RansomwareMonitoringService::RansomwareMonitoringServiceMain);
    sc.Start();

    ServiceMainWorker();
    return 0;
}

#endif


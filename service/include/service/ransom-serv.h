#ifdef _WIN32


#ifndef RANSOMWAREMONITORING_SERVICE_RANSOMWAREMONITORSERVICE_H_
#define RANSOMWAREMONITORING_SERVICE_RANSOMWAREMONITORSERVICE_H_

#include "../ulti/everything.h"
#include "../ulti/collections.h"
#include "../service/serviceevent.h"

namespace rm
{
    class RansomwareMonitoringService
    {
    private:

        inline static SERVICE_STATUS service_status_ = {0};
        inline static SERVICE_STATUS_HANDLE status_handle_ = nullptr;
        inline static LPVOID main_worker_function_ = nullptr;

    public:
        inline static const std::wstring kName = L"Ransomware Monitoring Service";

        static void SetMainWorkerFunction(LPVOID main_worker_function);

        static void RansomwareMonitoringServiceCtrlHandler(DWORD ctrl_code);
        static void RansomwareMonitoringWorkerFunction();
        static VOID WINAPI RansomwareMonitoringServiceMain();

        static void RansomwareMonitoringServiceStop();
        static void RansomwareMonitoringServicePause();
        static void RansomwareMonitoringServiceContinue();
        static void RansomwareMonitoringServiceShutdown();

        static void Close();

    };
}

#endif

#endif
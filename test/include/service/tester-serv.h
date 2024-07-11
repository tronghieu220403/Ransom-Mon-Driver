#ifdef _WIN32


#ifndef RANSOMWARESANDBOX_SERVICE_SANDBOXSERVICE_H_
#define RANSOMWARESANDBOX_SERVICE_SANDBOXSERVICE_H_

#include "../ulti/everything.h"
#include "../ulti/collections.h"
#include "../service/serviceevent.h"

namespace rm
{
    class RansomTesterService
    {
    private:

        inline static SERVICE_STATUS service_status_ = {0};
        inline static SERVICE_STATUS_HANDLE status_handle_ = nullptr;
        inline static LPVOID main_worker_function_ = nullptr;

    public:
        inline static const std::wstring kName = L"Ransomware Tester Service";

        static void SetMainWorkerFunction(LPVOID main_worker_function);

        static void RansomTesterServiceCtrlHandler(DWORD ctrl_code);
        static void RansomwareMonitoringWorkerFunction();
        static VOID WINAPI RansomTesterServiceMain();

        static void RansomTesterServiceStop();
        static void RansomTesterServicePause();
        static void RansomTesterServiceContinue();
        static void RansomTesterServiceShutdown();

        static void Close();

    };
}

#endif

#endif
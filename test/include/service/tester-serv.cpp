#ifdef _WIN32

#include "tester-serv.h"

namespace rm
{
    void RansomTesterService::RansomTesterServiceStop()
    {
        WriteDebug("Get SERVICE_CONTROL_STOP");

        if (service_status_.dwCurrentState != SERVICE_RUNNING)
            return;

        service_status_.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_STOP_PENDING;
        service_status_.dwWin32ExitCode = 1;
        service_status_.dwCheckPoint = 0x00000003;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_STOP_PENDING");
            exit(0);
        }

        // Do something here while pending

        // End of pending

        service_status_.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_STOPPED;
        service_status_.dwWin32ExitCode = 1;
        service_status_.dwCheckPoint = 0x00000001;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_STOPPED after SERVICE_STOP_PENDING");
            exit(0);
        }

        // This will signal the worker thread to start shutting down

    }

    void RansomTesterService::RansomTesterServicePause()
    {
        WriteDebug("Get SERVICE_CONTROL_PAUSE");

        if (service_status_.dwCurrentState != SERVICE_RUNNING)
            return;

        service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_PAUSE_PENDING;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 0x00000006;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_PAUSE_PENDING");
            exit(0);
        }
       
        // Do something here while pending

        // End of pending

        service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE |
            SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_PAUSED;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 0x00000007;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_PAUSED after SERVICE_PAUSE_PENDING");
            exit(0);
        }

        // This will signal the worker thread to be paused

    }

    void RansomTesterService::RansomTesterServiceContinue()
    {
        WriteDebug("Get SERVICE_CONTROL_CONTINUE");

        if (service_status_.dwCurrentState != SERVICE_PAUSED)
            return;
        
        service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_CONTINUE_PENDING;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 0x00000005;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_CONTINUE_PENDING");
            exit(0);
        }

        // Do something here while pending

        // End of pending

        service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE |
            SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_RUNNING;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 0x00000004;

        if (SetServiceStatus (status_handle_, &service_status_) == FALSE){
            WriteDebug("Failed to SetServiceStatus SERVICE_RUNNING after SERVICE_CONTINUE_PENDING");
            exit(0);
        }


    }

    void RansomTesterService::RansomTesterServiceShutdown()
    {
        WriteDebug("Get SERVICE_CONTROL_SHUTDOWN");

        exit(0);
    }


    void RansomTesterService::SetMainWorkerFunction(LPVOID main_worker_function)
    {
        main_worker_function_ = main_worker_function;
    }

    void RansomTesterService::RansomTesterServiceCtrlHandler(DWORD ctrl_code)
    {
        switch (ctrl_code)
        {
            /*
            case SERVICE_CONTROL_STOP:
                RansomTesterService::RansomTesterServiceStop();
                break;
        
            case SERVICE_CONTROL_PAUSE:
                RansomTesterService::RansomTesterServicePause();
                break;

            case SERVICE_CONTROL_CONTINUE:
                RansomTesterService::RansomTesterServiceContinue();
                break;
            */

            case SERVICE_CONTROL_SHUTDOWN:
                RansomTesterService::RansomTesterServiceShutdown();
                break;

                default:

                WriteDebug("Get service control code: " + ctrl_code);
                break;
        }

    }

    void RansomTesterService::RansomwareMonitoringWorkerFunction()
    {
        WriteDebug("In worker");
        // Tell the services that the service is running.
        service_status_.dwCurrentState = SERVICE_RUNNING;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 4;

        if (SetServiceStatus(status_handle_, &service_status_) == FALSE)
        {
            rm::WriteDebug("Can not SetServiceStatus in RansomwareMonitoringWorkerFunction");
            return;
        }

        ((void(*)(void))main_worker_function_)();
    }


    VOID WINAPI RansomTesterService::RansomTesterServiceMain()
    {
        WriteDebug("In main");

        status_handle_ = RegisterServiceCtrlHandler(kName.data(), (LPHANDLER_FUNCTION)RansomTesterService::RansomTesterServiceCtrlHandler);

        if (status_handle_ == NULL)
        {
            WriteDebug("Can not RegisterServiceCtrlHandler: " + GetLastError());
            Close();
            return;
        }

        ZeroMemory(&service_status_, sizeof(service_status_));
        service_status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        service_status_.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
        service_status_.dwCurrentState = SERVICE_RUNNING;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwServiceSpecificExitCode = 0;
        service_status_.dwCheckPoint = 0;

        if (SetServiceStatus (status_handle_ , &service_status_) == FALSE)
        {
            WriteDebug("Can not SetServiceStatus: " + GetLastError());
            Close();
            return;
        }

        /*
        Service worker is here
        */
        RansomwareMonitoringWorkerFunction();

        /*
        // Tell the service controller we are stopped
        service_status_.dwControlsAccepted = 0;
        service_status_.dwCurrentState = SERVICE_STOPPED;
        service_status_.dwWin32ExitCode = 0;
        service_status_.dwCheckPoint = 3;

        if (SetServiceStatus(status_handle_, &service_status_) == FALSE)
        {
            WriteDebug("Can not SetServiceStatus in line 253: " + GetLastError());
        }

        EXIT:
            Close();
            return;
        */
    }

    void RansomTesterService::Close()
    {
        if (status_handle_ != INVALID_HANDLE_VALUE && status_handle_ != nullptr)
        {
            CloseHandle(status_handle_);
        }
        return;
    }


}

#endif
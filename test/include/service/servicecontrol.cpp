#ifdef _WIN32

#include "servicecontrol.h"

namespace rm
{
    std::wstring ServiceControl::GetName() const {
        return w_name_;
    }

    void ServiceControl::SetName(const std::wstring& name) {
        w_name_ = name;
    }


    ServiceControl::ServiceControl(const std::wstring& name)
        : w_name_(name),
        h_services_control_manager_(OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS))
    {
    }

    ServiceControl::ServiceControl(const std::wstring& name, const std::wstring& path)
        : w_name_(name), w_path_(path),
        h_services_control_manager_(OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS))
    {
    }

    void ServiceControl::SetPath(std::wstring w_path)
    {
        w_path_ = L"\"" + w_path + L"\"";
    }

    ServiceControl::~ServiceControl()
    {
        CloseServiceHandle(h_services_control_manager_);
        h_services_control_manager_ = 0;
    }

    bool ServiceControl::Create()
    {

        SC_HANDLE handle_service = CreateServiceW(h_services_control_manager_, w_name_.data(), w_name_.data(),
            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
            w_path_.data(), NULL, NULL, NULL, NULL, NULL);


        if (handle_service == NULL)
        {
            ULONG status = GetLastError();
            if (status == ERROR_DUPLICATE_SERVICE_NAME || status == ERROR_SERVICE_EXISTS)
            {
                return true;
            }
            return false;
        }

        CloseServiceHandle(handle_service);
        return true;
    }

    bool ServiceControl::Run()
    {
        SC_HANDLE handle_service;
        std::wstring working_dir;
        working_dir.resize(MAX_PATH + 1);

        GetCurrentDirectoryW(MAX_PATH + 1, &working_dir[0]);
        LPCWSTR argv_start[] = { &w_name_[0], &working_dir[0] };

        handle_service = OpenServiceW(h_services_control_manager_, w_name_.data(), SERVICE_ALL_ACCESS);

        if (handle_service == NULL)
        {
            return false;
        }

        if (!StartServiceW(handle_service, 2, argv_start))
        {
            return false;
        }
        CloseServiceHandle(handle_service);
        return true;
    }

}

#endif
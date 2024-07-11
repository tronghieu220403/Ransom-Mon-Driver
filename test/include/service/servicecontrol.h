#ifdef _WIN32


#ifndef RANSOMTESTER_SERVICE_SERVICECONTROL_H_
#define RANSOMTESTER_SERVICE_SERVICECONTROL_H_

#include "../ulti/everything.h"
#include "../ulti/collections.h"

namespace rm
{
    class ServiceControl
    {
    private:
        std::wstring w_name_;
        std::wstring w_path_;
        SC_HANDLE h_services_control_manager_;

    public:

        ServiceControl() = default;

        ServiceControl(const std::wstring& name);
        ServiceControl(const std::wstring& name, const std::wstring& path);

        std::wstring GetName() const;
        void SetName(const std::wstring& name);

        void SetPath(std::wstring w_path);

        bool Create();
        bool Run();

        ~ServiceControl();
    };

}

#endif

#endif
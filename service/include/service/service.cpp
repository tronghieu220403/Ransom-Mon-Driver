#ifdef _WIN32

#include "service.h"

namespace rm
{

    LPSERVICE_MAIN_FUNCTION Service::GetServiceMainFunc() const {
        return service_main_func_;
    }

    void Service::SetServiceMainFunc(LPSERVICE_MAIN_FUNCTION main_func) {
        service_main_func_ = main_func;
    }


    std::wstring Service::GetName() const {
        return name_;
    }

    void Service::SetName(std::wstring name) {
        name_ = name;
    }

    void Service::Start()
    {
        WriteDebug("Begin Start");
        SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            { &(name_)[0],				service_main_func_},
            { NULL,						NULL }
        };
        StartServiceCtrlDispatcher(DispatchTable);
    }

}

#endif
#ifdef _WIN32

#ifndef PROCESSMONITORING_SERVICE_SERVICE_H_
#define PROCESSMONITORING_SERVICE_SERVICE_H_

#include "../ulti/everything.h"
#include "../ulti/collections.h"

namespace rm
{
    class Service
    {
    private:
        std::wstring name_;
        LPSERVICE_MAIN_FUNCTION service_main_func_ = nullptr;

    public:

        LPSERVICE_MAIN_FUNCTION GetServiceMainFunc() const;
        void SetServiceMainFunc(LPSERVICE_MAIN_FUNCTION main_func);

        std::wstring GetName() const;
        void SetName(std::wstring name);

        void Start();
    };
}

#endif

#endif
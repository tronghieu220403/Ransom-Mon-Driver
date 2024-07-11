#ifdef _WIN32

#ifndef RANSOMTESTER_SERVICE_SERVICEEVENT_H_
#define RANSOMTESTER_SERVICE_SERVICEEVENT_H_

#include "../ulti/everything.h"
#include "../ulti/collections.h"

namespace rm
{
    class ServiceEvent
    {
        public: 
            HANDLE stop_event_ = nullptr;
            HANDLE pause_event_ = nullptr;
            
            HANDLE pause_handled_ = nullptr;
            HANDLE stop_handled_ = nullptr;

            void Close();
    };
}
#endif

#endif

#ifdef _WIN32

#include "serviceevent.h"


namespace rm
{
    void ServiceEvent::Close()
    {
        if (stop_event_ != INVALID_HANDLE_VALUE && stop_event_ != nullptr)
        {
            CloseHandle(stop_event_);
        }
        if (pause_event_ != INVALID_HANDLE_VALUE && pause_event_ != nullptr)
        {
            CloseHandle(pause_event_);
        }
        if (pause_handled_ != INVALID_HANDLE_VALUE && pause_handled_ != nullptr)
        {
            CloseHandle(pause_handled_);
        }
        if (stop_handled_ != INVALID_HANDLE_VALUE && stop_handled_ != nullptr)
        {
            CloseHandle(stop_handled_);
        }

    }   

}
#endif
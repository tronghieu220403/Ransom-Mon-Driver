#ifdef _WIN32

#include "threadevent.h"

namespace rm
{
    ThreadEvent::ThreadEvent()
    {
        
    }

    ThreadEvent::ThreadEvent(PEVENT_TRACE p_event, int pointer_size)
    {
        this->Event::Event(p_event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }

    ThreadEvent::ThreadEvent(Event event, int pointer_size)
    {
        this->Event::Event(event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }


    bool ThreadEvent::FindPropertiesInfo(int pointer_size)
    {
        if (GetProcessIdOffset() == -1 || GetProcessIdLength() == -1)
        {
            std::pair<int, int> prop_info =  WmiEventClass(EventType::kThread, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"ProcessId");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetProcessIdOffset(prop_info.first);
                SetProcessIdLength(prop_info.second);
            }
        }

        if (GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            std::pair<int, int> prop_info =  WmiEventClass(EventType::kThread, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"ThreadId");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetThreadIdOffset(prop_info.first);
                SetThreadIdLength(prop_info.second);
            }
        }

        if (GetProcessIdOffset() == -1 || GetProcessIdLength() == -1 || GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            return false;
        }
        return true;
    }

    void ThreadEvent::UpdateAttributes()
    {
        int process_id = 0;
        int thread_id = 0;

        memcpy(&process_id, this->Event::GetPEventData() + GetProcessIdOffset(), GetProcessIdLength());
        SetProcessId(process_id);

        memcpy(&thread_id, this->Event::GetPEventData() + GetThreadIdOffset(), GetThreadIdLength());
        SetThreadId(thread_id);

    }

    int ThreadEvent::GetProcessIdOffset() {
        return process_id_offset_;
    }

    void ThreadEvent::SetProcessIdOffset(int process_id_offset) {
        process_id_offset_ = process_id_offset;
    }

    int ThreadEvent::GetProcessIdLength() {
        return process_id_length_;
    }

    void ThreadEvent::SetProcessIdLength(int process_id_length) {
        process_id_length_ = process_id_length;
    }

    int ThreadEvent::GetThreadIdOffset() {
            return thread_id_offset_;
    }

    void ThreadEvent::SetThreadIdOffset(int thread_id_offset) {
            thread_id_offset_ = thread_id_offset;
    }

    int ThreadEvent::GetThreadIdLength() {
        return thread_id_offset_;
    }

    void ThreadEvent::SetThreadIdLength(int thread_id_length) {
        thread_id_length_ = thread_id_length;
    }

    int ThreadEvent::GetProcessId() const {
            return process_id_;
    }

    void ThreadEvent::SetProcessId(int process_id) {
            process_id_ = process_id;
    }

    int ThreadEvent::GetThreadId() const {
            return thread_id_;
    }

    void ThreadEvent::SetThreadId(int thread_id) {
            thread_id_ = thread_id;
    }
};

#endif

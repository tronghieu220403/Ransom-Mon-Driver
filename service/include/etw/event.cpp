#ifdef _WIN32

#include "event.h"

namespace rm
{

    Event::Event()
    {
        guid_.resize(100);
    }

    Event::Event(PEVENT_TRACE p_event)
    {
        guid_.resize(1000);
        guid_.resize(StringFromGUID2(p_event->Header.Guid, &guid_[0], 1000));
        if (guid_[guid_.size() - 1] == 0)
        {
            guid_.pop_back();
        }
        SetVersion(p_event->Header.Class.Version);
        SetType(p_event->Header.Class.Type);

        ft_.dwHighDateTime = p_event->Header.TimeStamp.HighPart;
        ft_.dwLowDateTime = p_event->Header.TimeStamp.LowPart;

        SetTimeStamp(p_event->Header.TimeStamp.QuadPart);
        SetMofLength(p_event->MofLength);

        SetPEventData((PBYTE)(p_event->MofData));
    }

    std::wstring Event::GetGuid() const {
        return guid_;
    }

    void Event::SetGuid(std::wstring guid) {
        guid_ = guid;
    }

    int Event::GetVersion() const {
        return version_;
    }

    void Event::SetVersion(int version) {
        version_ = version;
    }

    int Event::GetType() const {
        return type_;
    }

    void Event::SetType(int type) {
        type_ = type;
    }

    FILETIME Event::GetFileTime() const
    {
        return ft_;
    }

    void Event::SetFileTime(FILETIME ft)
    {
        ft_ = ft;
    }

    ULONGLONG Event::GetTimeStamp() const
    {
        return time_stamp_;
    }

    void Event::SetTimeStamp(ULONGLONG time_stamp)
    {
        time_stamp_ = time_stamp;
    }

    ULONGLONG Event::GetTimeInMs() const
    {
        return time_stamp_ / 10000;
    }

    ULONG Event::GetMofLength() const
    {
        return mof_length_;
    }

    void Event::SetMofLength(ULONG mof_length)
    {
        mof_length_ = mof_length;
    }

    PBYTE Event::GetPEventData() const {
        return p_event_data_;
    }

    void Event::SetPEventData(PBYTE p_event_data) {
        p_event_data_ = p_event_data;
    }

}

#endif
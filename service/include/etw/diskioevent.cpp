#ifdef _WIN32


#include "diskioevent.h"

namespace rm
{
    DiskIoEvent::DiskIoEvent()
    {
        
    }

    DiskIoEvent::DiskIoEvent(PEVENT_TRACE p_event, int pointer_size)
    {
        this->Event::Event(p_event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }

    DiskIoEvent::DiskIoEvent(const Event& event, int pointer_size)
    {
        this->Event::Event(event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }

    bool DiskIoEvent::FindPropertiesInfo(int pointer_size)
    {
        if (GetTransferSizeOffset() == -1 || GetTransferSizeLength() == -1)
        {
            std::pair<int, int> prop_info =  WmiEventClass(EventType().kDiskIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"TransferSize");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetTransferSizeOffset(prop_info.first);
                SetTransferSizeLength(prop_info.second);
            }
        }

        if (GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            std::pair<int, int> prop_info =  WmiEventClass(EventType().kDiskIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"IssuingThreadId");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetThreadIdOffset(prop_info.first);
                SetThreadIdLength(prop_info.second);
            }
        }

        if (GetTransferSizeOffset() == -1 || GetTransferSizeLength() == -1 || GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            return false;
        }
        return true;
    }

    void DiskIoEvent::UpdateAttributes()
    {
        unsigned long long transfer_size = 0;
        int thread_id = 0;

        memcpy(&transfer_size, Event::GetPEventData() + GetTransferSizeOffset(), GetTransferSizeLength());
        SetTransferSize(transfer_size);

        memcpy(&thread_id, Event::GetPEventData() + GetThreadIdOffset(), GetThreadIdLength());
        SetThreadId(thread_id);

    }

    void DiskIoEvent::SetTransferSize(unsigned long long transfer_size)
    {
        transfer_size_ = transfer_size;
    }

    unsigned long long DiskIoEvent::GetTransferSize() const
    {
        return transfer_size_;
    }

    void DiskIoEvent::SetThreadId(int thread_id)
    {
        thread_id_ = thread_id;
    }

    int DiskIoEvent::GetThreadId() const
    {
        return thread_id_;
    }

    void DiskIoEvent::SetTransferSizeOffset(int transfer_size_offset)
    {
        transfer_size_offset_ = transfer_size_offset;

    }
    int DiskIoEvent::GetTransferSizeOffset()
    {
        return transfer_size_offset_;
    }

    void DiskIoEvent::SetTransferSizeLength(int transfer_size_length)
    {
        transfer_size_length_ = transfer_size_length;
    }

    int DiskIoEvent::GetTransferSizeLength()
    {
        return transfer_size_length_;
    }

    void DiskIoEvent::SetThreadIdOffset(int thread_id_offset)
    {
        thread_id_offset_ = thread_id_offset;

    }
    int DiskIoEvent::GetThreadIdOffset()
    {
        return thread_id_offset_;
    }

    void DiskIoEvent::SetThreadIdLength(int thread_id_length)
    {
        thread_id_length_ = thread_id_length;
    }

    int DiskIoEvent::GetThreadIdLength()
    {
        return thread_id_length_;
    }

};

#endif
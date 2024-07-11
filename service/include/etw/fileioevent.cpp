#ifdef _WIN32


#include "fileioevent.h"

namespace rm
{
    FileIoEvent::FileIoEvent()
    {

    }

    FileIoEvent::FileIoEvent(PEVENT_TRACE p_event, int pointer_size)
    {
        this->Event::Event(p_event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }

    FileIoEvent::FileIoEvent(const Event& event, int pointer_size)
    {
        this->Event::Event(event);
        if (FindPropertiesInfo(pointer_size) == false)
        {
            return;
        };
        UpdateAttributes();
    }

    bool FileIoEvent::FindPropertiesInfo(int pointer_size)
    {
        if (GetIoSizeOffset() == -1 || GetIoSizeLength() == -1)
        {
            std::pair<int, int> prop_info = WmiEventClass(EventType().kFileIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"IoSize");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetIoSizeOffset(prop_info.first);
                SetIoSizeLength(prop_info.second);
            }
        }

        if (GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            std::pair<int, int> prop_info = WmiEventClass(EventType().kFileIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"TTID");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetThreadIdOffset(prop_info.first);
                SetThreadIdLength(prop_info.second);
            }
        }

        if (GetFileKeyOffset() == -1 || GetFileKeyLength() == -1)
        {
            /*
            std::pair<int, int> prop_info = WmiEventClass(EventType().kFileIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"FileKey");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetFileKeyOffset(prop_info.first);
                SetFileKeyLength(prop_info.second);
            }
            */
        }

        if (GetFileNameOffset() == -1 || GetFileNameLength() == -1)
        {
            std::pair<int, int> prop_info = WmiEventClass(EventType().kFileIo, Event::GetVersion(), Event::GetType(), pointer_size).GetPropertyInfo(L"FileName");
            if (prop_info.first != -1 && prop_info.second != 0)
            {
                SetFileNameOffset(prop_info.first);
                SetFileNameLength(prop_info.second);
            }
        }

        if (GetIoSizeOffset() == -1 || GetIoSizeLength() == -1 || GetThreadIdOffset() == -1 || GetThreadIdLength() == -1)
        {
            return false;
        }
        return true;
    }

    void FileIoEvent::UpdateAttributes()
    {
        unsigned long long io_size = 0;
        int thread_id = 0;
		int file_key = 0;
        switch (Event::GetType())
        {
        case FileIoEventType::kWrite:
		case FileIoEventType::kRead:
            memcpy(&io_size, Event::GetPEventData() + GetIoSizeOffset(), GetIoSizeLength());
            SetIoSize(io_size);

            memcpy(&thread_id, Event::GetPEventData() + GetThreadIdOffset(), GetThreadIdLength());
            SetThreadId(thread_id);

			memcpy(&file_key, Event::GetPEventData() + GetFileKeyOffset(), GetFileKeyLength());
			SetFileKey(file_key);

			break;
		case FileIoEventType::kFileCreate:
        case FileIoEventType::kFileDelete:
		case FileIoEventType::kName:
        case FileIoEventType::kFileRundown:
            memcpy(&file_key, Event::GetPEventData() + GetFileKeyOffset(), GetFileKeyLength());
            SetFileKey(file_key);

			SetFileName((char*)(Event::GetPEventData() + GetFileNameOffset()));
			break;

        default:
            break;
        }


    }

    void FileIoEvent::SetIoSize(unsigned long long io_size)
    {
        io_size_ = io_size;
    }

    unsigned long long FileIoEvent::GetIoSize() const
    {
        return io_size_;
    }

    void FileIoEvent::SetThreadId(int thread_id)
    {
        thread_id_ = thread_id;
    }

    int FileIoEvent::GetThreadId() const
    {
        return thread_id_;
    }

    void FileIoEvent::SetFileKey(int file_key)
    {
		file_key_ = file_key;
    }

	int FileIoEvent::GetFileKey() const
	{
		return file_key_;
	}

    void FileIoEvent::SetFileName(std::string file_name)
    {
		file_name_ = file_name;
    }

	std::string FileIoEvent::GetFileName() const
	{
		return file_name_;
	}

    void FileIoEvent::SetIoSizeOffset(int io_size_offset)
    {
        io_size_offset_ = io_size_offset;

    }
    int FileIoEvent::GetIoSizeOffset()
    {
        return io_size_offset_;
    }

    void FileIoEvent::SetIoSizeLength(int io_size_length)
    {
        io_size_length_ = io_size_length;
    }

    int FileIoEvent::GetIoSizeLength()
    {
        return io_size_length_;
    }

    void FileIoEvent::SetThreadIdOffset(int thread_id_offset)
    {
        thread_id_offset_ = thread_id_offset;

    }
    int FileIoEvent::GetThreadIdOffset()
    {
        return thread_id_offset_;
    }

    void FileIoEvent::SetThreadIdLength(int thread_id_length)
    {
        thread_id_length_ = thread_id_length;
    }

    int FileIoEvent::GetThreadIdLength()
    {
        return thread_id_length_;
    }

	void FileIoEvent::SetFileKeyOffset(int file_key_offset)
	{
		file_key_offset_ = file_key_offset;
	}

	int FileIoEvent::GetFileKeyOffset()
	{
		return file_key_offset_;
	}

	void FileIoEvent::SetFileKeyLength(int file_key_length)
	{
		file_key_length_ = file_key_length;
	}

	int FileIoEvent::GetFileKeyLength()
	{
		return file_key_length_;
	}

	void FileIoEvent::SetFileNameOffset(int file_name_offset)
	{
		file_name_offset_ = file_name_offset;
	}

	int FileIoEvent::GetFileNameOffset()
	{
		return file_name_offset_;
	}

	void FileIoEvent::SetFileNameLength(int file_name_length)
	{
		file_name_length_ = file_name_length;
	}

	int FileIoEvent::GetFileNameLength()
	{
		return file_name_length_;
	}

};

#endif
#ifdef _WIN32

#include "consumer.h"
namespace rm
{
	KernelConsumer::KernelConsumer()
	{
        disk_io_mutex_.SetMutex("disk_io_mutex");
        trace_.LogFileName = NULL;
        trace_.LoggerName = (LPWSTR)KERNEL_LOGGER_NAME;
        trace_.BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACK)(ProcessBuffer);
        trace_.EventCallback = (PEVENT_CALLBACK)(ProcessEvent);
        trace_.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_RAW_TIMESTAMP;// | PROCESS_TRACE_MODE_EVENT_RECORD; // create real time sesion + event should be represented as EVENT_RECORD structure

	}

    DWORD KernelConsumer::Open()
    {
        handle_trace_ = OpenTrace(&trace_);
        TRACE_LOGFILE_HEADER* p_header_ = &trace_.LogfileHeader;
        if ((TRACEHANDLE)INVALID_HANDLE_VALUE == handle_trace_)
        {
            Close();
            return GetLastError();
        }

        SetPointerSize(p_header_->PointerSize);

        if (p_header_->PointerSize != sizeof(PVOID))
        {
            p_header_ = (PTRACE_LOGFILE_HEADER)((PUCHAR)p_header_ +
                2 * (p_header_->PointerSize - sizeof(PVOID)));
        }

        return ERROR_SUCCESS;
    }

    DWORD KernelConsumer::Process()
    {
        ULONG status = ProcessTrace(&handle_trace_, 1, 0, 0);
        if (status != ERROR_SUCCESS)
        {
            Close();
        }
        return status;
    }

    void KernelConsumer::SetPointerSize(int pointer_size)
    {
        pointer_size_ = pointer_size;
    }

    int KernelConsumer::GetPointerSize()
    {
        return pointer_size_;
    }

    ULONG WINAPI KernelConsumer::ProcessBuffer(PEVENT_TRACE_LOGFILE p_buffer)
    {
        UNREFERENCED_PARAMETER(p_buffer);

        return TRUE;
    }

    inline static bool wstrcmp(const std::wstring& a, const std::wstring& b)
    {
        if (a.size() != b.size())
        {
            return false;
        }
        for (int i = 0; i < a.size(); i++)
        {
            if (a[i] != b[i])
            {
                return false;
            }
        }
        return true;
    }

    VOID WINAPI KernelConsumer::ProcessEvent(PEVENT_TRACE p_event)
    {

        if (IsEqualGUID(p_event->Header.Guid, EventTraceGuid) &&
            p_event->Header.Class.Type == EVENT_TRACE_TYPE_INFO)
        {
            ; // Skip this event.
        }
        else
        {
            Event event(p_event);
            
            int type = event.GetType();
            std::wstring guid = event.GetGuid();

            std::transform(guid.begin(), guid.end(), guid.begin(), ::toupper);

            if (guid == EventType::kThread)
            {
                if (type == 1 || type == 2 || type == 3 || type == 4)
                {
                    ProcessThread(event);
                }
            }
            else if (guid == EventType::kDiskIo)
            {
                if (type == DiskIoEventType::kWrite)
                {
                    ProcessDiskIo(event);
                }
            }
            else if (guid == EventType::kFileIo)
            {
                rm::WriteDebug(std::to_string(type));
                if (type == FileIoEventType::kWrite || type == FileIoEventType::kFileCreate)
                {
                    ProcessFileIo(event);
                }
            }
        }
                
        return;
    }

    VOID WINAPI KernelConsumer::ProcessThread(Event event)
    {
        ThreadEvent thread_event(event, GetPointerSize());

        // remember that this thread event has 2 event: start and end thread

        int type = thread_event.GetType();
        int thread_id = thread_event.GetThreadId();
        int pid = thread_event.GetProcessId();

        if (static_cast<unsigned long long>(thread_id) + 1 >= thread_.size())
        {
            thread_.resize(static_cast<std::vector<int, std::allocator<int>>::size_type>(thread_id) + 1);
        }
        
        // Thread start
        if (thread_event.GetType() == 1 || thread_event.GetType() == 3)
        {
            rm::WriteDebug("PID: " + std::to_string(pid) + ", Thread: " + std::to_string(thread_id));
            // push data to a shared pointer pointer vector
            thread_[thread_id] = pid;
            /*
            WriteDebug(std::to_string(pid) + " " + std::to_string(thread_id) + " create.");
            */
        }
        // Thread end
        else 
        {
            // remove data from a shared pointer vector
            thread_[thread_id] = 0;
            /*
            WriteDebug(std::to_string(pid) + " " + std::to_string(thread_id) + " end.");
            */
        }

        return VOID();
    }

    VOID WINAPI KernelConsumer::ProcessDiskIo(const Event& event)
    {
        DiskIoEvent disk_io_event(event, GetPointerSize());
        struct IoInfo io = { 0 };

        if (disk_io_event.GetThreadId() > thread_.size() || thread_[disk_io_event.GetThreadId()] == 0)
        {
            return;
        }
        io.pid = thread_[disk_io_event.GetThreadId()];
        io.size = disk_io_event.GetTransferSize();
        io.filetime = disk_io_event.GetFileTime();

		//rm::WriteDebug("Disk IO: " + std::to_string(io.pid) + " " + std::to_string(io.size) + " ");

        return VOID();
    }
    
    VOID WINAPI KernelConsumer::ProcessFileIo(const Event& event)
    {
		FileIoEvent file_io_event(event, GetPointerSize());
		
        if (event.GetType() == FileIoEventType::kFileCreate)
        {
            rm::WriteDebug("File key: " + std::to_string(file_io_event.GetFileKey()) + " " + file_io_event.GetFileName());
			return;
		}

        rm::WriteDebug("File IO: " + std::to_string(file_io_event.GetThreadId()) + " " + std::to_string(file_io_event.GetIoSize()));

        struct IoInfo io = { 0 };
        /*
        if (file_io_event.GetThreadId() > thread_.size() || thread_[file_io_event.GetThreadId()] == 0)
        {
            return;
        }
        io.pid = thread_[file_io_event.GetThreadId()];
        */
        io.size = file_io_event.GetIoSize();
        io.filetime = file_io_event.GetFileTime();


        return VOID();
    }

    ULONG KernelConsumer::Close()
    {
        ULONG status = ERROR_SUCCESS;
        if ((TRACEHANDLE)INVALID_HANDLE_VALUE != handle_trace_)
        {
            status = CloseTrace(handle_trace_);
            handle_trace_ = NULL;
        }
        return status;
    }

    KernelConsumer::~KernelConsumer()
    {
        if (Close() != ERROR_SUCCESS){
            // throw some exception here;
        }
        return;
    }
}

#endif
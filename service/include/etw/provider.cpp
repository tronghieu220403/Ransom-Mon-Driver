#ifdef _WIN32

#include "provider.h"

namespace rm
{

    KernelProvider::KernelProvider()
    {
        while (session_properties_ == nullptr)
        {
            session_properties_ = (EVENT_TRACE_PROPERTIES*)malloc(buffer_size_);
        }
        ZeroMemory(session_properties_, sizeof(EVENT_TRACE_PROPERTIES));
        session_properties_->Wnode.BufferSize = buffer_size_;
        session_properties_->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
        session_properties_->Wnode.ClientContext = 2; //System clock resolution
        session_properties_->Wnode.Guid = SystemTraceControlGuid;
        session_properties_->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
        session_properties_->MaximumFileSize = 0;
        session_properties_->LoggerNameOffset = 0;

    }

    KernelProvider::KernelProvider(ULONG flags)
    {
        this->KernelProvider::KernelProvider();
        SetFlags(flags);
    }

    void KernelProvider::SetFlags(ULONG flags)
    {
        session_properties_->EnableFlags = flags;
    }


    TRACEHANDLE KernelProvider::GetSessionHandle() const {
        return session_handle_;
    }

    void KernelProvider::SetSessionHandle(TRACEHANDLE session_handle) {
        session_handle_ = session_handle;
    }

    EVENT_TRACE_PROPERTIES KernelProvider::GetSessionProperties() const {
        EVENT_TRACE_PROPERTIES session_properties;
        memcpy(&session_properties, session_properties_, sizeof(session_properties));
        return session_properties;
    }

    ULONG KernelProvider::BeginTrace()
	{
        ULONG status = ERROR_SUCCESS;

        status = StartTrace((PTRACEHANDLE)&session_handle_, KERNEL_LOGGER_NAME, session_properties_);

        return status;

	}

    ULONG KernelProvider::CloseTrace()
	{
        ULONG status = ERROR_SUCCESS;
        if (session_handle_ != NULL)
        {
            status = ControlTrace(session_handle_, KERNEL_LOGGER_NAME, session_properties_, EVENT_TRACE_CONTROL_STOP);
            session_handle_ = NULL;
        }

        return status;
	}

    KernelProvider::~KernelProvider()
    {
        /*
        CloseTrace();
        if (session_properties_ != nullptr)
        {
            free(session_properties_);
            session_properties_ = 0;
        }
        */
    }
}

#endif

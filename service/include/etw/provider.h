#ifdef _WIN32

#pragma once

#ifndef PROCESSMONITORING_ETW_PROVIDER_H_
#define PROCESSMONITORING_ETW_PROVIDER_H_

#define INITGUID

#include "../ulti/everything.h"

namespace rm
{
	class KernelProvider
	{
	private:
		TRACEHANDLE session_handle_ = NULL;
		int buffer_size_ = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME) * 2;
		EVENT_TRACE_PROPERTIES* session_properties_;

	public:

		KernelProvider();
		KernelProvider(ULONG flags);

		void SetFlags(ULONG flags);

    	TRACEHANDLE GetSessionHandle() const;
		void SetSessionHandle(TRACEHANDLE session_handle);
		EVENT_TRACE_PROPERTIES GetSessionProperties() const;

		ULONG BeginTrace();
		ULONG CloseTrace();

		~KernelProvider();
	};

}


#endif

#endif

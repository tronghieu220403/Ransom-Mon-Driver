#ifdef _WIN32

#pragma once

#ifndef PROCESSMONITORING_ETW_COMSUMER_H_
#define PROCESSMONITORING_ETW_COMSUMER_H_

#define INITGUID

#include "../ulti/everything.h"
#include "../ulti/collections.h"
#include "../mutex/mutex.h"

#include "event.h"
#include "diskioevent.h"
#include "fileioevent.h"
#include "threadevent.h"

namespace rm
{

	class KernelConsumer
	{
	private:
		EVENT_TRACE_LOGFILE trace_ = { 0 };
		TRACEHANDLE handle_trace_ = NULL;
		inline static int pointer_size_ = 0;

		inline static NamedMutex disk_io_mutex_;

		inline static std::vector<int> thread_;

	public:
		KernelConsumer();

		DWORD Open();
		DWORD Process();

		static void SetPointerSize(int pointer_size);
		static int GetPointerSize();

		static ULONG WINAPI ProcessBuffer(PEVENT_TRACE_LOGFILE p_buffer);
		static VOID WINAPI ProcessEvent(PEVENT_TRACE p_event);

		static VOID WINAPI
			ProcessThread(Event event);

		static VOID WINAPI
			ProcessDiskIo(const Event& event);

		static VOID WINAPI
			ProcessFileIo(const Event& event);

		ULONG Close();

		~KernelConsumer();
	};

};

#endif

#endif
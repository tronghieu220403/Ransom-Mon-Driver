#ifdef _WIN32

#pragma once

#ifndef PROCESSMONITORING_ETW_THREADEVENT_H_
#define PROCESSMONITORING_ETW_THREADEVENT_H_

#include "event.h"
#include "wmieventclass.h"

namespace rm
{
	class ThreadEvent: public Event
	{
	private:
		inline static int process_id_offset_ = -1;
		inline static int process_id_length_ = -1;
        inline static int thread_id_offset_ = -1;
		inline static int thread_id_length_ = -1;
        int process_id_ = -1;
        int thread_id_ = -1;

	public:
		ThreadEvent();
		ThreadEvent(PEVENT_TRACE p_event, int pointer_size);
		ThreadEvent(Event event, int pointer_size);

        bool FindPropertiesInfo(int pointer_size);
		void UpdateAttributes();

        static int GetProcessIdOffset();
        static void SetProcessIdOffset(int process_id_offset);

        static int GetProcessIdLength();
        static void SetProcessIdLength(int process_id_length);
        
        static int GetThreadIdOffset();
        static void SetThreadIdOffset(int thread_id_offset);

        static int GetThreadIdLength();
        static void SetThreadIdLength(int thread_id_length);

        int GetProcessId() const;
        void SetProcessId(int process_id);

        int GetThreadId() const;
        void SetThreadId(int thread_id);

	};

};

#endif

#endif

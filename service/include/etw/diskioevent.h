#ifdef _WIN32


#pragma once

#ifndef PROCESSMONITORING_ETW_DISKIOEVENT_H_
#define PROCESSMONITORING_ETW_DISKIOEVENT_H_

#include "event.h"
#include "wmieventclass.h"

namespace rm
{
	class DiskIoEvent: public Event
	{
	private:
		inline static int transfer_size_offset_ = -1;
		inline static int transfer_size_length_ = -1;

		inline static int thread_id_offset_ = -1;
		inline static int thread_id_length_ = -1;

		unsigned long long transfer_size_ = 0;
		int thread_id_ = -1;

	public:
		DiskIoEvent();
		DiskIoEvent(PEVENT_TRACE p_event, int pointer_size);
		DiskIoEvent(const Event& event, int pointer_size);

		bool FindPropertiesInfo(int pointer_size);
		void UpdateAttributes();

		void SetTransferSize(unsigned long long transfer_size);
		unsigned long long GetTransferSize() const;

		void SetThreadId(int thread_id);
		int GetThreadId() const;

		static void SetTransferSizeOffset(int transfer_size_offset);
		static int GetTransferSizeOffset();
		static void SetTransferSizeLength(int transfer_size_length);
		static int GetTransferSizeLength();

		static void SetThreadIdOffset(int thread_id_offset);
		static int GetThreadIdOffset();
		static void SetThreadIdLength(int thread_id_length);
		static int GetThreadIdLength();

	};

};

#endif

#endif
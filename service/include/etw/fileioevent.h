#pragma once
#ifdef _WIN32


#pragma once

#ifndef PROCESSMONITORING_ETW_FILEIOEVENT_H_
#define PROCESSMONITORING_ETW_FILEIOEVENT_H_

#include "event.h"
#include "wmieventclass.h"

namespace rm
{
	class FileIoEvent : public Event
	{
	private:
		inline static int io_size_offset_ = -1;
		inline static int io_size_length_ = -1;

		inline static int thread_id_offset_ = -1;
		inline static int thread_id_length_ = -1;

		inline static int file_key_offset_ = -1;
		inline static int file_key_length_ = -1;

		inline static int file_name_offset_ = -1;
		inline static int file_name_length_ = -1;

		unsigned long long io_size_ = 0;
		int thread_id_ = -1;
		int file_key_ = -1;
		std::string file_name_;

	public:
		FileIoEvent();
		FileIoEvent(PEVENT_TRACE p_event, int pointer_size);
		FileIoEvent(const Event& event, int pointer_size);

		bool FindPropertiesInfo(int pointer_size);
		void UpdateAttributes();

		void SetIoSize(unsigned long long io_size);
		unsigned long long GetIoSize() const;

		void SetThreadId(int thread_id);
		int GetThreadId() const;

		void SetFileKey(int file_key);
		int GetFileKey() const;

		void SetFileName(std::string file_name);
		std::string GetFileName() const;

		static void SetIoSizeOffset(int io_size_offset);
		static int GetIoSizeOffset();
		static void SetIoSizeLength(int io_size_length);
		static int GetIoSizeLength();

		static void SetThreadIdOffset(int thread_id_offset);
		static int GetThreadIdOffset();
		static void SetThreadIdLength(int thread_id_length);
		static int GetThreadIdLength();

		static void SetFileKeyOffset(int file_key_offset);
		static int GetFileKeyOffset();
		static void SetFileKeyLength(int file_key_length);
		static int GetFileKeyLength();

		static void SetFileNameOffset(int file_name_offset);
		static int GetFileNameOffset();
		static void SetFileNameLength(int file_name_length);
		static int GetFileNameLength();
	};

};

#endif

#endif
#ifdef _WIN32


#pragma once

#ifndef PROCESSMONITORING_ETW_EVENT_H_
#define PROCESSMONITORING_ETW_EVENT_H_

#include "../ulti/everything.h"

namespace rm
{
	class Event
	{
	private:
		std::wstring guid_;
		int version_ = 0;
		int type_ = 0;
		FILETIME ft_ = { 0 };
		ULONGLONG time_stamp_ = 0;
		ULONG mof_length_ = 0;

		PBYTE p_event_data_ = NULL;

	public:
		Event();
		Event(PEVENT_TRACE p_event);

		std::wstring GetGuid() const;
		void SetGuid(std::wstring guid);

		int GetVersion() const;
		void SetVersion(int version_);

		int GetType() const;
		void SetType(int type);

		FILETIME GetFileTime() const;
		void SetFileTime(FILETIME ft);

		ULONGLONG GetTimeStamp() const;
		void SetTimeStamp(ULONGLONG time_stamp);
		ULONGLONG GetTimeInMs() const;

		ULONG GetMofLength() const;
		void SetMofLength(ULONG mof_length);

		PBYTE GetPEventData() const;
		void SetPEventData(PBYTE p_event_data);

	};

	
    class EventType
    {
	public: 
		inline static const std::wstring kThread = L"{3D6FA8D1-FE05-11D0-9DDA-00C04FD7BA7C}";
		inline static const std::wstring kProcess = L"{3D6FA8D0-FE05-11D0-9DDA-00C04FD7BA7C}";
		inline static const std::wstring kDiskIo = L"{3D6FA8D4-FE05-11D0-9DDA-00C04FD7BA7C}";
		inline static const std::wstring kFileIo = L"{90CBDC39-4A3E-11D1-84F4-0000F80464E3}";
	};

	class DiskIoEventType
	{
	public:
		inline static const int kRead = 10;
		inline static const int kWrite = 11;
	};
	
	class FileIoEventType
	{
	public:
		inline static const int kRead = 67;
		inline static const int kWrite = 68;
		inline static const int kName = 0;
		inline static const int kFileCreate = 32;
		inline static const int kFileDelete = 35;
		inline static const int kFileRundown = 36;
	};

};

#endif

#endif
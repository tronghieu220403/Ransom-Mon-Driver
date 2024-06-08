#include "proc-mon.h"


namespace proc_mon
{
	void DrvRegister()
	{
		kRansomPidList = new Vector<eprocess::ProcInfo>();
		kRansomPidMutex.Create();
		NTSTATUS status;

		status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBack, FALSE);
		if (STATUS_SUCCESS != status)
		{
			DebugMessage("Fail to register: %x", status);
		}
	}

	void DrvUnload()
	{
		delete kRansomPidList;
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBack, TRUE);
	}


	void ProcessNotifyCallBack(PEPROCESS eprocess, size_t pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		if (create_info) // Process creation
		{
			if (create_info->ImageFileName == nullptr || create_info->FileOpenNameAvailable == FALSE)
			{
				return;
			}

			String<WCHAR> process_image_name(*(create_info)->ImageFileName);
			if (String<WCHAR>(L"\\??\\").IsPrefixOf(process_image_name))
			{
				process_image_name = &process_image_name[String<WCHAR>(L"\\??\\").Size()];
			}
			// If image has "ransom", add to blacklist
			
		}
		else // Process termination
		{

		}
	}


}


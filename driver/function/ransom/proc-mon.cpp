#include "proc-mon.h"


namespace proc_mon
{
	void DrvRegister()
	{
		kRansomPidList = new Vector<int>();
		kRansomPidMutex.Create();
		NTSTATUS status;

		status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBack, FALSE);
		if (STATUS_SUCCESS != status)
		{
			DebugMessage("Fail to register: %x", status);
		}
		// Need to handle a ransom write to memory of a process -> mark the victim as ransom
	}

	void DrvUnload()
	{
		delete kRansomPidList;
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBack, TRUE);
	}

	void AddPidToRansomPid(size_t pid)
	{
		kRansomPidMutex.Lock();
		kRansomPidList->PushBack(pid);
		kRansomPidMutex.Unlock();
	}

	bool ContainRansomPid(size_t pid)
	{
		bool ans = false;
		kRansomPidMutex.Lock();
		for (int i = 0; i < kRansomPidList->Size(); i++)
		{
			if ((*kRansomPidList)[i] == pid)
			{
				ans = true;
				break;
			}
		}
		kRansomPidMutex.Unlock();
		return ans;
	}

	void DeletePid(size_t pid)
	{
		kRansomPidMutex.Lock();
		for (int i = 0; i < kRansomPidList->Size(); i++)
		{
			if ((*kRansomPidList)[i] == pid)
			{
				kRansomPidList->EraseUnordered(i);
				break;
			}
		}
		kRansomPidMutex.Unlock();
	}


	void ProcessNotifyCallBackEx(PEPROCESS eprocess, size_t pid, PPS_CREATE_NOTIFY_INFO create_info)
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
			if (process_image_name.Find(L"ransom") != static_cast<size_t>(-1))
			{
				DebugMessage("Ransom: %d", pid);
				AddPidToRansomPid(pid);
			}
			
		}
		else // Process termination
		{
			DeletePid(pid);
		}
	}

	bool KillProcess(size_t pid)
	{
		NTSTATUS status;
		PEPROCESS peprocess = nullptr;
		HANDLE process_handle = NULL;
		HANDLE new_process_handle = NULL;

		status = PsLookupProcessByProcessId((HANDLE)pid, &peprocess);
		if (!NT_SUCCESS(status))
		{
			peprocess = nullptr;
			DebugMessage("KillProcess: Fail on the first PsLookupProcessByProcessId %d ", status);
			return false;
		}

		status = ObOpenObjectByPointer(peprocess, 0, NULL, DELETE, *PsProcessType, KernelMode, &process_handle);
		if (!NT_SUCCESS(status))
		{
			DebugMessage("KillProcess: Fail on the first ObOpenObjectByPointer %d ", status);
			if (peprocess)
			{
				ObDereferenceObject(peprocess);
			}
		}

		peprocess = nullptr;
		ObReferenceObjectByHandle(process_handle, 0, *PsProcessType, KernelMode, (PVOID*)&peprocess, NULL);
		if (!NT_SUCCESS(status))
		{
			DebugMessage("KillProcess: Fail on the first ObReferenceObjectByHandle %d ", status);
			peprocess = nullptr;
			return false;
		}
		if (NT_SUCCESS(status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, DELETE, *PsProcessType, KernelMode, &new_process_handle)))
		{
			status = ZwTerminateProcess(new_process_handle, 0);
			if (!NT_SUCCESS(status))
			{
				DebugMessage("KillProcess: Fail on the first ZwTerminateProcess %d ", status);
			}
			ZwClose(new_process_handle);
			ObDereferenceObject(peprocess);
			return true;
		}
		else
		{
			DebugMessage("KillProcess: Fail on the second ObOpenObjectByPointer %d ", status);
		}
		ObDereferenceObject(peprocess);
		return false;

	}

}


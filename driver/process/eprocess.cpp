#include "eprocess.h"

namespace eprocess
{
	ProcInfo::ProcInfo(const PEPROCESS& eproc)
	{
		eproc_ = eproc; 
		pid_ = GetPid();
		active_process_links_ = GetActiveProcessLinks();
		parent_pid_ = 0;
	}

	ProcInfo::ProcInfo(const size_t pid)
	{
		parent_pid_ = 0;
		if (KeGetCurrentIrql() != PASSIVE_LEVEL && KeGetCurrentIrql() != APC_LEVEL)
		{
			// Debug message to know if this error is happening
			return;
		}
		NTSTATUS status;
		status = PsLookupProcessByProcessId((HANDLE)pid, (PEPROCESS*)&eproc_);
		if (NT_SUCCESS(status))
		{
			ObDereferenceObject(eproc_);
			active_process_links_ = PLIST_ENTRY((PUCHAR)eproc_ + kAplRva);
			return;
		}
		eproc_ = nullptr;
		pid_ = 0;
	}

	ProcInfo& ProcInfo::operator=(const ProcInfo& proc)
	{
		eproc_ = proc.eproc_;
		pid_ = proc.pid_;
		active_process_links_ = proc.active_process_links_;
		parent_pid_ = proc.parent_pid_;
		return *this;
	}

	PEPROCESS ProcInfo::GetPeprocess() const
	{
		return eproc_;
	}


	PLIST_ENTRY ProcInfo::GetActiveProcessLinks()
	{
		if (active_process_links_ == nullptr)
		{
			active_process_links_ = (PLIST_ENTRY)((PUCHAR)eproc_ + kAplRva);
		}
		return active_process_links_;
	}

	String<WCHAR> ProcInfo::GetName() const
	{
		return name_;
	}

	size_t ProcInfo::GetPid()
	{
		if (pid_ == 0)
		{
			if (eproc_ != nullptr)
			{
				pid_ = *(size_t*)((PUCHAR)eproc_ + kPidRva);
			}
		}
		return pid_;
	}

	size_t ProcInfo::GetParentPid() const
	{
		return parent_pid_;
	}

	String<WCHAR> ProcInfo::GetProcessImageName() const
	{
		String<WCHAR> process_image_name;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		ULONG returned_length;
		HANDLE h_process = NULL;

		if (eproc_ == NULL || pid_ == NULL)
		{
			return String<WCHAR>();
		}

		PEPROCESS eproc;
		status = PsLookupProcessByProcessId((HANDLE)pid_, &eproc);

		if (!NT_SUCCESS(status))
		{
			return String<WCHAR>();
		}

		status = ObOpenObjectByPointer(eproc,
			0, NULL, 0, 0, KernelMode, &h_process);
		if (!NT_SUCCESS(status))
		{
			DebugMessage("ObOpenObjectByPointer Failed: %08x\n", status);
			return String<WCHAR>();
		}

		ObDereferenceObject(eproc);

		if (ZwQueryInformationProcess == NULL)
		{
			DebugMessage("Cannot resolve ZwQueryInformationProcess\n");
			status = STATUS_UNSUCCESSFUL;
			goto cleanUp;
		}

		/* Query the actual size of the process path */
		status = ZwQueryInformationProcess(h_process,
			ProcessImageFileName,
			NULL, // buffer
			0,    // buffer size
			&returned_length);

		if (STATUS_INFO_LENGTH_MISMATCH != status) {
			DebugMessage("ZwQueryInformationProcess status = %x\n", status);
			goto cleanUp;
		}

		process_image_name.Resize(returned_length);

		if (process_image_name.Data() == NULL)
		{
			goto cleanUp;
		}

		/* Retrieve the process path from the handle to the process */
		status = ZwQueryInformationProcess(h_process,
			ProcessImageFileName,
			(PVOID)process_image_name.Data(),
			returned_length,
			&returned_length);


	cleanUp:

		ZwClose(h_process);

		return process_image_name;
	}


	PEPROCESS ProcInfo::GetNextProc() const
	{
		LIST_ENTRY* apl = active_process_links_->Flink;
		return (PEPROCESS)((PUCHAR)apl - kAplRva);
	}

	PEPROCESS ProcInfo::GetPrevProc() const
	{
		LIST_ENTRY* apl = active_process_links_->Blink;
		return (PEPROCESS)((PUCHAR)apl - kAplRva);
	}

	void ProcInfo::SetNextEntryProc(const PLIST_ENTRY& entry)
	{
		active_process_links_->Flink = entry;
	}

	void ProcInfo::SetPrevEntryProc(const PLIST_ENTRY& entry)
	{
		active_process_links_->Blink = entry;
	}

	void ProcInfo::SetName(const String<WCHAR>& name)
	{
		name_ = name;
	}

	void ProcInfo::SetPid(const size_t pid)
	{
		pid_ = pid;
	}

	void ProcInfo::SetParentPid(const size_t parent_pid)
	{
		parent_pid_ = parent_pid;
	}

	void ProcInfo::DetachFromProcessList()
	{
		kEprocessMutex.Lock();
		if (IsDetached() == false)
		{
			// What ever you do, the value-assign must be as quick as possible or PatchGuard come for BSOD :)))
			PLIST_ENTRY cur = GetActiveProcessLinks();
			RemoveEntryList(cur);
			SetPrevEntryProc(cur);
			SetNextEntryProc(cur);
		}
		kEprocessMutex.Unlock();
	}

	void ProcInfo::JoinToProcessList()
	{
		kEprocessMutex.Lock();
		if (IsDetached())
		{
			// What if SYSTEM_PROCESS_ID is detached? We will have to use ZwQueryObject or PsLookupProcessByProcessId to find a process that is not detached
			ProcInfo proc(ProcInfo(ProcInfo(SYSTEM_PROCESS_ID).GetPrevProc()).GetPrevProc());
			ProcInfo next_proc(proc.GetNextProc());
			PLIST_ENTRY cur = GetActiveProcessLinks();
			PLIST_ENTRY prev = proc.GetActiveProcessLinks();
			PLIST_ENTRY next = prev->Flink;
			// What ever you do, the value-assign must be as quick as possible or PatchGuard come for BSOD :)))
			InsertTailList(prev, cur);
			/*
			cur->Blink = prev;
			cur->Flink = next;
			prev->Flink = cur;
			next->Blink = cur;
			*/
		}
		kEprocessMutex.Unlock();
	}

	bool ProcInfo::IsDetached()
	{
		if (GetNextProc() == GetPeprocess() && GetPrevProc() == GetPeprocess())
		{
			return true;
		}
		return false;
	}

	size_t GetAplRva()
	{
		if (KeGetCurrentIrql() != PASSIVE_LEVEL && KeGetCurrentIrql() != APC_LEVEL)
		{
			// Debug message to know if this is happening
			return 0;
		}

		return GetPidRva() + sizeof(INT_PTR);
	}

	size_t GetPidRva()
	{
		DebugMessage("Finding pid rva.");
		if (KeGetCurrentIrql() != PASSIVE_LEVEL && KeGetCurrentIrql() != APC_LEVEL)
		{
			// Debug message to know if this is happening
			return 0;
		}

		ULONG pid_rva = 0; // The offset we're looking for
		int idx = 0;                // Index 
		ULONG pids[3] = { 0 };				// List of PIDs for our 3 processes
		PEPROCESS eprocs[3] = { 0 };		// Process list, will contain 3 processes

		//Select 3 process PIDs and get their EPROCESS Pointer
		for (int i = 16; idx < 3; i += 4)
		{
			if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)i, &eprocs[idx])))
			{
				pids[idx] = i;
				idx++;
			}
		}

		// Go through the EPROCESS structure and look for the PID
		// we can start at 0x20 because Uniqueproc_Id should
		// not be in the first 0x20 bytes,
		// also we should stop after 0x300 bytes with no success
		for (int i = 0x20; i < 0x1000; i += 4)
		{
			if ((*(ULONG*)((UCHAR*)eprocs[0] + i) == pids[0])
				&& (*(ULONG*)((UCHAR*)eprocs[1] + i) == pids[1])
				&& (*(ULONG*)((UCHAR*)eprocs[2] + i) == pids[2]))
			{
				pid_rva = i;
				break;
			}
		}

		ObDereferenceObject(eprocs[0]);
		ObDereferenceObject(eprocs[1]);
		ObDereferenceObject(eprocs[2]);

		return pid_rva;
	}

	void DrvRegister()
	{
		kPidRva = GetPidRva();
		kAplRva = GetAplRva();
		kEprocessMutex.Create();
	}
}


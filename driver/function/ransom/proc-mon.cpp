#include "proc-mon.h"


namespace proc_mon
{
	void DrvRegister()
	{
		p_manager = new ProcessManager();
		kRansomPidList = new Vector<int>();
		kRansomPidMutex.Create();
		NTSTATUS status;

		status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, FALSE);
		if (STATUS_SUCCESS != status)
		{
			DebugMessage("Fail to register: %x", status);
		}
		// Need to handle a ransom write to memory of a process -> mark the victim as ransom
	}

	void DrvUnload()
	{
		delete p_manager;
		delete kRansomPidList;
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, TRUE);
	}

	void AddPidToRansomPid(size_t pid)
	{
		kRansomPidMutex.Lock();
		kRansomPidList->PushBack(pid);
		kRansomPidMutex.Unlock();
	}

	ProcessManager::ProcessManager() {
		mtx_.Create();
	}

	int ProcessManager::GetProcessInfo(int pid, int info_type) {
		mtx_.Lock();
		if (pid >= processes_.Size() || !processes_[pid].active) {
			mtx_.Unlock();
			return 0;
		}

		if (info_type == GET_PPID) {
			mtx_.Unlock();
			return processes_[pid].ppid;
		}
		else if (info_type == GET_WRITE_BYTES) {
			mtx_.Unlock();
			return processes_[pid].write_bytes;
		}
		mtx_.Unlock();
		return 0;
	}

	void ProcessManager::AddProcess(int pid, int ppid) {
		mtx_.Lock();
		if (pid >= processes_.Size()) {
			processes_.Resize(pid + 1);
		}
		processes_[pid].ppid = ppid;
		processes_[pid].write_bytes = 0;
		processes_[pid].active = true;
		mtx_.Unlock();
		return;
	}

	void ProcessManager::DeleteProcess(int pid) {
		mtx_.Lock();
		if (pid >= processes_.Size() || !processes_[pid].active) {
			mtx_.Unlock();
			return;
		}

		int oldPpid = processes_[pid].ppid;
		int sz = processes_.Size();
		for (int i = 0; i < sz; i++)
		{
			Process process = processes_[i];
			if (process.active && process.ppid == pid) {
				process.ppid = oldPpid;
			}
		}

		processes_[pid] = Process();
		mtx_.Unlock();
		return;
	}

	Vector<int> ProcessManager::GetDescendants(int pid) {
		mtx_.Lock();
		Vector<int> descendants;
		Vector<int> temp_vector;
		size_t index = 0;

		if (pid >= processes_.Size() || !processes_[pid].active) {
			mtx_.Unlock();
			return descendants;
		}
		descendants.PushBack(pid);
		temp_vector.PushBack(pid);

		char* check = new char[processes_.Size()];
		ZeroMemory(check, processes_.Size() * sizeof(char));

		while (index < temp_vector.Size()) {
			int cur_pid = temp_vector[index++];
			for (int i = 0; i < processes_.Size(); ++i) {
				if (processes_[i].active && processes_[i].ppid == cur_pid && !check[i]) {
					descendants.PushBack(i);
					temp_vector.PushBack(i);
					check[i] = true;
				}
			}
		}
		delete[] check;
		mtx_.Unlock();
		return descendants;
	}

	bool ProcessManager::KillProcess(int pid)
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
				DebugMessage("Ransom: %lld", pid);
				p_manager->AddProcess(pid, (int)create_info->ParentProcessId);
				AddPidToRansomPid(pid);
			}
			
		}
		else // Process termination
		{
			DeletePid(pid);
			p_manager->DeleteProcess(pid);
		}
	}

	


	
}


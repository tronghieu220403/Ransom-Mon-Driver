#include "proc-mon.h"

namespace proc_mon
{
	void DrvRegister()
	{
		p_manager = new ProcessManager();
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
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, TRUE);
	}

	void Process::Clean()
	{
		ppid_ = 0;
		forced_ransom_ = false;
	}

	bool Process::Suicide()
	{
		NTSTATUS status;
		PEPROCESS peprocess = nullptr;
		HANDLE process_handle = NULL;
		HANDLE new_process_handle = NULL;

		status = PsLookupProcessByProcessId((HANDLE)pid_, &peprocess);
		if (!NT_SUCCESS(status))
		{
			peprocess = nullptr;
			return false;
		}

		status = ObOpenObjectByPointer(peprocess, 0, NULL, DELETE, *PsProcessType, KernelMode, &process_handle);
		if (!NT_SUCCESS(status))
		{
			if (peprocess)
			{
				ObDereferenceObject(peprocess);
			}
		}

		peprocess = nullptr;
		ObReferenceObjectByHandle(process_handle, 0, *PsProcessType, KernelMode, (PVOID*)&peprocess, NULL);
		if (!NT_SUCCESS(status))
		{
			peprocess = nullptr;
			return false;
		}
		if (NT_SUCCESS(status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, DELETE, *PsProcessType, KernelMode, &new_process_handle)))
		{
			status = ZwTerminateProcess(new_process_handle, 0);
			if (!NT_SUCCESS(status))
			{
				ZwClose(new_process_handle);
				ObDereferenceObject(peprocess);
				return false;
			}
			ZwClose(new_process_handle);
			ObDereferenceObject(peprocess);
			Clean();
			return true;
		}
		ObDereferenceObject(peprocess);
		return false;
	}

	void Process::AddData(const Vector<unsigned char>* data)
	{
		data_analyzer_.AddData(*data);
	}



	ProcessManager::ProcessManager() {
		mtx_.Create();
	}

	int ProcessManager::GetProcessInfo(int pid, int info_type) {
		mtx_.Lock();
		int ret;
		if (pid >= processes_.Size() || processes_[pid] == nullptr) {
			mtx_.Unlock();
			return 0;
		}

		if (info_type == GET_PPID) {
			ret = processes_[pid]->ppid_;
			mtx_.Unlock();
			return ret;
		}
		else if (info_type == GET_WRITE_BYTES) {
			ret = processes_[pid]->data_analyzer_.GetSize();
			mtx_.Unlock();
			return ret;
		}
		mtx_.Unlock();
		return 0;
	}

	void ProcessManager::AddProcess(int pid, int ppid) {
		mtx_.Lock();
		if (pid >= processes_.Size()) {
			processes_.Resize(pid + (size_t)1);
		}
		processes_[pid] = new Process();
		processes_[pid]->pid_ = pid;
		processes_[pid]->ppid_ = ppid;
		mtx_.Unlock();
		return;
	}

	void ProcessManager::DeleteProcess(int pid) {
		mtx_.Lock();
		if (pid >= processes_.Size() || processes_[pid] == nullptr) {
			mtx_.Unlock();
			return;
		}
		int old_ppid = processes_[pid]->ppid_;
		int sz = processes_.Size();
		for (int i = 0; i < sz; i++)
		{
			if (processes_[i] != nullptr && processes_[i]->ppid_ == pid) {
				processes_[i]->ppid_ = old_ppid;
			}
		}
		delete processes_[pid];
		processes_[pid] = nullptr;
		mtx_.Unlock();
		return;
	}

	bool ProcessManager::KillProcess(int pid)
	{
		mtx_.Lock();
		if (pid >= processes_.Size() || processes_[pid] == nullptr) {
			mtx_.Unlock();
			return false;
		}
		bool ret = processes_[pid]->Suicide();
		if (ret == true)
		{
			delete processes_[pid];
			processes_[pid] = nullptr;
		}
		mtx_.Unlock();
		return ret;
	}

	void ProcessManager::AddData(int pid, const Vector<unsigned char>* data)
	{
		mtx_.Lock();
		if (pid >= processes_.Size()) {
			processes_.Resize(pid + (size_t)1);
		}
		if (processes_[pid] == nullptr)
		{
			processes_[pid] = new Process();
		}
		processes_[pid]->pid_ = pid;
		processes_[pid]->AddData(data);
		mtx_.Unlock();
	}

	Vector<int> ProcessManager::GetDescendants(int pid) {
		mtx_.Lock();
		Vector<int> descendants;
		Vector<int> temp_vector;
		size_t index = 0;

		if (pid >= processes_.Size() || processes_[pid] == nullptr) {
			mtx_.Unlock();
			return descendants;
		}
		descendants.PushBack(pid);
		temp_vector.PushBack(pid);

		bool* check = new bool[processes_.Size() + 1];
		ZeroMemory(check, processes_.Size() * sizeof(char));

		while (index < temp_vector.Size()) {
			int cur_pid = temp_vector[index++];
			for (int i = 0; i < processes_.Size(); ++i) {
				if (processes_[i] != nullptr && processes_[i]->ppid_ == cur_pid)
					if (!check[i]) 
					{
						descendants.PushBack(i);
						temp_vector.PushBack(i);
						check[i] = true;
					}
			}
		}
		delete check;
		mtx_.Unlock();
		return descendants;
	}

	void ProcessManager::SetForcedRansomPid(size_t pid)
	{
		mtx_.Lock();
		if (processes_.Size() <= pid)
		{
			processes_.Resize(pid + 1);
			processes_[pid] = new Process();
		}
		processes_[pid]->pid_ = pid;
		processes_[pid]->forced_ransom_ = true;
		mtx_.Unlock();
		Vector<int> ransom_child = p_manager->GetDescendants(pid);
		mtx_.Lock();
		for (int i = 0; i < ransom_child.Size(); i++)
		{
			int id = ransom_child[i];
			processes_[pid]->forced_ransom_ = true;
		}
		mtx_.Unlock();
		return;
	}

	bool ProcessManager::IsProcessRansomware(int pid)
	{
		mtx_.Lock();
		bool ans = false;
		if (pid >= processes_.Size() || processes_[pid] == nullptr)
		{
			mtx_.Unlock();
			return false;
		}
		ans = processes_[pid]->data_analyzer_.IsRandom();
		mtx_.Unlock();
		return ans;
	}

	bool ProcessManager::IsProcessForcedRansomware(int pid)
	{
		mtx_.Lock();
		bool ans = false;
		if (pid >= processes_.Size() || processes_[pid] == nullptr)
		{
			mtx_.Unlock();
			return false;
		}
		ans = processes_[pid]->forced_ransom_;
		mtx_.Unlock();
		return ans;
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
			
			p_manager->AddProcess(pid, (int)create_info->ParentProcessId);
			// If image has "ransom", add to blacklist
			if (process_image_name.Find(L"ransom") != static_cast<size_t>(-1))
			{
				p_manager->SetForcedRansomPid(pid);
				DebugMessage("Forced ransomware: %lld", pid);
			}
			else if (p_manager->IsProcessForcedRansomware((int)create_info->ParentProcessId))
			{
				p_manager->SetForcedRansomPid(pid);
				DebugMessage("Forced ransomware: %lld", pid);
			}
			
		}
		else // Process termination
		{
			p_manager->DeleteProcess(pid);
		}
	}

	

}


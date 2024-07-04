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
	}

	void DrvUnload()
	{
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, TRUE);
		delete p_manager;
	}

	void Process::AddData(const Vector<unsigned char>* data)
	{
		data_analyzer_->AddData(*data);
	}

	ProcessManager::ProcessManager() {
		mtx_.Create();
		processes_.Resize(30000);
	}

	void ProcessManager::AddProcess(int pid, int ppid) {
		mtx_.Lock();
		if (pid >= processes_.Size()) {
			processes_.Resize(pid + (size_t)1);
		}
		if (processes_[pid] != nullptr)
		{
			delete processes_[pid];
		}
		processes_[pid] = new Process();

		processes_[pid]->proc_mtx_ = new Mutex();
		processes_[pid]->proc_mtx_->Create();
		processes_[pid]->data_analyzer_ = new ransom::EntropyAnalyzer();
		processes_[pid]->ppid = ppid;

		mtx_.Unlock();

		
		return;
	}

	void ProcessManager::DeleteProcess(int pid) {
		mtx_.Lock();
		if (pid >= processes_.Size()) {
			mtx_.Unlock();
			return;
		}
		delete processes_[pid];
		processes_[pid] = nullptr;
		mtx_.Unlock();

		return;
	}

	void ProcessManager::AddData(int pid, const Vector<unsigned char>* data)
	{
		mtx_.Lock();
		if (pid < processes_.Size() && processes_[pid]->proc_mtx_ != nullptr)
		{
			processes_[pid]->proc_mtx_->Lock();
			if (processes_[pid]->data_analyzer_ == nullptr) {
				processes_[pid]->data_analyzer_ = new ransom::EntropyAnalyzer();
			}
			processes_[pid]->AddData(data);
			processes_[pid]->proc_mtx_->Unlock();
		}
		mtx_.Unlock();
	}

	bool ProcessManager::IsProcessRansomware(int pid)
	{
		bool ans = false;
		mtx_.Lock();
		if (pid >= processes_.Size())
		{
			mtx_.Unlock();
			return false;
		}
		if (processes_[pid] == nullptr)
		{
			return false;
		}
		mtx_.Unlock();

		processes_[pid]->proc_mtx_->Lock();
		if (processes_[pid]->data_analyzer_ != nullptr)
		{
			ans = processes_[pid]->data_analyzer_->IsRandom();
		}
		processes_[pid]->proc_mtx_->Unlock();

		return ans;
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
			DeleteProcess(pid);
			return true;
		}
		ObDereferenceObject(peprocess);
		return false;
	}

	void ProcessNotifyCallBackEx(PEPROCESS eprocess, int pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		if (create_info) // Process creation
		{	
			if (create_info->ImageFileName == nullptr || create_info->IsSubsystemProcess == TRUE || create_info->FileOpenNameAvailable == FALSE)
			{
				return;
			}
			String<WCHAR> process_image_name(*(create_info)->ImageFileName);
			if (String<WCHAR>(L"\\??\\").IsPrefixOf(process_image_name))
			{
				process_image_name = &process_image_name[String<WCHAR>(L"\\??\\").Size()];
			}
			DebugMessage("%wS", process_image_name.Data());
			com::kComPort->Send(process_image_name.Data(), (process_image_name.Size() + 1)*2);
			p_manager->AddProcess(pid, (int)create_info->ParentProcessId);
			STATUS_TIMEOUT;
		}
		else // Process termination
		{
			p_manager->DeleteProcess(pid);
		}
	}

	

}


#include "proc-mon.h"

namespace proc_mon
{
	void DrvRegister(PDRIVER_OBJECT driver_object)
	{
		p_manager = new ProcessManager();
		proctected_pids = new Vector<int>();
		NTSTATUS status;

		status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, FALSE);
		if (STATUS_SUCCESS != status)
		{
			DebugMessage("Fail to register PsSetCreateProcessNotifyRoutineEx: %x", status);
		}

		OB_CALLBACK_REGISTRATION obRegistration = { 0, };
		OB_OPERATION_REGISTRATION opRegistration[2] = {};

		obRegistration.Version = ObGetFilterVersion();	// Get version
		obRegistration.OperationRegistrationCount = 2;
		RtlInitUnicodeString(&obRegistration.Altitude, L"2204");
		obRegistration.RegistrationContext = NULL;
		
		opRegistration[0].ObjectType = PsProcessType;
		opRegistration[0].Operations = OB_OPERATION_HANDLE_CREATE;
		opRegistration[0].PreOperation = PreObCallback;
		opRegistration[0].PostOperation = nullptr;

		opRegistration[1].ObjectType = PsThreadType;
		opRegistration[1].Operations = OB_OPERATION_HANDLE_CREATE;
		opRegistration[1].PreOperation = PreObCallback;
		opRegistration[1].PostOperation = nullptr;

		obRegistration.OperationRegistration = opRegistration;

		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] ObRegisterCallbacks Test\n");

		status = ObRegisterCallbacks(&obRegistration, &hRegistration);
		if (STATUS_SUCCESS != status)
		{
			DebugMessage("Fail to register ObRegisterCallbacks: %x", status);
		}
	}

	void DrvUnload()
	{
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&proc_mon::ProcessNotifyCallBackEx, TRUE);
		ObUnRegisterCallbacks(hRegistration);
		delete p_manager;
		delete proctected_pids;
	}

	void Process::AddData(const Vector<unsigned char>* data)
	{
		data_analyzer_->AddData(*data);
	}

	ProcessManager::ProcessManager() {
		mtx_.Create();
		processes_.Resize(30000);
	}

	bool ProcessManager::Exist(int pid)
	{
		mtx_.Lock();
		bool ret = false;
		if (pid < processes_.Size() && processes_[pid] != nullptr)
		{
			ret = true;
		}
		mtx_.Unlock();
		return ret;
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
		processes_[pid]->ppid_ = ppid;

		if (ppid != 0 && ppid < processes_.Size() && processes_[ppid] != nullptr)
		{
			processes_[pid]->level_ = processes_[pid]->level_ + 1;
			processes_[ppid]->cpid_->PushBack(pid);
		}

		mtx_.Unlock();

		return;
	}

	void ProcessManager::DeleteProcess(int pid) {
		mtx_.Lock();
		if (pid >= processes_.Size() || processes_[pid] == nullptr) {
			mtx_.Unlock();
			return;
		}

		int ppid = processes_[pid]->ppid_;
		if (ppid != 0 && ppid < processes_.Size() && processes_[ppid] != nullptr)
		{
			for (int i = 0; i < processes_[ppid]->cpid_->Size(); i++)
			{
				if ((*processes_[ppid]->cpid_)[i] == pid)
				{
					processes_[ppid]->cpid_->EraseUnordered(i);
					break;
				}
			}
		}
		for (int i = 0; i < processes_[pid]->cpid_->Size(); i++)
		{
			int child_pid = (*processes_[pid]->cpid_)[i];
			if (child_pid != 0 && child_pid < processes_.Size() && processes_[child_pid] != nullptr)
			{
				processes_[child_pid]->ppid_ = ppid;
			}
		}

		delete processes_[pid];
		processes_[pid] = nullptr;
		mtx_.Unlock();

		return;
	}

	void ProcessManager::FamilyRecursive(int cur_pid, int prev_pid)
	{
		member_.PushBack(cur_pid);
		if (cur_pid < processes_.Size() && processes_[cur_pid] != nullptr)
		{
			for (int i = 0; i < processes_[cur_pid]->cpid_->Size(); i++)
			{
				int child_pid = (*processes_[cur_pid]->cpid_)[i];
				if (child_pid != prev_pid)
				{
					FamilyRecursive(child_pid, cur_pid);
				}
			}
		}
		int ppid = processes_[cur_pid]->ppid_;
		if (ppid != 0 && ppid != prev_pid && ppid < processes_.Size() && processes_[ppid] != nullptr)
		{
			FamilyRecursive(ppid, cur_pid);
		}
	}

	Vector<int> ProcessManager::GetProcessFamily(int pid)
	{
		mtx_.Lock();
		member_.Clear();
		FamilyRecursive(pid, 0);
		Vector<int> ans(member_);
		mtx_.Unlock();
		return ans;
	}

	bool ProcessManager::KillProcessFamily(int pid)
	{
		DebugMessage("Detected a ransom family from pid: %d", pid);
		Vector<int> ans = GetProcessFamily(pid);
		mtx_.Lock();
		int max_level = 0;
		for (int i = 0; i < ans.Size(); i++)
		{
			int id = ans[i];
			if (id < processes_.Size() && processes_[id] != nullptr)
			{
				max_level = max(max_level, processes_[id]->level_);
			}
		}

		for (int level = max_level; level >= 0; level--)
		{
			for (int i = 0; i < ans.Size(); i++)
			{
				int id = ans[i];
				if (id < processes_.Size() && processes_[id] != nullptr && processes_[id]->level_ == level)
				{
					KillProcess(id);
					delete processes_[id];
					processes_[id] = nullptr;
					DebugMessage("Kill ransom pid: %d", id);
				}
			}
		}
		member_.Clear();
		mtx_.Unlock();
		return true;
	}

	void ProcessManager::AddData(int pid, const Vector<unsigned char>* data)
	{
		mtx_.Lock();
		if (pid < processes_.Size() && processes_[pid] != nullptr && processes_[pid]->proc_mtx_ != nullptr)
		{
			test_total_write_ += data->Size();
			processes_[pid]->proc_mtx_->Lock();
			if (processes_[pid]->data_analyzer_ == nullptr) {
				processes_[pid]->data_analyzer_ = new ransom::EntropyAnalyzer();
			}
			processes_[pid]->AddData(data);
			processes_[pid]->proc_mtx_->Unlock();
		}
		mtx_.Unlock();
	}

	void ProcessManager::MarkDeleteOrOverwrite(int pid)
	{
		mtx_.Lock();
		if (pid < processes_.Size() && processes_[pid] != nullptr)
		{
			processes_[pid]->delete_or_overwrite_ = true;
		}
		mtx_.Unlock();
	}

	void ProcessManager::MarkModifyProcMem(int pid)
	{
		mtx_.Lock();
		if (pid < processes_.Size() && processes_[pid] != nullptr)
		{
			processes_[pid]->modify_proc_mem_ = true;
		}
		mtx_.Unlock();
	}

	void ProcessManager::IncHoneyCnt(int pid, const String<WCHAR>& str)
	{
		mtx_.Lock();
		if (pid < processes_.Size() && processes_[pid] != nullptr)
		{
			processes_[pid]->honey_->IncHoneyCnt(str);
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
			mtx_.Unlock();
			return false;
		}
		mtx_.Unlock();

		processes_[pid]->proc_mtx_->Lock();
		if (processes_[pid]->honey_->IsThresholdReached() == true)
		{
			ans = true;
			report_.honey_detected = true;
		}
		if (processes_[pid]->data_analyzer_ != nullptr && processes_[pid]->delete_or_overwrite_ == true)
		{
			ans = processes_[pid]->data_analyzer_->IsRandom();
			if (ans == true)
			{
				report_.entropy_detected = true;
			}
		}
		if (processes_[pid]->modify_proc_mem_ == true)
		{
			ans = true;
			report_.proc_mem_detected = true;
		}
		processes_[pid]->proc_mtx_->Unlock();

		if (ans == true)
		{
			report_.detected = true;
		}

		report_.total_write = test_total_write_;
		return ans;
	}

	void ProcessManager::KillAll()
	{
		mtx_.Lock();
		for (int pid = 0; pid < processes_.Size(); pid++)
		{
			if (processes_[pid] != nullptr)
			{
				KillProcess(pid);
				delete processes_[pid];
				processes_[pid] = nullptr;
				DebugMessage("Killed: %d", pid);
			}
		}
		mtx_.Unlock();
	}

	void ProcessManager::ResetReport()
	{
		report_.detected = 0;
		report_.total_write = 0;
		test_total_write_ = 0;
		report_.honey_detected = false;
		report_.entropy_detected = false;
		report_.proc_mem_detected = false;
	}

	Report ProcessManager::GetReport()
	{
		return report_;
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

		status = ObOpenObjectByPointer(peprocess, OBJ_KERNEL_HANDLE, NULL, DELETE, *PsProcessType, KernelMode, &process_handle);
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
			return true;
		}
		ObDereferenceObject(peprocess);
		return false;
	}

	void ProcessNotifyCallBackEx(PEPROCESS eprocess, int pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		if (ransom::is_enabled == false)
		{
			return;
		}
		if (create_info) // Process creation
		{
			if (create_info->ImageFileName == nullptr || create_info->IsSubsystemProcess == TRUE || create_info->FileOpenNameAvailable == FALSE)
			{
				return;
			}
			String<WCHAR> process_image_name(*(create_info)->ImageFileName);
			if (process_image_name.Find(CODE_FOLDER) != static_cast<size_t>(-1))
			{
				return;
			}
			if (String<WCHAR>(L"\\??\\").IsPrefixOf(process_image_name))
			{
				process_image_name = &process_image_name[String<WCHAR>(L"\\??\\").Size()];
			}

			bool is_valid = true;
			int ppid = (int)create_info->ParentProcessId;

			//DebugMessage("Creating pid %d, name %wS", pid, process_image_name.Data());
			if (p_manager->Exist(ppid))
			{
				DebugMessage("Watch1 ppid %d, pid %d, name %wS", ppid, pid, process_image_name.Data());
				//create_info->CreationStatus = STATUS_ACCESS_DENIED;
				p_manager->AddProcess(pid, ppid);
				return;
			}
			else if (com::kComPort->Send(process_image_name.Data(), (process_image_name.Size() + 1) * 2, &is_valid, sizeof(bool)) == STATUS_SUCCESS)
			{
				if (process_image_name[0] == L'\\' || process_image_name.HasPrefix(L"C:\\Windows\\"))
				{
					return;
				}
				if (is_valid == false)
				{
					DebugMessage("Watch2 ppid %d, pid %d, name %wS", ppid, pid, process_image_name.Data());
					p_manager->AddProcess(pid, 0);
				}
			}
		}
		else // Process termination
		{
			if (p_manager->Exist(pid))
			{
				DebugMessage("Process %d terminated", pid);
				p_manager->DeleteProcess(pid);
			}
		}
	}

	OB_PREOP_CALLBACK_STATUS PreObCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
	{
		UNREFERENCED_PARAMETER(RegistrationContext);
		UNREFERENCED_PARAMETER(pOperationInformation);

		if (ransom::is_enabled == false)
		{
			return OB_PREOP_SUCCESS;
		}

		if (pOperationInformation->KernelHandle)
		{
			return OB_PREOP_SUCCESS;
		}

		long long cur_pid = (long long)PsGetCurrentProcessId();
		long long target_pid = (long long)PsGetProcessId((PEPROCESS)pOperationInformation->Object);

		if (cur_pid == target_pid)
		{
			return OB_PREOP_SUCCESS;
		}


		if (p_manager->Exist(cur_pid) == false)
		{
			return OB_PREOP_SUCCESS;
		}

		if ((pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE))
		{
			if (pOperationInformation->ObjectType == *PsProcessType)
			{
				ACCESS_MASK bits_to_clear = PROCESS_CREATE_PROCESS | PROCESS_CREATE_THREAD | PROCESS_DUP_HANDLE | PROCESS_VM_OPERATION | PROCESS_VM_WRITE;
				if (FlagOn(pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess, bits_to_clear))
				{
					p_manager->MarkModifyProcMem(cur_pid);
					if (p_manager->IsProcessRansomware(cur_pid))
					{
						DebugMessage("Procmem: %lld", cur_pid);
						p_manager->KillProcessFamily(cur_pid);
					}
				}
			}
			for (int i = 0; i < proc_mon::proctected_pids->Size(); i++)
			{
				if ((*proc_mon::proctected_pids)[i] == target_pid)
				{
					pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
				}
			}
		}

		return OB_PREOP_SUCCESS;
	}

	/*
	NTSTATUS ShutDownFunction(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		PIO_STACK_LOCATION stackLocation = NULL;
		stackLocation = IoGetCurrentIrpStackLocation(irp);

		int pid = 0;
		int status = 0;
		if (stackLocation->MajorFunction == IRP_MJ_POWER)
		{
			if (stackLocation->MinorFunction == IRP_MN_SET_POWER)
			{
				pid = (int)PsGetCurrentProcessId();
				if (p_manager->Exist(pid))
				{

				}
			}
		}
		

		irp->IoStatus.Information = 0;
		irp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;

	}
	*/
}


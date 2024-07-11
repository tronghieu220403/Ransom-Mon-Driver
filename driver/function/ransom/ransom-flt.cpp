#include "ransom-flt.h"

namespace ransom
{

	void FltRegister()
	{
		proc_mon::DrvRegister();
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, (PFLT_PRE_OPERATION_CALLBACK)PreWriteOperation, nullptr });
		reg::kFltFuncVector->PushBack({ IRP_MJ_SET_INFORMATION, (PFLT_PRE_OPERATION_CALLBACK)PreWriteOperation, nullptr });

		// TODO: monitor delete/change operation and write to honey_ pot.
		return;
	}

	void FltUnload()
	{
		proc_mon::DrvUnload();
	}

	void DrvRegister()
	{
	}

	void DrvUnload()
	{
	}

	void AddData(int pid, Vector<unsigned char> data)
	{
		proc_mon::p_manager->AddData(pid, &data);
	}

	void IncPidHoneyCnt(int pid)
	{
		proc_mon::p_manager->IncHoneyCnt(pid);
	}

	bool IsPidRansomware(int pid)
	{
		return proc_mon::p_manager->IsProcessRansomware(pid);
	}

	void KillRansomPids(int pid)
	{
		if (proc_mon::p_manager->KillProcess(pid) == false)
		{
			DebugMessage("Failed to kill ransomware process pid: %d", pid);
		}
		else
		{
			proc_mon::p_manager->DeleteProcess(pid);
			DebugMessage("Killed: %d", pid);
		}
	}

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		if (isEnabled == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (proc_mon::p_manager->Exist(pid) == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		
		Vector<unsigned char> write_data;
		ULONG length = data->Iopb->Parameters.Write.Length;
		unsigned char* buffer = nullptr;

		String<WCHAR> file_name = flt::GetFileFullPathName(data);
		if (file_name.Size() != 0)
		{
			if (file_name.Find(HONEY_NAME) != static_cast<size_t>(-1))
			{
				IncPidHoneyCnt(pid);
				goto check_ransom;
			}
		}

		if (data->Iopb->Parameters.Write.MdlAddress != nullptr)
		{
			buffer = (unsigned char*)MmGetSystemAddressForMdlSafe(data->Iopb->Parameters.Write.MdlAddress,
				NormalPagePriority | MdlMappingNoExecute);
			if (buffer == nullptr) {
				data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
				data->IoStatus.Information = 0;
				//DebugMessage("STATUS_INSUFFICIENT_RESOURCES");
				return FLT_PREOP_COMPLETE;
			}
		}
		else
		{
			buffer = (unsigned char*)data->Iopb->Parameters.Write.WriteBuffer;
		}

		write_data.Resize(length);
		__try {
			MemCopy(&write_data[0], buffer, length);
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			data->IoStatus.Status = GetExceptionCode();
			data->IoStatus.Information = 0;
			//DebugMessage("GetExceptionCode(): %llx", GetExceptionCode());

			return FLT_PREOP_COMPLETE;
		}

		AddData(pid, write_data);

		check_ransom:
		if (IsPidRansomware(pid) == true)
		{
			DebugMessage("Ransomware pid detected: %d", pid);
			KillRansomPids(pid);
		}
		
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}

	FLT_POSTOP_CALLBACK_STATUS PostWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		if (isEnabled == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (proc_mon::p_manager->Exist(pid) == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}

		String<WCHAR> file_name = flt::GetFileFullPathName(data);

		if (data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION) {
			switch (data->Iopb->Parameters.SetFileInformation.FileInformationClass) {
			case FileAllocationInformation:
			case FileRenameInformation:
			case FileRenameInformationEx:
			case FileDispositionInformation:
			case FileDispositionInformationEx:
			case FileRenameInformationBypassAccessCheck:
			case FileRenameInformationExBypassAccessCheck:
				if (file_name.Size() != 0)
				{
					if (file_name.Find(HONEY_NAME) != static_cast<size_t>(-1))
					{
						IncPidHoneyCnt(pid);
						if (IsPidRansomware(pid) == true)
						{
							DebugMessage("Ransomware pid detected: %d", pid);
							KillRansomPids(pid);
						}
					}
				}
				break;

			default:
				return FLT_PREOP_SUCCESS_WITH_CALLBACK;
			}
		}

		return FLT_PREOP_SUCCESS_WITH_CALLBACK;

	}

	FLT_POSTOP_CALLBACK_STATUS PostSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{
		return FLT_POSTOP_FINISHED_PROCESSING;
	}


}



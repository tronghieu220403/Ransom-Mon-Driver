#include "ransom-flt.h"

namespace ransom
{

	void FltRegister()
	{
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, (PFLT_PRE_OPERATION_CALLBACK)PreWriteOperation, (PFLT_POST_OPERATION_CALLBACK)PostWriteOperation });
		reg::kFltFuncVector->PushBack({ IRP_MJ_SET_INFORMATION, (PFLT_PRE_OPERATION_CALLBACK)PreSetInfoOperation, (PFLT_POST_OPERATION_CALLBACK)PostSetInfoOperation });
		reg::kFltFuncVector->PushBack({ IRP_MJ_CREATE, (PFLT_PRE_OPERATION_CALLBACK)PreCreateOperation, (PFLT_POST_OPERATION_CALLBACK)PostCreateOperation });
		// TODO: monitor delete/change operation and write to honey_ pot.
		return;
	}

	void FltUnload()
	{

	}

	void DrvRegister()
	{
		proc_mon::DrvRegister();
	}

	void DrvUnload()
	{
		proc_mon::DrvUnload();
	}

	void AddData(int pid, Vector<unsigned char> data)
	{
		proc_mon::p_manager->AddData(pid, &data);
	}

	void IncPidHoneyCnt(int pid, const String<WCHAR>& str)
	{
		proc_mon::p_manager->IncHoneyCnt(pid, str);
	}

	void MarkDeleteOrOverwrite(int pid)
	{
		proc_mon::p_manager->MarkDeleteOrOverwrite(pid);
	}

	bool IsPidRansomware(int pid)
	{
		return proc_mon::p_manager->IsProcessRansomware(pid);
	}

	void KillProcessFamily(int pid)
	{
		if (proc_mon::p_manager->KillProcessFamily(pid) == false)
		{
			DebugMessage("Failed to kill ransomware family");
		}
		else
		{
			DebugMessage("Killed the entire family.");
		}
	}

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		if (is_enabled == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (proc_mon::p_manager->Exist(pid) == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}

		String<WCHAR> file_name = flt::GetFileFullPathName(data);

		if (test_mode == true)
		{
			for (int i = 0; i < WHILE_LIST_SIZE; i++)
			{
				if (file_name.Find(white_list[i]) != static_cast<size_t>(-1))
				{
					data->IoStatus.Status = STATUS_ACCESS_DENIED;
					data->IoStatus.Information = 0;
					return FLT_PREOP_COMPLETE;
				}
			}
		}

		Vector<unsigned char> write_data;
		ULONG length = data->Iopb->Parameters.Write.Length;
		unsigned char* buffer = nullptr;

		if (file_name.Size() != 0)
		{
			if (file_name.Find(HONEY_NAME) != static_cast<size_t>(-1))
			{
				IncPidHoneyCnt(pid, file_name);
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
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			data->IoStatus.Status = GetExceptionCode();
			data->IoStatus.Information = 0;
			//DebugMessage("GetExceptionCode(): %llx", GetExceptionCode());

			return FLT_PREOP_COMPLETE;
		}

		MarkDeleteOrOverwrite(pid);
		AddData(pid, write_data);

	check_ransom:

		if (IsPidRansomware(pid) == true)
		{
			DebugMessage("Entropy: Ransomware pid detected: %d", pid);
			KillProcessFamily(pid);
		}

		//data->Iopb->Parameters.Write.Length = 0;
		//data->Iopb->Parameters.Write.ByteOffset.QuadPart = 0;
		//FltSetCallbackDataDirty(data);
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}

	FLT_POSTOP_CALLBACK_STATUS PostWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		if (is_enabled == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (proc_mon::p_manager->Exist(pid) == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}

		if (data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION) {
			String<WCHAR> file_name = flt::GetFileFullPathName(data);
			if (test_mode == true)
			{
				for (int i = 0; i < WHILE_LIST_SIZE; i++)
				{
					if (file_name.Find(white_list[i]) != static_cast<size_t>(-1))
					{
						data->IoStatus.Status = STATUS_ACCESS_DENIED;
						data->IoStatus.Information = 0;
						return FLT_PREOP_COMPLETE;
					}
				}
			}

			if (data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileAllocationInformation)
			{
				if (((PFILE_ALLOCATION_INFORMATION)(data->Iopb->Parameters.SetFileInformation.InfoBuffer))->AllocationSize.QuadPart == 0)
				{
					MarkDeleteOrOverwrite(pid);
				}
			}
			else
			{
				MarkDeleteOrOverwrite(pid);
			}

			switch (data->Iopb->Parameters.SetFileInformation.FileInformationClass) {

			case FileRenameInformation:
			case FileRenameInformationEx:
			case FileDispositionInformation:
			case FileDispositionInformationEx:
			case FileRenameInformationBypassAccessCheck:
			case FileRenameInformationExBypassAccessCheck:
			case FileAllocationInformation:
				if (file_name.Size() != 0)
				{
					if (file_name.Find(HONEY_NAME) != static_cast<size_t>(-1))
					{
						IncPidHoneyCnt(pid, file_name);
						if (IsPidRansomware(pid) == true)
						{
							DebugMessage("Honey - Ransomware pid detected: %d", pid);
							KillProcessFamily(pid);
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

	FLT_PREOP_CALLBACK_STATUS PreCreateOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		if (is_enabled == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (proc_mon::p_manager->Exist(pid) == false)
		{
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}

		String<WCHAR> file_name = flt::GetFileFullPathName(data);

		if (test_mode == true)
		{
			for (int i = 0; i < WHILE_LIST_SIZE; i++)
			{
				if (file_name.Find(white_list[i]) != static_cast<size_t>(-1))
				{
					data->IoStatus.Status = STATUS_ACCESS_DENIED;
					data->IoStatus.Information = 0;
					return FLT_PREOP_COMPLETE;
				}
			}

		}
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}

	FLT_POSTOP_CALLBACK_STATUS PostCreateOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
}



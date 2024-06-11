#include "ransom-flt.h"

namespace ransom
{

	void FltRegister()
	{
		proc_mon::DrvRegister();
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, (PFLT_PRE_OPERATION_CALLBACK)PreWriteOperation, nullptr });
		reg::kFltFuncVector->PushBack({ IRP_MJ_SET_INFORMATION, (PFLT_PRE_OPERATION_CALLBACK)PreSetInfoOperation, nullptr });
		reg::kFltFuncVector->PushBack({ IRP_MJ_CREATE, (PFLT_PRE_OPERATION_CALLBACK)PreCreateOperation, (PFLT_POST_OPERATION_CALLBACK)PostCreateOperation });
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

	bool IsPidRansomware(int pid)
	{
		return proc_mon::p_manager->IsProcessRansomware(pid);
	}

	void BlockPid(int pid)
	{
		proc_mon::p_manager->SetForcedRansomPid(pid);
		Vector<int> ransom_child = proc_mon::p_manager->GetDescendants(pid);
		DebugMessage("Killing ransomware...");
		for (int i = 0; i < ransom_child.Size(); ++i)
		{
			if (proc_mon::p_manager->KillProcess(ransom_child[i]) == false)
			{
				DebugMessage("Failed to kill ransomware process pid: %d", ransom_child[i]);
			}
		}
		DebugMessage("Finished killing ransomware");
	}

	bool IsPidInBlockedList(int pid)
	{
		return proc_mon::p_manager->IsProcessForcedRansomware(pid);
	}

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));

		// Check if pid in kRansomPidList from proc-mon.h. If not, return FLT_PREOP_SUCCESS_NO_CALLBACK immediately.
		if (proc_mon::p_manager->IsProcessForcedRansomware(pid) == false)
		{
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		}

		if (IsPidInBlockedList(pid) == true)
		{
			data->IoStatus.Status = STATUS_ACCESS_DENIED;
			data->IoStatus.Information = 0;
			return FLT_PREOP_COMPLETE;
		}

		unsigned char* buffer = (unsigned char*)data->Iopb->Parameters.Write.WriteBuffer;
		if (buffer == nullptr)
		{
			buffer = (unsigned char*)MmGetSystemAddressForMdlSafe(data->Iopb->Parameters.Write.MdlAddress, NormalPagePriority | MdlMappingNoExecute);
		}
		ULONG length = data->Iopb->Parameters.Write.Length;
		
		String<WCHAR> file_name = flt::GetFileFullPathName(data).Data();

		Vector<unsigned char> write_data(length);
		MemCopy(&write_data[0], buffer, length);
		AddData(pid, write_data);

		if (IsPidRansomware(pid) == true)
		{
			DebugMessage("Ransomware pid detected: %d", pid);
			BlockPid(pid);
		}
		
		// Intercept the write operation, set length = 0

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));

		// Check if pid in kRansomPidList from proc-mon.h. If not, return FLT_PREOP_SUCCESS_NO_CALLBACK immediately.
		if (proc_mon::p_manager->IsProcessForcedRansomware(pid) == false)
		{
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		}
		
		if (IsPidInBlockedList(pid) == true)
		{
			data->IoStatus.Status = STATUS_ACCESS_DENIED;
			data->IoStatus.Information = 0;
			return FLT_PREOP_COMPLETE;
		}

		/*
		String<WCHAR> file_name = flt::GetFileFullPathName(data).Data();
		size_t size = file::File(file_name).Size();
		if (size == 0)
		{
			DebugMessage("Cannot get file size: %ws",file_name.Data());
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		}
		
		// Intercept the set information operation, set length = 8
		data->Iopb->Parameters.SetFileInformation.FileInformationClass = FileEndOfFileInformation;
		data->Iopb->Parameters.SetFileInformation.Length = sizeof(FILE_END_OF_FILE_INFORMATION);
		FILE_END_OF_FILE_INFORMATION* end_of_file_info = (FILE_END_OF_FILE_INFORMATION*)data->Iopb->Parameters.SetFileInformation.InfoBuffer;
		end_of_file_info->EndOfFile.QuadPart = size;
		*/

		data->Iopb->Parameters.SetFileInformation.FileInformationClass = FileDispositionInformation;
		data->Iopb->Parameters.SetFileInformation.Length = sizeof(FILE_DISPOSITION_INFORMATION);
		FILE_DISPOSITION_INFORMATION* disposition_info = (FILE_DISPOSITION_INFORMATION*)data->Iopb->Parameters.SetFileInformation.InfoBuffer;
		disposition_info->DeleteFile = FALSE;

		FltSetCallbackDataDirty(data);

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	FLT_PREOP_CALLBACK_STATUS PreCreateOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}

	FLT_POSTOP_CALLBACK_STATUS PostCreateOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));

		// Check if pid in kRansomPidList from proc-mon.h. If not, return FLT_PREOP_SUCCESS_NO_CALLBACK immediately.
		if (proc_mon::p_manager->IsProcessForcedRansomware(pid) == false)
		{
			return FLT_POSTOP_FINISHED_PROCESSING;
		}

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

}



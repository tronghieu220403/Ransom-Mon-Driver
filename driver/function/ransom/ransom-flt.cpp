#include "ransom-flt.h"

namespace ransom
{

	inline Map<int, DataAnalyzer*>* kMapPidAna;

	void FltRegister()
	{
		kMapPidAna = new Map<int, DataAnalyzer*>();
		kBlockPid = new Vector<int>();
		kMapMutex.Create();
		kPidMutex.Create();
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, PreWriteOperation, nullptr });
		// Need to handle CHANGE_INFORMATION: block delete, setsize operation but still noti SUCCESS status to the ransom.
		reg::kFltFuncVector->PushBack({ IRP_MJ_SET_INFORMATION, PreSetInfoOperation, nullptr });
		return;
	}

	void FltUnload()
	{
		for (auto it = (*kMapPidAna).Begin(); it != kMapPidAna->End(); it++)
		{
			delete it->second_;
		}
		delete kMapPidAna;
		delete kBlockPid;
	}

	void DrvRegister()
	{
	}

	void DrvUnload()
	{
	}

	void AddData(int pid, Vector<unsigned char> data)
	{
		kMapMutex.Lock();
		if (kMapPidAna->Find(pid) == kMapPidAna->End())
		{
			(*kMapPidAna)[pid] = new DataAnalyzer();
		}
		kMapMutex.Unlock();

		kMapMutex.Lock();
		(*kMapPidAna)[pid]->AddData(data);
		kMapMutex.Unlock();
	}

	bool IsPidRansom(int pid)
	{
		bool ans = false;
		kMapMutex.Lock();
		if (kMapPidAna->Find(pid) == kMapPidAna->End())
		{
			ans = false;
			goto end_func;
		}

		if ((*kMapPidAna)[pid]->GetSize() < 10 * 1024 * 1024)
		{
			ans = false;
			goto end_func;
		}
		ans = (*kMapPidAna)[pid]->IsRandom();

	end_func:
		kMapMutex.Unlock();
		return ans;
	}

	void BlockPid(int pid)
	{
		kPidMutex.Lock();
		//kBlockPid->PushBack(pid);
		kPidMutex.Unlock();
	}

	bool IsPidInBlockedList(int pid)
	{
		bool check = false;
		kPidMutex.Lock();
		for (int i = 0; i < kBlockPid->Size(); i++)
		{
			if ((*kBlockPid)[i] == pid)
			{
				check = true;
				break;
			}
		}
		kPidMutex.Unlock();
		return check;
	}

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));

		// Check if pid in kRansomPidList from proc-mon.h. If not, return FLT_PREOP_SUCCESS_NO_CALLBACK immediately.
		if (proc_mon::ContainRansomPid(pid) == false)
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
		if (IsPidRansom(pid) == true)
		{
			DebugMessage("Ransom: %d", pid);
			BlockPid(pid);
		}
		
		// Intercept the write operation, set length = 0

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));

		// Check if pid in kRansomPidList from proc-mon.h. If not, return FLT_PREOP_SUCCESS_NO_CALLBACK immediately.
		if (proc_mon::ContainRansomPid(pid) == false)
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

}



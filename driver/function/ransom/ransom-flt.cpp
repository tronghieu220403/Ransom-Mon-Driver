#include "ransom-flt.h"

namespace ransom
{

	inline Map<int, DataAnalyzer*>* kMapPidAna;

	void FltRegister()
	{
		DataAnalyzer gg = DataAnalyzer();
		kMapPidAna = new Map<int, DataAnalyzer*>();
		kBlockPid = new Vector<int>();
		kMapMutex.Create();
		kPidMutex.Create();
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, PreOperation, nullptr });
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
		kMapMutex.Unlock();
	end_func:
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

	FLT_PREOP_CALLBACK_STATUS PreOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{
		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		if (IsPidInBlockedList(pid) == true)
		{
			data->IoStatus.Status = STATUS_ACCESS_DENIED;
			data->IoStatus.Information = 0;
			return FLT_PREOP_COMPLETE;
		}
		unsigned char* buffer = (unsigned char*)data->Iopb->Parameters.Write.WriteBuffer;
		if (buffer == nullptr)
		{
			buffer = (unsigned char*)data->Iopb->Parameters.Write.MdlAddress;
		}
		ULONG length = data->Iopb->Parameters.Write.Length;
		
		String<WCHAR> file_name = flt::GetFileFullPathName(data).Data();

		String<WCHAR> tar = L"random_data_10MB.txt";

		if (tar.IsSuffixOf(file_name))
		{
			DebugMessage("Length: %d", length);
			Vector<unsigned char> write_data(length);
			MemCopy(&write_data[0], buffer, length);
			AddData(pid, write_data);
			if (IsPidRansom(pid) == true)
			{
				DebugMessage("Ransom: %d", pid);
				BlockPid(pid);
			}
		}
		
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}


}



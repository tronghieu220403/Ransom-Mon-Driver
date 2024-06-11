#include "ransom-flt.h"

namespace ransom
{

	void FltRegister()
	{
		proc_mon::DrvRegister();
		reg::kFltFuncVector->PushBack({ IRP_MJ_WRITE, (PFLT_PRE_OPERATION_CALLBACK)PreWriteOperation, nullptr });
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

	void KillRansomPids(int pid)
	{
		Vector<int> ransom_child = proc_mon::p_manager->GetDescendants(pid);
		for (int i = 0; i < ransom_child.Size(); ++i)
		{
			if (proc_mon::p_manager->KillProcess(ransom_child[i]) == false)
			{
				DebugMessage("Failed to kill ransomware process pid: %d", ransom_child[i]);
			}
			else
			{
				DebugMessage("Killed: %d", ransom_child[i]);
			}
		}
	}

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _Flt_CompletionContext_Outptr_ PVOID* completion_context)
	{

		int pid = (int)(size_t)PsGetProcessId(IoThreadToProcess(data->Thread));
		ULONG length = data->Iopb->Parameters.Write.Length;
		//DebugMessage("Pid %d with length: %d", pid, length);
		unsigned char* buffer = nullptr;
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

		Vector<unsigned char> write_data(length);
		__try {
			MemCopy(&write_data[0], buffer, length);
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			data->IoStatus.Status = GetExceptionCode();
			data->IoStatus.Information = 0;
			//DebugMessage("GetExceptionCode(): %llx", GetExceptionCode());

			return FLT_PREOP_COMPLETE;
		}

		AddData(pid, write_data);
		
		if (IsPidRansomware(pid) == true)
		{
			DebugMessage("Ransomware pid detected: %d", pid);
			KillRansomPids(pid);
		}

		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}

	FLT_POSTOP_CALLBACK_STATUS PostWriteOperation(_Inout_ PFLT_CALLBACK_DATA data, _In_ PCFLT_RELATED_OBJECTS flt_objects, _In_ PVOID completion_context, _In_ FLT_POST_OPERATION_FLAGS flags)
	{
		// Handle 

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

}



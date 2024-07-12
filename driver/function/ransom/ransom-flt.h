#pragma once

#include "../../std/vector/vector.h"
#include "../../std/string/string.h"
#include "../../std/sync/mutex.h"
#include "../../std/map/map.hpp"
#include "../../std/file/file.h"

#include "../../template/register.h"
#include "../../template/flt-ex.h"
#include "proc-mon.h"
#include "config.h"

#include <wdm.h>
#include <fltKernel.h>

namespace ransom
{
	void FltRegister();
	void FltUnload();
	void DrvRegister();
	void DrvUnload();

	void AddData(int pid, Vector<unsigned char> data);
	void IncPidHoneyCnt(int pid, const String<WCHAR>& str);
	bool IsPidRansomware(int pid);

	void KillRansomPids(int pid);

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

	FLT_POSTOP_CALLBACK_STATUS PostWriteOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _In_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags);

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

	FLT_POSTOP_CALLBACK_STATUS PostSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _In_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags);

};


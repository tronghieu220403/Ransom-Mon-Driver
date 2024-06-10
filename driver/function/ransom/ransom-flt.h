#pragma once

#include "../../std/vector/vector.h"
#include "../../std/string/string.h"
#include "../../std/sync/mutex.h"
#include "../../std/map/map.hpp"
#include "../../std/file/file.h"

#include "../../template/register.h"
#include "../../template/flt-ex.h"
#include "proc-mon.h"

#include <wdm.h>
#include <fltKernel.h>

namespace ransom
{
	void FltRegister();
	void FltUnload();
	void DrvRegister();
	void DrvUnload();

	void AddData(int pid, Vector<unsigned char> data);
	bool IsPidRansomware(int pid);

	void BlockPid(int pid);

	bool IsPidInBlockedList(int pid);

	FLT_PREOP_CALLBACK_STATUS PreWriteOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

	FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

};


#pragma once

#include "../../std/vector/vector.h"
#include "../../std/string/string.h"
#include "../../std/sync/mutex.h"
#include "../../std/map/map.hpp"

#include "../../process/ps-monitor.h"
#include "../../template/register.h"
#include "../../template/flt-ex.h"

#include "watcher.h"

#include <wdm.h>
#include <fltKernel.h>

namespace ransom
{
	inline Vector<int>* kBlockPid;

	extern inline Mutex kMapMutex = Mutex();
	extern inline Mutex kPidMutex = Mutex();

	void FltRegister();
	void FltUnload();
	void DrvRegister();
	void DrvUnload();

	void AddData(int pid, Vector<unsigned char> data);
	bool IsPidRansom(int pid);

	void BlockPid(int pid);

	bool IsPidInBlockedList(int pid);

	FLT_PREOP_CALLBACK_STATUS PreOperation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

};


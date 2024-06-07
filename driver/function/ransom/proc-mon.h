#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"

#include "../../process/ps-monitor.h"

#include "../../template/register.h"
#include "../../template/flt-ex.h"

namespace proc_mon
{
	extern inline Vector<eprocess::ProcInfo>* kRansomPidList = nullptr;
	inline Mutex kRansomPidMutex = Mutex();

	void DrvRegister();
	void DrvUnload();

	void AddPidToRansomPid(size_t);

	void ContainRansomPid(size_t);

	void DeletePid(size_t);

	void ProcessNotifyCallBack(PEPROCESS, size_t, PPS_CREATE_NOTIFY_INFO);

	void KillProcess(size_t);

}

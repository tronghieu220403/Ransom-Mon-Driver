#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"

#include "../../process/ps-monitor.h"
#include "watcher.h"

#include "../../template/register.h"
#include "../../template/flt-ex.h"

#define GET_PPID 1
#define GET_WRITE_BYTES 2

namespace proc_mon
{
    struct Process {
        int ppid;
        long long write_bytes;
        bool active;
        Process() : ppid(0), write_bytes(0), active(false) {}
    };

    class ProcessManager {
    private:
        Vector<Process> processes_;
        Mutex mtx_;
        ransom::DataAnalyzer data_analyzer_;
    public:
        ProcessManager();
        int GetProcessInfo(int pid, int info_type);
        void AddProcess(int pid, int ppid);
        void DeleteProcess(int pid);
        Vector<int> GetDescendants(int pid);
        bool KillProcess(int pid);
    };

    extern inline ProcessManager* p_manager = nullptr;
	extern inline Vector<int>* kRansomPidList = nullptr;
	inline Mutex kRansomPidMutex = Mutex();

	void DrvRegister();
	void DrvUnload();

	void AddPidToRansomPid(size_t);

	bool ContainRansomPid(size_t);

	void DeletePid(size_t);

	void ProcessNotifyCallBackEx(PEPROCESS, size_t, PPS_CREATE_NOTIFY_INFO);


    

}

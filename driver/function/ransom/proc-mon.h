#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"

#include "watcher.h"

#define GET_PPID 1
#define GET_WRITE_BYTES 2

namespace proc_mon
{
    struct Process {
        int pid_;
        int ppid_;
        bool forced_ransom_;
        ransom::DataAnalyzer data_analyzer_;
        
        Process() : pid_(0), ppid_(0), forced_ransom_(false), data_analyzer_(){};

        void Clean();
        bool Suicide();

        void AddData(const Vector<unsigned char>* data);
    };

    class ProcessManager {
    private:
        Vector<Process*> processes_;
        Mutex mtx_;
    public:
        ProcessManager();
        int GetProcessInfo(int pid, int info_type);
        
        void AddProcess(int pid, int ppid);
        void DeleteProcess(int pid);
        bool KillProcess(int pid);

        void AddData(int pid, const Vector<unsigned char>* data);

        Vector<int> GetDescendants(int pid);

        void SetForcedRansomPid(size_t pid);
        bool IsProcessRansomware(int pid);
        bool IsProcessForcedRansomware(int pid);

    };

    extern inline ProcessManager* p_manager = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, size_t, PPS_CREATE_NOTIFY_INFO);


    

}

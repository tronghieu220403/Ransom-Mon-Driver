#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"

#include "watcher.h"

namespace proc_mon
{
    struct Process {
        int pid_;
        int ppid_;
        ransom::DataAnalyzer data_analyzer_;
        
        Process() : pid_(0), ppid_(0), data_analyzer_(){};

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
        
        void AddProcess(int pid, int ppid);
        void DeleteProcess(int pid);
        bool KillProcess(int pid);

        void AddData(int pid, const Vector<unsigned char>* data);

        Vector<int> GetDescendants(int pid);

        bool IsProcessRansomware(int pid);

    };

    extern inline ProcessManager* p_manager = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);
    
}

#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"

#include "watcher.h"

namespace proc_mon
{
    struct Process {

        Mutex* proc_mtx_;
        ransom::DataAnalyzer* data_analyzer_;
        
        void AddData(const Vector<unsigned char>* data);

        Process() : data_analyzer_(nullptr), proc_mtx_(nullptr){
        };

        ~Process() {
            delete proc_mtx_;
            delete data_analyzer_;
        }
    };

    class ProcessManager {
    private:
        Vector<Process> processes_;
        Mutex mtx_;
    public:
        ProcessManager();
        
        void AddProcess(int pid);
        void DeleteProcess(int pid);
        bool KillProcess(int pid);

        void AddData(int pid, const Vector<unsigned char>* data);

        bool IsProcessRansomware(int pid);

    };

    extern inline ProcessManager* p_manager = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);
    
}

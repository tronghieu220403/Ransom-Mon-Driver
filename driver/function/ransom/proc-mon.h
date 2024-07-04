#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"
#include "../../com/comport/comport.h"

#include "watcher.h"

namespace proc_mon
{
    struct Process {

        Mutex* proc_mtx_;
        ransom::EntropyAnalyzer* data_analyzer_;
        
        int ppid = 0;

        void AddData(const Vector<unsigned char>* data);

        Process() : data_analyzer_(nullptr), proc_mtx_(nullptr){
        };

        ~Process() {
            delete proc_mtx_;
            proc_mtx_ = nullptr;
            delete data_analyzer_;
            proc_mtx_ = nullptr;
        }
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

        bool IsProcessRansomware(int pid);

    };

    extern inline ProcessManager* p_manager = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);
    

}

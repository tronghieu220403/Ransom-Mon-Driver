#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"
#include "../../com/comport/comport.h"

#include "config.h"
#include "watcher.h"

namespace proc_mon
{
    struct Process {

        Mutex* proc_mtx_;
        ransom::EntropyAnalyzer* data_analyzer_;
        ransom::HoneyAnalyzer* honey_;

        int ppid_ = 0;
        bool delete_or_overwrite_ = 0;

        void AddData(const Vector<unsigned char>* data);

        Process() : data_analyzer_(nullptr), proc_mtx_(nullptr), honey_(nullptr){
        };

        ~Process() {
            delete proc_mtx_;
            proc_mtx_ = nullptr;
            delete data_analyzer_;
            proc_mtx_ = nullptr;
            delete honey_;
            honey_ = nullptr;
        }
    };

    class ProcessManager {
    private:
        Vector<Process*> processes_;
        Mutex mtx_;
    public:
        ProcessManager();
        
        bool Exist(int pid);

        void AddProcess(int pid, int ppid);
        void DeleteProcess(int pid);
        bool KillProcess(int pid);

        void AddData(int pid, const Vector<unsigned char>* data);
        void MarkDeleteOrOverwrite(int pid);
        void IncHoneyCnt(int pid);

        bool IsProcessRansomware(int pid);

        void KillAll();
    };

    extern inline ProcessManager* p_manager = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);
    

}

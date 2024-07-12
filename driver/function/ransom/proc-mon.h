#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"
#include "../../com/comport/comport.h"

#include "config.h"
#include "watcher.h"


#define PROCESS_TERMINATE       0x0001	// TerminateProcess
#define PROCESS_VM_OPERATION    0x0008	// VirtualProtect, WriteProcessMemory
#define PROCESS_VM_READ         0x0010	// ReadProcessMemory
#define PROCESS_VM_WRITE        0x0020	// WriteProcessMemory

namespace proc_mon
{
    struct Report
    {
        bool detected = 0;
        unsigned long long total_write = 0;

        int honey_detected = false;
        int entropy_detected = false;
        int proc_mem_detected = false;
    };

    struct Process {

        Mutex* proc_mtx_;
        ransom::EntropyAnalyzer* data_analyzer_;
        ransom::HoneyAnalyzer* honey_;

        Vector<int>* cpid_;
        int ppid_ = 0;
        int level_ = 0;
        bool delete_or_overwrite_ = false;
        bool modify_proc_mem_ = false;

        void AddData(const Vector<unsigned char>* data);

        Process()
        {
            proc_mtx_ = new Mutex();
            proc_mtx_->Create();
            data_analyzer_ = new ransom::EntropyAnalyzer();
            ppid_ = 0;
            level_ = 0;
            delete_or_overwrite_ = false;
            modify_proc_mem_ = false;
            honey_ = new ransom::HoneyAnalyzer();
            cpid_ = new Vector<int>();
        }

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
        
        unsigned long long test_total_write_ = 0;
        Report report_;

        Vector<int> member_;
        void FamilyRecursive(int cur, int prev);

    public:
        ProcessManager();
        
        bool Exist(int pid);

        void AddProcess(int pid, int ppid);
        void DeleteProcess(int pid);
        Vector<int> GetProcessFamily(int pid);
        bool KillProcessFamily(int pid);
        bool KillProcess(int pid);

        void AddData(int pid, const Vector<unsigned char>* data);
        void MarkDeleteOrOverwrite(int pid);
        void MarkModifyProcMem(int pid);
        void IncHoneyCnt(int pid, const String<WCHAR>& str);

        bool IsProcessRansomware(int pid);

        void KillAll();

        void ResetReport();
        Report GetReport();
    };

    extern inline ProcessManager* p_manager = nullptr;
    inline PVOID hRegistration = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);

    OB_PREOP_CALLBACK_STATUS PreObCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);

}

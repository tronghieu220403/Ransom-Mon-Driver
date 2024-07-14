#pragma once

#include "../../std/string/string.h"
#include "../../std/vector/vector.h"
#include "../../std/sync/mutex.h"
#include "../../com/comport/comport.h"

#include "config.h"
#include "watcher.h"


#define THREAD_TERMINATE                 (0x0001)  
#define THREAD_SUSPEND_RESUME            (0x0002)  
#define THREAD_GET_CONTEXT               (0x0008)  
#define THREAD_SET_CONTEXT               (0x0010)  
#define THREAD_SET_INFORMATION           (0x0020)  
#define THREAD_SET_THREAD_TOKEN          (0x0080)
#define THREAD_IMPERSONATE               (0x0100)
#define THREAD_DIRECT_IMPERSONATION      (0x0200)
#define THREAD_SET_LIMITED_INFORMATION   (0x0400)
#define THREAD_RESUME                    (0x1000)

#define PROCESS_TERMINATE                  (0x0001)  
#define PROCESS_CREATE_THREAD              (0x0002)  
#define PROCESS_SET_SESSIONID              (0x0004)  
#define PROCESS_VM_OPERATION               (0x0008)  
#define PROCESS_VM_READ                    (0x0010)  
#define PROCESS_VM_WRITE                   (0x0020)  
#define PROCESS_DUP_HANDLE                 (0x0040)  
#define PROCESS_CREATE_PROCESS             (0x0080)  
#define PROCESS_SET_QUOTA                  (0x0100)  
#define PROCESS_SET_INFORMATION            (0x0200)  
#define PROCESS_SUSPEND_RESUME             (0x0800)  
#define PROCESS_SET_LIMITED_INFORMATION    (0x2000)  

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
	extern inline Vector<int>* proctected_pids = nullptr;
    inline PVOID hRegistration = nullptr;

	void DrvRegister();
	void DrvUnload();

	void ProcessNotifyCallBackEx(PEPROCESS, int, PPS_CREATE_NOTIFY_INFO);

    OB_PREOP_CALLBACK_STATUS PreObCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);

}

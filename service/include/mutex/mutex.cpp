#include "mutex.h"

namespace rm
{
    NamedMutex::NamedMutex(const std::string& mutex_name):
        mutex_name_(mutex_name)
    {
        SetMutex(mutex_name);
    }

    void NamedMutex::SetMutex(const std::string& mutex_name)
    {
        #ifdef _WIN32
        Close();
        mutex_name_ = mutex_name;
        _SECURITY_ATTRIBUTES sec;
        sec.nLength = sizeof(_SECURITY_ATTRIBUTES);
        sec.bInheritHandle = TRUE;
        sec.lpSecurityDescriptor = nullptr;
        handle_mutex_ = CreateMutexA(
                            &sec,
                            FALSE, 
                            mutex_name.size() != 0 ? &mutex_name[0] : nullptr
                        );
        #elif __linux__
        Close();
        mutex_name_ = mutex_name;
        p_sema_ = &sema_;
        if (mutex_name.size() != 0)
        {
            std::string name = "/" + mutex_name;
            p_sema_ = sem_open(name.data(), O_RDWR | O_CREAT, 0660, 1);
            if (p_sema_ == SEM_FAILED){
                p_sema_ = sem_open(name.data(), O_RDWR);
                return;
            }
        }
        else
        {
            sem_init(p_sema_, 0, 1);
        }
        #endif
    }

    std::string NamedMutex::GetMutexName() const
    {
        return mutex_name_;
    }

    void NamedMutex::Lock()
    {
        #ifdef _WIN32
            WaitForSingleObject(handle_mutex_, INFINITY);
        #elif __linux__
            sem_wait(p_sema_);
        #endif
    }

    void NamedMutex::Unlock()
    {
        #ifdef _WIN32
            ReleaseMutex(handle_mutex_);
        #elif __linux__
            sem_post(p_sema_);
        #endif
    }

    void NamedMutex::Close()
    {
        #ifdef _WIN32
            if (handle_mutex_ != nullptr)
            {
                CloseHandle(handle_mutex_);
                handle_mutex_ = 0;
            }
        #elif __linux__
            if (p_sema_ != nullptr)
            {
                sem_destroy(p_sema_);
                p_sema_ = 0;
                ZeroMem(&sema_, sizeof(sema_));
            }
        #endif
    }

    NamedMutex::~NamedMutex()
    {
        Close();
    }
}

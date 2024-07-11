
#ifndef PROCESSMONITORING_COMMUNICATION_MUTEX_MUTEX_H_
#define PROCESSMONITORING_COMMUNICATION_MUTEX_MUTEX_H_

#include "../ulti/everything.h"

namespace rm
{
    class NamedMutex
    {
    private:
        std::string mutex_name_;

        #ifdef _WIN32
            HANDLE handle_mutex_ = nullptr;
        #elif __linux__
            sem_t *p_sema_;
            sem_t sema_;
        #endif
    public:
        NamedMutex() = default;
        explicit NamedMutex(const std::string& mutex_name);
        
        void SetMutex(const std::string& mutex_name);

        std::string GetMutexName() const;

        void Lock();
        void Unlock();

        void Close();

        ~NamedMutex();
    };

}

#endif
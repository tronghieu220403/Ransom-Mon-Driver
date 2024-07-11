#ifndef RANSOMMON_RANSOMMON
#define RANSOMMON_RANSOMMON

#include "include/service/ransom-serv.h"
#include "include/service/servicecontrol.h"
#include "include/service/service.h"
#include "include/file/file.h"

void VerifySignature()
{
    HANDLE hPort;
    HRESULT hr;
    COMPORT_MESSAGE buffer, reply;

    while (true)
    {
        hr = FilterConnectCommunicationPort(PORT_NAME, 0, NULL, 0, NULL, &hPort);
        if (FAILED(hr))
        {
            Sleep(100);
            continue;
        }

        while (TRUE)
        {
            hr = FilterGetMessage(hPort, (PFILTER_MESSAGE_HEADER)&buffer, sizeof(COMPORT_MESSAGE), NULL);
            if (SUCCEEDED(hr))
            {
                bool is_valid = VerifyEmbeddedSignature(buffer.data);
                reply.header = buffer.header;
                reply.data[0] = is_valid;
                if (!SUCCEEDED(FilterReplyMessage(hPort, (PFILTER_REPLY_HEADER)&reply, sizeof(FILTER_MESSAGE_HEADER) + sizeof(bool))))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        CloseHandle(hPort);
        Sleep(100);
    }
}

LPVOID ServiceMainWorker()
{

    std::jthread verify_sig(&VerifySignature);
    verify_sig.join();

    return nullptr;
}

int main()
{
    rm::ServiceControl service_control(L"RanMon");

    std::wstring w_path;
    w_path.resize(1000);
    GetModuleFileNameW(nullptr, &w_path[0], 1000);
    w_path.resize(wcslen(&w_path[0]));
    service_control.SetPath(w_path);
    if (service_control.Create() == true)
    {
        service_control.Run();
    }
    else
    {
        return 0;
    }

    rm::RansomwareMonitoringService::SetMainWorkerFunction(ServiceMainWorker);
    rm::Service sc;
    sc.SetName(L"aaaRanMon");
    sc.SetServiceMainFunc((LPSERVICE_MAIN_FUNCTION)rm::RansomwareMonitoringService::RansomwareMonitoringServiceMain);
    sc.Start();

    ServiceMainWorker();
    return 0;
}

#endif


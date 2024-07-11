#include "file.h"


bool VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    LONG lStatus;
    bool ret = false;

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;
    memset(&WinTrustData, 0, sizeof(WinTrustData));
    WinTrustData.cbStruct = sizeof(WinTrustData);
    WinTrustData.pPolicyCallbackData = NULL;
    WinTrustData.pSIPClientData = NULL;
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    WinTrustData.hWVTStateData = NULL;
    WinTrustData.pwszURLReference = NULL;
    WinTrustData.dwUIContext = 0;
    WinTrustData.pFile = &FileData;
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus)
    {
    case ERROR_SUCCESS:
        ret = true;
        break;

    default:
        break;
    }

    WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    return ret;
}

wstring GetDosPath(const wstring* wstr)
{
    wstring device_name = wstr->substr(0, wstr->find_first_of('\\'));
    wstring dos_name;
    dos_name.resize(128);
    DWORD status;
    while (QueryDosDeviceW(device_name.data(), (WCHAR*)dos_name.data(), dos_name.size()) == 0)
    {
        status = GetLastError();
        if (status != ERROR_INSUFFICIENT_BUFFER)
        {
            return wstring();
        }
        dos_name.resize(dos_name.size() * 2);
    }
    dos_name.resize(wcslen(dos_name.data()));

    return dos_name + wstr->substr(wstr->find_first_of('\\'));
}


std::wstring getRandomFile(const std::wstring& directoryPath) {

    // Check if the directory exists
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        return wstring();
    }

    // Iterate over the directory and collect all files
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry.status())) {
            return entry.path().wstring();
        }
    }

    return wstring();
}

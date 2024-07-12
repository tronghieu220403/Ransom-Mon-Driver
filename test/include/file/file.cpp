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


std::string getRandomFile(const std::string& directoryPath) {

    // Check if the directory exists
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        return string();
    }

    // Iterate over the directory and collect all files
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry.status())) {
            return entry.path().string();
        }
    }

    return string();
}

string copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        // Check if source file exists and is a file
        if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
            throw std::runtime_error("Source file does not exist or is not a file");
        }

        // Check if destination directory exists, if not, create it
        if (!fs::exists(destinationPath)) {
            fs::create_directories(destinationPath);
        }

        // Construct the destination file path
        fs::path source(sourcePath);
        fs::path destination(destinationPath);
        destination /= source.filename();

        // Copy the file
        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        std::cout << "File copied successfully to " << destination.string() << std::endl;
        return destination.string();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "";
    }
}

void copyDirectory(const fs::path& source, const fs::path& destination) {
    try {
        // Check if source directory exists and is a directory
        if (!fs::exists(source) || !fs::is_directory(source)) {
            throw std::runtime_error("Source directory does not exist or is not a directory");
        }

        // Remove the destination directory if it exists
        if (fs::exists(destination)) {
            fs::remove_all(destination);
        }

        // Create the destination directory
        fs::create_directories(destination);

        // Iterate over the source directory
        for (const auto& entry : fs::recursive_directory_iterator(source)) {
            const auto& path = entry.path();
            auto relativePath = fs::relative(path, source);
            auto dest = destination / relativePath;

            if (fs::is_directory(path)) {
                // Create directory at destination if it is a directory
                fs::create_directories(dest);
            }
            else if (fs::is_regular_file(path)) {
                // Copy file to destination if it is a regular file
                fs::copy_file(path, dest, fs::copy_options::overwrite_existing);
            }
        }

        std::cout << "Directory copied successfully from " << source << " to " << destination << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void moveFile(const fs::path& sourcePath, const fs::path& destinationDir) {
    try {
        // Check if source file exists
        if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
            throw std::runtime_error("Source file does not exist or is not a file");
        }

        // Check if destination directory exists, if not, create it
        if (!fs::exists(destinationDir)) {
            fs::create_directories(destinationDir);
        }

        // Construct the full destination path
        fs::path destinationPath = destinationDir / sourcePath.filename();

        // Move the file
        fs::rename(sourcePath, destinationPath);

        std::cout << "File moved successfully from " << sourcePath << " to " << destinationPath << std::endl;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

std::string getFileNameWithoutExtension(const std::string& filePath) {
    fs::path path(filePath);
    return path.stem().string();
}
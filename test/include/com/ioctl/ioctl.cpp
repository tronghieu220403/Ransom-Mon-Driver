#include "ioctl.h"

bool Ioctl::Create()
{
	device_ = CreateFileW(HIEU_DEVICE, GENERIC_WRITE | GENERIC_READ | GENERIC_EXECUTE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	return device_ != INVALID_HANDLE_VALUE;
    1
}

vector<unsigned char> Ioctl::StartMonitor()
{
    DWORD bytes_returned = 0;
    const int output_sz = 20000;
    vector<unsigned char> output;
    output.resize(output_sz);
    BOOL status = FALSE;
    vector<char> in_buffer(sizeof(IOCTL_CMD));

    IOCTL_CMD* cmd = (IOCTL_CMD*)&in_buffer[0];
    cmd->cmd_class = kTestEnableRansom;
    cmd->data_len = 0;

    status = DeviceIoControl(device_, IOCTL_HIEU, &in_buffer[0], in_buffer.size() * sizeof(char), output.data(), sizeof(output_sz), &bytes_returned, (LPOVERLAPPED)NULL);

    output.resize(bytes_returned);
    return output;
}

vector<unsigned char> Ioctl::StopMonitor()
{
    DWORD bytes_returned = 0;
    const int output_sz = 20000;
    vector<unsigned char> output;
    output.resize(output_sz);
    BOOL status = FALSE;
    vector<char> in_buffer(sizeof(IOCTL_CMD));

    IOCTL_CMD* cmd = (IOCTL_CMD*)&in_buffer[0];
    cmd->cmd_class = kTestDisableRansom;
    cmd->data_len = 0;

    status = DeviceIoControl(device_, IOCTL_HIEU, &in_buffer[0], in_buffer.size() * sizeof(char), output.data(), sizeof(output_sz), &bytes_returned, (LPOVERLAPPED)NULL);

    output.resize(bytes_returned);
    return output;
}

void Ioctl::Close()
{
    CloseHandle(device_);
    device_ = INVALID_HANDLE_VALUE;
}

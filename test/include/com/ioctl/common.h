#include <Windows.h>

#define IOCTL_HIEU CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2204, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define HIEU_DEVICE L"\\\\.\\HieuDeviceLink"

enum IOCTL_CMD_CLASS
{
	kHideFile = 0x80002000,
	kHideDir = 0x80002001,
	kUnhideFile = 0x80002002,
	kUnhideDir = 0x80002003,
	kProctectFile = 0x80002004,
	kUnproctectFile = 0x80002005,
	kProctectDir = 0x80002006,
	kUnproctectDir = 0x80002007,
	kHideReg = 0x80002008,
	kUnhideReg = 0x80002009,
	kHideProcId = 0x8000200A,
	kUnhideProcId = 0x8000200B,
	kProtectProcId = 0x8000200C,
	kUnprotectProcId = 0x8000200D,
	kHideProcImage = 0x8000200E,
	kUnhideProcImage = 0x80002010,
	kProtectProcImage = 0x80002011,
	kUnprotectProcImage = 0x80002012,
	kEnableRansom = 0x80002013,
	kDisableRansom = 0x80002014,

	//Testing purpose
	kTestEnableRansom = 0x80002015,
	kTestDisableRansom = 0x80002016,
	kTestDir = 0x80002017
};

struct IOCTL_CMD
{
	IOCTL_CMD_CLASS cmd_class;
	size_t data_len;
	char data[1];
};

struct Report
{
	bool detected = 0;
	unsigned long long total_write = 0;

	int honey_detected = false;
	int entropy_detected = false;
	int proc_mem_detected = false;
};
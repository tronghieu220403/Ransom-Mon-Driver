#include "ioctl.h"

NTSTATUS ioctl::DrvRegister(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ioctl::HandleIoctl;

	// routines that will execute once a handle to our device's symbolik link is opened/closed
	driver_object->MajorFunction[IRP_MJ_CREATE] = ioctl::MajorFunction;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = ioctl::MajorFunction;

	NTSTATUS status = IoCreateDevice(driver_object, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &driver_object->DeviceObject);

	if (!NT_SUCCESS(status))
	{
		DebugMessage("Could not create device %wZ", DEVICE_NAME);
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		DebugMessage("Device %wZ created", DEVICE_NAME);
	}

	status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_NAME, &DEVICE_NAME);
	if (NT_SUCCESS(status))
	{
		DebugMessage("Symbolic link %wZ created", DEVICE_SYMBOLIC_NAME);
	}
	else
	{
		DebugMessage("Error creating symbolic link %wZ", DEVICE_SYMBOLIC_NAME);
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

NTSTATUS ioctl::DrvUnload(PDRIVER_OBJECT driver_object)
{
	DebugMessage("DriverUnload: Unload\n");

	IoDeleteDevice(driver_object->DeviceObject);
	IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);

	return STATUS_SUCCESS;
}

NTSTATUS ioctl::HandleIoctl(PDEVICE_OBJECT device_object, PIRP irp)
{
	UNREFERENCED_PARAMETER(device_object);

	PIO_STACK_LOCATION stack_loc = IoGetCurrentIrpStackLocation(irp);
	stack_loc = IoGetCurrentIrpStackLocation(irp);

	IOCTL_CMD* cmd = (IOCTL_CMD*)irp->AssociatedIrp.SystemBuffer;

	bool test_start_reply = false;
	proc_mon::Report report;

	if (stack_loc->Parameters.DeviceIoControl.IoControlCode != IOCTL_HIEU)
	{
		return STATUS_UNSUCCESSFUL;
	}

	IOCTL_CMD_CLASS type = cmd->cmd_class;
	// TODO: Build a function to handle each type
	String<WCHAR> str;
	int pid;
	switch (type)
	{
	case IOCTL_CMD_CLASS::kHideFile:

		str = cmd->ParseHideFile().file_path;
		break;

	case IOCTL_CMD_CLASS::kHideDir:

		str = cmd->ParseHideDir().dir_path;
		break;

	case IOCTL_CMD_CLASS::kUnhideFile:

		str = cmd->ParseUnhideFile().file_path;
		break;

	case IOCTL_CMD_CLASS::kUnhideDir:

		str = cmd->ParseUnhideDir().dir_path;
		break;

	case IOCTL_CMD_CLASS::kHideProcId:
		pid = cmd->ParseHideProcId().pid;
		break;

	case IOCTL_CMD_CLASS::kUnhideProcId:
		pid = cmd->ParseHideProcId().pid;
		break;

	case IOCTL_CMD_CLASS::kProtectProcId:
		pid = cmd->ParseProtectProcId().pid;

		break;
	case IOCTL_CMD_CLASS::kHideProcImage:

		break;

	case IOCTL_CMD_CLASS::kUnhideProcImage:

		break;

	case IOCTL_CMD_CLASS::kHideReg:

		break;

	case IOCTL_CMD_CLASS::kUnhideReg:

		break;

	case IOCTL_CMD_CLASS::kProctectFile:

		str = cmd->ParseProtectFile().file_path;
		break;

	case IOCTL_CMD_CLASS::kUnproctectFile:

		str = cmd->ParseUnprotectFile().file_path;
		break;

	case IOCTL_CMD_CLASS::kProctectDir:

		str = cmd->ParseProtectDir().dir_path;
		break;

	case IOCTL_CMD_CLASS::kUnproctectDir:

		str = cmd->ParseUnprotectDir().dir_path;
		break;

	case IOCTL_CMD_CLASS::kTestEnableRansom:
		DebugMessage("Get kTestEnableRansom from service");
		ransom::is_enabled = true;
		ransom::test_mode = true;
		proc_mon::p_manager->ResetReport();
		test_start_reply = true;

		pid = cmd->ParseEnableTestRansom().pid;
		proc_mon::p_manager->DeleteProcess(pid);

		proc_mon::proctected_pids->Clear();
		proc_mon::proctected_pids->PushBack(pid);

		irp->IoStatus.Information = sizeof(bool);
		irp->IoStatus.Status = STATUS_SUCCESS;

		RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, &test_start_reply, sizeof(bool));
		break;
	case IOCTL_CMD_CLASS::kTestDisableRansom:
		DebugMessage("Get kTestDisableRansom from service");
		ransom::is_enabled = false;
		ransom::test_mode = false;
		proc_mon::p_manager->KillAll();
		irp->IoStatus.Information = sizeof(proc_mon::Report);
		irp->IoStatus.Status = STATUS_SUCCESS;
		report = proc_mon::p_manager->GetReport();
		RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, &report, sizeof(proc_mon::Report));
		proc_mon::p_manager->ResetReport();
		break;
	default:
		break;
	}

	irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ioctl::MajorFunction(PDEVICE_OBJECT device_object, PIRP irp)
{
	UNREFERENCED_PARAMETER(device_object);

	PIO_STACK_LOCATION stackLocation = NULL;
	stackLocation = IoGetCurrentIrpStackLocation(irp);

	switch (stackLocation->MajorFunction)
	{
	case IRP_MJ_CREATE:
		DebugMessage("Handle to symbolink link %wZ opened", DEVICE_SYMBOLIC_NAME);
		break;
	case IRP_MJ_CLOSE:
		DebugMessage("Handle to symbolink link %wZ closed", DEVICE_SYMBOLIC_NAME);
		break;
	default:
		break;
	}

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}
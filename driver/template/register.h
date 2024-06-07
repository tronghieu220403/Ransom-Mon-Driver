#pragma once

#include <Ntifs.h>
#include <fltKernel.h>

#include "../std/vector/vector.h"

#include "../function/protect/file-protect.h"
#include "../function/hide/hide-file.h"
#include "../function/hide/hide-proc.h"
#include "../function/ransom/ransom-flt.h"
#include "../process/ps-monitor.h"
#include "../com/ioctl/ioctl.h"
#include "debug.h"

namespace reg
{
	struct IrpMjFunc
	{
		size_t irp_mj_function_code = 0;
		PFLT_PRE_OPERATION_CALLBACK pre_func = nullptr;
		PFLT_POST_OPERATION_CALLBACK post_func = nullptr;
	};

	struct Context
	{
		Vector<FLT_PREOP_CALLBACK_STATUS>* status = nullptr;
	};

	extern Vector<IrpMjFunc>* kFltFuncVector;

	extern Vector<void*>* kDrvFuncVector;

	void DrvRegister(
		_In_ PDRIVER_OBJECT driver_object,
		_In_ PUNICODE_STRING registry_path
	);

	void FltRegister();

	void DrvUnload(PDRIVER_OBJECT driver_object);

	void FltUnload();

	Context* AllocCompletionContext();

	void DeallocCompletionContext(Context*);
}


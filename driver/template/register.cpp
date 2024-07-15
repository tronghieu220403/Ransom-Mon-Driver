#include "register.h"

namespace reg
{
	Vector<IrpMjFunc>* kFltFuncVector = nullptr;

	Vector<void*>* kDrvFuncVector = nullptr;

	void DrvRegister(
		_In_ PDRIVER_OBJECT driver_object,
		_In_ PUNICODE_STRING registry_path
	)
	{
		DebugMessage("DriverRegister");

		kDrvFuncVector = new Vector<void*>();

		ioctl::DrvRegister(driver_object, registry_path);
		ransom::DrvRegister(driver_object);
		return;
	}


	void DrvUnload(PDRIVER_OBJECT driver_object)
	{
		DebugMessage("DriverUnloadRegistered");

		ioctl::DrvUnload(driver_object);
		ransom::DrvUnload();
		delete kDrvFuncVector;
		kDrvFuncVector = nullptr;

		return;
	}


	void FltRegister()
	{
		DebugMessage("MiniFilterRegister");

		kFltFuncVector = new Vector<IrpMjFunc>();
		
		ransom::FltRegister();

		com::kComPort = new com::ComPort();

		return;
	}

	void PostFltRegister()
	{
		com::kComPort->SetPfltFilter(kFilterHandle);
		com::kComPort->Create();
	}

	void FltUnload()
	{
		com::kComPort->Close();
		FltUnregisterFilter(kFilterHandle);

		delete kFltFuncVector;
		kFltFuncVector = nullptr;

		return;
	}

	Context* AllocCompletionContext()
	{
		// DebugMessage("AllocCompletionContext");

		Context* context = new Context();
		context->status = new Vector<FLT_PREOP_CALLBACK_STATUS>(kFltFuncVector->Size());

		return context;

	}

	void DeallocCompletionContext(Context *context)
	{
		// DebugMessage("DeallocCompletionContext");

		delete context->status;

		delete context;
	}

}
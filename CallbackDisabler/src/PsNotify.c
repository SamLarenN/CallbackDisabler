#include <ntddk.h>
#include "PsNotify.h"
#include "Global.h"
#include "Utils.h"



UINT64 FindPspLoadImageNotifyRoutine()
{
	UNICODE_STRING usFunc = { 0 };
	RtlInitUnicodeString(&usFunc, L"PsSetLoadImageNotifyRoutine");
	UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);
	
	if (!Function)
		return 0;
	
	char Pattern[3] = { 0x48,0x8D,0x0D };
	Function = FindPattern(Function, 0xFF, Pattern, sizeof(Pattern));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		Function = Function + 7 + Offset;
	}
	
	return Function;
}

NTSTATUS DisablePsImageCallback()
{
	UINT64 PspLoadImageFunc = FindPspLoadImageNotifyRoutine(), pRoutineAddress;
	if (PspLoadImageFunc)
	{
		ListCallbacks(PspLoadImageFunc, DriverInfo.Base, DriverInfo.Size, &pRoutineAddress, TRUE);
		return pRoutineAddress ? PsRemoveLoadImageNotifyRoutine(pRoutineAddress) : STATUS_UNSUCCESSFUL;
	}
	return STATUS_UNSUCCESSFUL;
}

UINT64 FindPspCreateProcessNotifyRoutine()
{
	UNICODE_STRING usFunc = { 0 };
	RtlInitUnicodeString(&usFunc, L"PsSetCreateProcessNotifyRoutine");
	UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

	if (!Function)
		return 0;

	char Pattern1[2] = { 0xC2,0xE9 };
	Function = FindPattern(Function, 20, Pattern1, sizeof(Pattern1));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 2), 4);
		Function = Function + 5 + Offset + 9;
	}

	char Pattern2[3] = { 0x4C,0x8D,0x25 };
	Function = FindPattern(Function, 0xFF, Pattern2, sizeof(Pattern2));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		UINT64 Addy = Function + 7 + Offset;
		return Addy;
	}
	
	return 0;
}

NTSTATUS DisablePsProcessCallback()
{
	UINT64 PspCreateProcessFunc = FindPspLoadImageNotifyRoutine(), pRoutineAddress;
	if (PspCreateProcessFunc)
	{
		ListCallbacks(PspCreateProcessFunc, DriverInfo.Base, DriverInfo.Size, &pRoutineAddress, TRUE);
		return pRoutineAddress ? PsSetCreateProcessNotifyRoutine(pRoutineAddress, TRUE) : STATUS_UNSUCCESSFUL;
	}
	return STATUS_UNSUCCESSFUL;
}

UINT64 FindPspCreateThreadNotifyRoutine()
{
	UNICODE_STRING usFunc = { 0 };
	RtlInitUnicodeString(&usFunc, L"PsSetCreateThreadNotifyRoutine");
	UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

	if (!Function)
		return 0;

	char Pattern1[3] = { 0x33,0xD2,0xE9 };
	Function = FindPattern(Function, 20, Pattern1, sizeof(Pattern1));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		Function = Function + 5 + Offset;
	}

	char Pattern2[3] = { 0x48,0x8D,0x0D };
	Function = FindPattern(Function, 0xFF, Pattern2, sizeof(Pattern2));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		UINT64 Addy = Function + 7 + Offset;
		return Addy;
	}

	return 0;
}

NTSTATUS DisablePsThreadCallback()
{
	UINT64 PspCreateThreadFunc = FindPspCreateThreadNotifyRoutine(), pRoutineAddress;
	if (PspCreateThreadFunc)
	{
		ListCallbacks(PspCreateThreadFunc, DriverInfo.Base, DriverInfo.Size, &pRoutineAddress, TRUE);
		return pRoutineAddress ? PsRemoveCreateThreadNotifyRoutine(pRoutineAddress) : STATUS_UNSUCCESSFUL;
	}
	return STATUS_UNSUCCESSFUL;
}

VOID ListCallbacks(
	_In_ UINT64* Psp,
	_In_ UINT64 Base, _In_ UINT64 Size,
	_Out_ UINT64* pCallbackAddress,
	_In_ BOOLEAN bSaveCallback
)
{
	if (Psp)
	{
		__try
		{
			for (int i = 0; i < 0x10; ++i)
			{
				if (!MmIsAddressValid((PVOID)Psp[i]))
					continue;

				PPS_CALLBACK_ENTRY monCallBack = (PPS_CALLBACK_ENTRY)Mask3Bits(Psp[i]);

				if (MmIsAddressValid(monCallBack->Callback))
				{
					if ((UINT64)monCallBack->Callback > Base && (UINT64)monCallBack->Callback < (Base + Size))
					{
						kprintf("%s: Found Callback Address at: %p\n", __FUNCTION__, monCallBack->Callback);
						if (bSaveCallback)
							*pCallbackAddress = (UINT64)monCallBack->Callback;
						else
							monCallBack->Callback = (PVOID*)*pCallbackAddress;
					}
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}
}


VOID DummyNotifyRoutine()
{
	return;
}
#include <ntddk.h>
#include "Utils.h"
#include "Global.h"


BOOLEAN GetDriverInformation(
	_In_ wchar_t* DriverName,
	_In_ PDRIVER_OBJECT pDriver,
	_Out_ UINT64* ModuleBase,
	_Out_ UINT64* ModuleSize
)
{
	UINT64 pLdrDataEntry = (UINT64)pDriver->DriverSection;
	UINT64 pHead = pLdrDataEntry;
	do
	{
		if (MmIsAddressValid(pLdrDataEntry + 0x58))
		{
			PUNICODE_STRING pusName = (PUNICODE_STRING)(pLdrDataEntry + 0x58);
			kprintf("%wZ\n", pusName);

			if (MmIsAddressValid(pusName->Buffer))
			{
				if (!memcmp(pusName->Buffer, DriverName, sizeof(DriverName)))
				{
					if (MmIsAddressValid(pLdrDataEntry + 0x30))
						*ModuleBase = *(ULONGLONG*)(pLdrDataEntry + 0x30);

					if (MmIsAddressValid(pLdrDataEntry + 0x40))
						*ModuleSize = *(ULONG*)(pLdrDataEntry + 0x40);

					return TRUE;

				}
			}
		}

		pLdrDataEntry = *(UINT64*)pLdrDataEntry;
	} while (pHead != pLdrDataEntry);
	return FALSE;
}

UINT64 FindPattern(
	_In_ UINT64 Base,
	_In_ UINT64 Size,
	_In_ char* Pattern,
	_In_ size_t PatternSize
)
{
	__try
	{
		for (UINT64 i = Base; i < (Base + Size); ++i)
		{
			for (int j = 0; j < PatternSize; ++j)
			{
				if ((UCHAR)Pattern[j] == *(UCHAR*)(i + j))
				{					
					if (j == (PatternSize - 1))
						return i;
				}
				else
					break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		kprintf("%s: Exception!\n", __FUNCTION__);
		return 0;
	}
	return 0;
}


BOOLEAN InitDynamicData()
{
	RTL_OSVERSIONINFOEXW VersionInfo = { 0 };
	BOOLEAN bStatus = TRUE;

	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	NTSTATUS status = RtlGetVersion(&VersionInfo);

	if (bStatus = NT_SUCCESS(status))
	{		
		if (VersionInfo.dwBuildNumber >= 9200)
			g_DynData.CallbackListOffset = 0xC8;

		else if (VersionInfo.dwBuildNumber >= 7600)
			g_DynData.CallbackListOffset = 0xC0;

		else
			bStatus = FALSE;
	}
	return bStatus;
}
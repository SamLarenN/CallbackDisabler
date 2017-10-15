#include <ntddk.h>
#include "Global.h"
#include "Native.h"
#include "ObCallbacks.h"

OB_CALLBACK_ADDRESSES ObAddys = { 0 };

VOID SaveOrigObCallbacks()
{
	EnumerateObCallbacks(*PsProcessType, DriverInfo.Base, DriverInfo.Size, &ObAddys.pProcPreCallback, &ObAddys.pProcPostCallback, TRUE);
	EnumerateObCallbacks(*PsThreadType, DriverInfo.Base, DriverInfo.Size, &ObAddys.pThreadPreCallback, &ObAddys.pThreadPostCallback, TRUE);

	if (ObAddys.pProcPreCallback)
		ObAddys.OrigProcPre = *ObAddys.pProcPreCallback;

	if (ObAddys.pThreadPreCallback)
		ObAddys.OrigThreadPre = *ObAddys.pThreadPreCallback;

	if (ObAddys.pProcPostCallback)
		ObAddys.OrigProcPost = *ObAddys.pProcPostCallback;

	if (ObAddys.pThreadPostCallback)
		ObAddys.OrigThreadPost = *ObAddys.pThreadPostCallback;
}

VOID DisableObCallbacks()
{
	if (ObAddys.pProcPreCallback)
		*ObAddys.pProcPreCallback = DummyObjectPreCallback;

	if (ObAddys.pThreadPreCallback)
		*ObAddys.pThreadPreCallback = DummyObjectPreCallback;

	if (ObAddys.pProcPostCallback)
		*ObAddys.pProcPostCallback = DummyObjectPostCallback;

	if (ObAddys.pThreadPostCallback)
		*ObAddys.pThreadPostCallback = DummyObjectPostCallback;
}

VOID RestoreObCallbacks()
{
	if (ObAddys.pProcPreCallback)
		*ObAddys.pProcPreCallback = ObAddys.OrigProcPre;

	if (ObAddys.pThreadPreCallback)
		*ObAddys.pThreadPreCallback = ObAddys.OrigThreadPre;

	if (ObAddys.pProcPostCallback)
		*ObAddys.pProcPostCallback = ObAddys.OrigProcPost;

	if (ObAddys.pThreadPostCallback)
		*ObAddys.pThreadPostCallback = ObAddys.OrigThreadPost;
}


/*
	Traversing the CallbackLinkedList for the correct callback entry.
*/
VOID EnumerateObCallbacks(_In_ POBJECT_TYPE ObjectType, _In_ UINT64 Base, _In_ UINT64 Size
	, _Out_ UINT64* pPreCallbackAddress, _Out_ UINT64* pPostCallbackAddress)
{
	POBJECT_TYPE pObject = ObjectType;
	__try
	{
		POBJECT_CALLBACK_ENTRY pCallbackEntry = *(POBJECT_CALLBACK_ENTRY*)((UCHAR*)pObject + g_DynData.CallbackListOffset);
		POBJECT_CALLBACK_ENTRY Head = pCallbackEntry;
		while (TRUE)
		{
			if (MmIsAddressValid(pCallbackEntry->CallbackEntry->Altitude.Buffer))
			{
				UINT64 PreOp = (UINT64)pCallbackEntry->PreOperation;
				UINT64 PostOp = (UINT64)pCallbackEntry->PostOperation;

				if (PreOp > Base && PreOp < (Base + Size))
				{
					kprintf("%s: Found PreOperation Callback at: %p\n", &pCallbackEntry->PreOperation);
					*pPreCallbackAddress = &pCallbackEntry->PreOperation;
				}

				if (PostOp > Base && PostOp < (Base + Size))
				{
					kprintf("%s: Found PostOperation Callback at: %p\n", &pCallbackEntry->PostOperation);
					*pPostCallbackAddress = &pCallbackEntry->PostOperation;
				}
			}
			
			pCallbackEntry = (POBJECT_CALLBACK_ENTRY)pCallbackEntry->CallbackList.Flink;
			if (Head == pCallbackEntry)
				break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return;
	}
}


OB_PREOP_CALLBACK_STATUS DummyObjectPreCallback()
{
	return OB_PREOP_SUCCESS;
}

VOID DummyObjectPostCallback()
{
	return;
}
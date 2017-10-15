#pragma once

#ifndef PS_NOTIFY_H
#define PS_NOTIFY_H

#define Mask3Bits(addr)	 (((ULONG_PTR) (addr)) & ~7)

typedef struct _PS_CALLBACK_ENTRY
{
	PVOID* Callback;
	LARGE_INTEGER* Fillz;
} PS_CALLBACK_ENTRY, *PPS_CALLBACK_ENTRY;

UINT64 FindPspLoadImageNotifyRoutine();
UINT64 FindPspCreateProcessNotifyRoutine();
UINT64 FindPspCreateThreadNotifyRoutine();

NTSTATUS DisablePsImageCallback();
NTSTATUS DisablePsProcessCallback();
NTSTATUS DisablePsThreadCallback();

/*
	Searching for the callback address for disabling it.
	Uses the Psp as base.
*/
VOID ListCallbacks(
	_In_ UINT64* Psp,
	_In_ UINT64 Base, _In_ UINT64 Size,
	_Out_ UINT64* pCallbackAddress,
	_In_ BOOLEAN bSaveCallback
);

VOID DummyNotifyRoutine();

#endif PS_NOTIFY_H
#pragma once

#ifndef OB_CALLBACKS_H
#define OB_CALLBACKS_H

/*
	Enumerate ObCallbacks of *PsThreadType or *PsProcess Type.
	Checks if Pre and Post Callbacks are within the limits specified of Base and Size.
	Saves the Pre and Post Callback pointers.
*/
VOID EnumerateObCallbacks(_In_ POBJECT_TYPE ObjectType, _In_ UINT64 Base, _In_ UINT64 Size
						, _Out_ UINT64* pPreCallbackAddress, _Out_ UINT64* pPostCallbackAddress);

/*
	Disable ObCallbacks of Thread and Process If there they exist.
*/
VOID DisableObCallbacks();

/*
	Restore ObCallbacks with pointers saved from SaveOrigObCallbacks ( Call SaveOrigObCallbacks before calling this ).
*/
VOID RestoreObCallbacks();

/*
	Save Original ObCallbacks in a global container.
*/
VOID SaveOrigObCallbacks();

/*
	Dummy Pre Callback Function to disable all functionality of the callback.
*/
OB_PREOP_CALLBACK_STATUS DummyObjectPreCallback();

/*
	Dummy Post Callback Function to disable all functionality of the callback.
*/
VOID DummyObjectPostCallback();

typedef struct _OB_CALLBACK_ADDRESSES
{
	UINT64* pProcPreCallback, *pProcPostCallback;
	UINT64* pThreadPreCallback, *pThreadPostCallback;
	UINT64 OrigProcPre, OrigProcPost;
	UINT64 OrigThreadPre, OrigThreadPost;
}OB_CALLBACK_ADDRESSES, *POB_CALLBACK_ADDRESSES;

// Global ObAddress Container.
extern OB_CALLBACK_ADDRESSES ObAddys;


#endif // !OB_CALLBACKS_H


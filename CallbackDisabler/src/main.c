#include <ntddk.h>
#include "Native.h"
#include "ObCallbacks.h"
#include "PsNotify.h"
#include "Utils.h"
#include "Global.h"


Module DriverInfo = { 0 };
PDRIVER_OBJECT g_pDriverObject = 0;
DYNDATA g_DynData = { 0 };

/* 
	I/O Driver Control Function:
	CTL_GET_DRIVERINFO: 
		Retrieves Base Address and Size of the Driver Specified in SystemBuffer.

	CTL_DISABLE_OB_CALLBACKS:
		Disables ObCallbacks of the thread and process of the driver specified ( call CTL_GET_DRIVERINFO before this ).

	CTL_RESTORE_OB_CALLBACKS:
		Restores ObCallbacks of the thread and process of the driver specified ( call CTL_GET_DRIVERINFO before this ).

	CTL_DISABLE_IMAGE_CALLBACK:
		Disables ImageLoadNotifyRoutine of the specified driver ( call CTL_GET_DRIVERINFO before this ).

	CTL_DISABLE_PROCESS_CALLBACK:
		Disables ProcessNotifyRoutine of the specified driver ( call CTL_GET_DRIVERINFO before this ).

	CTL_DISABLE_THREAD_CALLBACK:
		Disables ThreadNotifyRoutine of the specified driver ( call CTL_GET_DRIVERINFO before this ).
*/
NTSTATUS IOControl(DEVICE_OBJECT* pDeviceObject, IRP* Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	kprintf("Called %s!\n", __FUNCTION__);

	IO_STACK_LOCATION* pStackLoc = IoGetCurrentIrpStackLocation(Irp);

	if (pStackLoc)
	{
		switch (pStackLoc->Parameters.DeviceIoControl.IoControlCode)
		{
		case CTL_GET_DRIVERINFO:
			
			kprintf("%s: Getting Driver Info\n", __FUNCTION__);
			DRIVERNAME name = *(DRIVERNAME*)Irp->AssociatedIrp.SystemBuffer;
			GetDriverInformation(name.Name, g_pDriverObject, &DriverInfo.Base, &DriverInfo.Size);
			kprintf("Base: %p \t Size: 0x%X\n", DriverInfo.Base, DriverInfo.Size);
			break;

		case CTL_DISABLE_OB_CALLBACKS:

			kprintf("%s: Disabling ObCallbacks\n", __FUNCTION__);
			DisableObCallbacks();
			break;

		case CTL_RESTORE_OB_CALLBACKS:

			kprintf("%s: Restore ObCallbacks\n", __FUNCTION__);
			RestoreObCallbacks();
			break;

		case CTL_DISABLE_IMAGE_CALLBACK:

			kprintf("%s: Disabling Image Callback\n", __FUNCTION__);
			status = DisablePsImageCallback();
			break;

		case CTL_DISABLE_PROCESS_CALLBACK:

			kprintf("%s: Disabling Process Callback\n", __FUNCTION__);
			status = DisablePsProcessCallback();
			break;

		case CTL_DISABLE_THREAD_CALLBACK:

			kprintf("%s: Disabling Thread Callback\n", __FUNCTION__);
			status = DisablePsThreadCallback();
			break;

		default:
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	return status;
}

NTSTATUS MJCreateAndClose(DEVICE_OBJECT* pDeviceObject, IRP* Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

/*
	Restore ObCallbacks if I/O forgot to do it to prevent BSOD.
*/
void DriverUnload(PDRIVER_OBJECT pDriver)
{
	kprintf("%s: Done!\n", __FUNCTION__);
	
	UNICODE_STRING usDosDeviceName;
	RtlInitUnicodeString(&usDosDeviceName, DosDeviceName);
	IoDeleteSymbolicLink(&usDosDeviceName);

	RestoreObCallbacks();

	IoDeleteDevice(pDriver->DeviceObject);
}

/*
	Check if OS is supported.
	Create Symbolic Link for I/O.
	Save Original ObCallbacks for restoring them later.
*/
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	if (!InitDynamicData())
		return STATUS_UNSUCCESSFUL;

	PDEVICE_OBJECT pDevice = NULL;
	UNICODE_STRING usDeviceName, usDosDeviceName;
	g_pDriverObject = pDriver;

	RtlInitUnicodeString(&usDeviceName, DeviceName);
	RtlInitUnicodeString(&usDosDeviceName, DosDeviceName);

	NTSTATUS status = IoCreateDevice(pDriver, 0, &usDeviceName
									, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN
									, FALSE, &pDevice);
	if (NT_SUCCESS(status))
	{
		status = IoCreateSymbolicLink(&usDosDeviceName, &usDeviceName);

		if (NT_SUCCESS(status))
		{
			pDriver->DriverUnload = DriverUnload;
			pDriver->MajorFunction[IRP_MJ_CREATE] = MJCreateAndClose;
			pDriver->MajorFunction[IRP_MJ_CLOSE] = MJCreateAndClose;
			pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOControl;

			SaveOrigObCallbacks();
						
			kprintf("%s: Driver Loaded!\n", __FUNCTION__);
		}
	}
	
	return status;
}
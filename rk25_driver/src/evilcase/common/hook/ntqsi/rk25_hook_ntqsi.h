#ifndef _RK25_HOOK_NTQSI_H_
#define _RK25_HOOK_NTQSI_H_

#include <ntddk.h>
#include "../../../../common/rk25_dbg.h"
#include "../../../common/system/kernel/rk25_kernel.h"
#include "../../../common/system/syscall/rk25_syscall.h"

typedef NTSTATUS(*ProtoOrigNtQuerySystemInformation) (
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
	);

typedef VOID(*ProtoHookNtQuerySystemInformation) (
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
	);

NTSTATUS RK25HookNtQuerySystemInformation(
	IN	ULONG attr,
	IN	ProtoHookNtQuerySystemInformation newAddress
);

#endif
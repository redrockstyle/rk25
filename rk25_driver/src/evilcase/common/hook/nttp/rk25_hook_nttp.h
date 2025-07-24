#ifndef _RK25_HOOK_NTTP_H_
#define _RK25_HOOK_NTTP_H_

#include <ntddk.h>
#include "../../../../common/rk25_dbg.h"
#include "../../../common/system/kernel/rk25_kernel.h"
#include "../../../common/system/syscall/rk25_syscall.h"

typedef NTSTATUS(*ProtoOrigNtTerminateProcess)(
	IN OPTIONAL HANDLE ProcessHandle,
	IN			NTSTATUS ExitStatus
	);

typedef VOID(*ProtoHookNtTerminateProcess)(
	IN OPTIONAL HANDLE ProcessHandle,
	IN			NTSTATUS ExitStatus
	);

NTSTATUS RK25HookNtTerminateProcess(
	IN	ULONG attr,
	IN	ProtoHookNtTerminateProcess newAddress
);

#endif // !_RK25_HOOK_NTTP_H_

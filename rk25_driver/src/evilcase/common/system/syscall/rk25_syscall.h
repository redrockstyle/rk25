#ifndef _RK25_SYSCALL_H_
#define _RK25_SYSCALL_H_


/*
* +---------------------+-----------------------------------+--------------------+
* | Версия Windows		|				Метод				|	Риск обнаружения |
* |---------------------+-----------------------------------+--------------------|
* |	XP/2003 (x86)		|		Прямая модификация SSDT		|		Низкий		 |
* |	Vista/7 (x64)		|			MDL + CR0.WP			|		Средний		 |
* |	8/10/11 (x64/ARM)	|	HalPrivateDispatchTable + MDL	|		Низкий		 |
* |	10/11 с HVCI		|	Kernel CET Disable + IAT Hook	|		Высокий		 |
* +---------------------+-----------------------------------+--------------------+
*/

#include <ntddk.h>

#include <WinDef.h>
#include "../../../../common/rk25_dbg.h"
#include "../../../../common/rk25_def.h"
#include "../../../../common/list/rk25_list.h"
#include "../kernel/rk25_kernel.h"
#include "../switcher/rk25_switcher.h"

#define RK25_MAX_RESTORE_BYTES 32
typedef struct _RK25_SYSCALL_RESTORE_DATA {
    PVOID address;
    UCHAR bytes[RK25_MAX_RESTORE_BYTES];
    ULONG len;
} RK25_SYSCALL_RESTORE_DATA, RK25_SRD, *PRK25_SRD;

typedef struct _RK25_SYSCALL_DATA {
    USHORT number;
    ULONG attr;
    PVOID real;
    RK25_SRD restore;
} RK25_SYSCALL_DATA, RK25_SD, * PRK25_SD;

VOID RK25QueueSyscallInit();
VOID RK25QueueSyscallFree();
BOOLEAN RK25QueueSyscallAdd(IN USHORT number, IN PVOID real, IN ULONG attr, IN PRK25_SRD restoreData);
PRK25_SD RK25QueueSyscallGet(USHORT number);
VOID RK25QueueSyscallDelData(PRK25_SD data);
NTSTATUS RK25QueueSyscallDel(USHORT number);


NTSTATUS RK25WriteTrampoline(IN PUCHAR dst, IN PVOID address, OUT PUCHAR buffer, OUT PULONG len);
VOID RK25RestoreAfterHook(PRK25_SRD restoreData);

BOOLEAN RK25IsHookSyscall(USHORT index);
NTSTATUS RK25HookSyscall(IN USHORT index, IN PVOID newAddress, OUT PVOID* oldAddress, OUT PRK25_SRD restoreData);
NTSTATUS RK25HookSyscallSplicing(IN USHORT index, IN PVOID newAddress, OUT PVOID* oldAddress, OUT PRK25_SRD restoreData);
NTSTATUS RK25HookSyscallSplicingRestore(IN USHORT index);


VOID RK25PrintEntrySyscall(IN PKSERVICE_TABLE_DESCRIPTOR entry);



#endif // !_RK25_SYSCALL_H_

#ifndef _RK25_PRIV_PROCESS_H_
#define _RK25_PRIV_PROCESS_H_

#include <ntddk.h>
#include "../../../common/list/rk25_list.h"
#include "../../common/system/kernel/rk25_kernel.h"
#include "../../common/system/switcher/rk25_switcher.h"

#define RK25_PRIV_PROCESS_MODIFY	0x01
#define RK25_PRIV_PROCESS_TO_SYS	0x01
#define RK25_PRIV_PROCESS_STEAL_SYS 0x02

typedef struct _RK25_PRIV_PROC_DATA {
	HANDLE pid;
	UCHAR method;
	PULONG_PTR pointer;
	_STP stp;
} RK25_PRIV_PROC_DATA, RK25_PPD, *PRK25_PPD;

VOID RK25PrivProcessInit();
VOID RK25PrivProcessFree();

NTSTATUS RK25PrivProcessUpModify(HANDLE pid);
NTSTATUS RK25PrivProcessUpToSystem(HANDLE pid);
NTSTATUS RK25PrivProcessUpStealing(HANDLE pid);

PRK25_PPD RK25PrivProcessGet(HANDLE pid);

NTSTATUS RK25PrivProcessRestore(HANDLE pid);
NTSTATUS RK25PrivProcessRestoreData(PRK25_PPD data);

#endif // !_RK25_PRIV_PROCESS_H_

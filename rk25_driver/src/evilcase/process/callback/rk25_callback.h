#ifndef _RK25_CALLBACK_H_
#define _RK25_CALLBACK_H_

#include <ntddk.h>

#include "../../../common/rk25_dbg.h"
#include "../../../common/list/rk25_list.h"
#include "../../common/hook/nttp/rk25_hook_nttp.h"

typedef VOID(*ProtoCallback) (IN HANDLE pid);

typedef struct _RK25_TAMP_CALLBACK_DATA {
	HANDLE pid;
	ProtoCallback callback;
} RK25_TAMP_CALLBACK_DATA, RK25_TCD, *PRK25_TCD;

VOID RK25CallbackInit();
NTSTATUS RK25CallbackHook(ULONG attr);
BOOLEAN RK25CallbackInstall(HANDLE pid, ProtoCallback address);
VOID RK25CallbackExecute(HANDLE pid);
PRK25_TCD RK25CallbackGet(HANDLE pid);
VOID RK25CallbackDel(HANDLE pid);
VOID RK25CallbackFree();

#endif // !_RK25_CALLBACK_H_

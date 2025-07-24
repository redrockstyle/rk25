#ifndef _RK25_NET_CONVERT_H_
#define _RK25_NET_CONVERT_H_

#include <ntddk.h>
#include <ntstrsafe.h>
#include <ip2string.h>
#include <inaddr.h>
#include "../../common/rk25_dbg.h"

#define RK25_NET_IPV4_BUFLEN	16 // longest ip address + '\0'
#define RK25_NET_HTONS(port)	(((port >> 8) & 0x00FF) | ((port << 8) & 0xFF00))

#if DBG
#define RK25_NET_DBG_IPV4(prolog, ip, msgform) \
		CHAR ipm[RK25_NET_IPV4_BUFLEN]; \
		if (NT_SUCCESS(RK25ConvertIPv4Dec2Str(ip, &ipm, RK25_NET_IPV4_BUFLEN))) \
			RK25_DBG_INFO_F(prolog, msgform, &ipm);

#define RK25_NET_DBG_IPV4_CONN(prolog, ip, port, msgform) \
		CHAR ipm[RK25_NET_IPV4_BUFLEN]; \
		USHORT portm = RK25_NET_HTONS(port); \
		if (NT_SUCCESS(RK25ConvertIPv4Dec2Str(ip, &ipm, RK25_NET_IPV4_BUFLEN))) \
			RK25_DBG_INFO_F(prolog, msgform, &ipm, portm);

#define RK25_NET_DBG_IPV4_CONN_2(prolog, ip1, port1, ip2, port2, msgform) \
		CHAR lip[RK25_NET_IPV4_BUFLEN]; \
		CHAR rip[RK25_NET_IPV4_BUFLEN]; \
		USHORT lport = RK25_NET_HTONS(port1); \
		USHORT rport = RK25_NET_HTONS(port2); \
		if (NT_SUCCESS(RK25ConvertIPv4Dec2Str(ip1, &lip, RK25_NET_IPV4_BUFLEN))) \
			if (NT_SUCCESS(RK25ConvertIPv4Dec2Str(ip2, &rip, RK25_NET_IPV4_BUFLEN))) \
				RK25_DBG_INFO_F(prolog, msgform, &lip, lport, &rip, rport);
#else
#define RK25_NET_DBG_IPV4(prolog, ip, msgform)
#define RK25_NET_DBG_IPV4(prolog, ip, port, msgform)
#define RK25_NET_DBG_IPV4_2(prolog, ip1, port1, ip2, port2, msgform)
#endif // !DBG

NTSTATUS RK25ConvertIPv4Dec2Str(IN ULONG ip, OUT PSTR buffer, IN ULONG bufferSize);

NTSTATUS RK25ConvertIPv4Str2Dec(IN PSTR ip, OUT PULONG buffer);

VOID RK25PrintTCPConnection(ULONG localIP, USHORT localPort, ULONG remoteIp, USHORT remotePort);

#endif // !_RK25_NET_CONVERT_H_

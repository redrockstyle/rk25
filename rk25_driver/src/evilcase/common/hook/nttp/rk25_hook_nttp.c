#pragma once
#include "rk25_hook_nttp.h"

#define RK25_PROLOG_HOOK_NTTP "Dispatcher NtTermProc"

//RK25_SRD glSplicingRestoreDataT = { 0 };

ProtoOrigNtTerminateProcess glOrigNtTerminateProcess;
ProtoHookNtTerminateProcess glHookNtTerminateProcess;

NTSTATUS NTAPI HookNtTerminateProcess(
	IN OPTIONAL HANDLE ProcessHandle,
	IN			NTSTATUS ExitStatus
) {
	if (glHookNtTerminateProcess) {
		__try {
			glHookNtTerminateProcess(ProcessHandle, ExitStatus);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
			RK25_DBG_EXCEPT(RK25_PROLOG_HOOK_NTTP, exrec);
		}
	}

	/*
	* It do not returned if Proc
	*/
	return glOrigNtTerminateProcess(ProcessHandle, ExitStatus);
}

//NTSTATUS RK25JampToHookNtTerminateProcess(
//	IN OPTIONAL HANDLE ProcessHandle,
//	IN			NTSTATUS ExitStatus
//) {
//	NTSTATUS status;
//	RK25RestoreAfterHook(&glSplicingRestoreDataT);
//	status = HookNtTerminateProcess(ProcessHandle, ExitStatus);
//	RK25WriteTrampoline(glSplicingRestoreDataT.address, RK25JampToHookNtTerminateProcess, NULL, NULL);
//	return status;
//}

NTSTATUS RK25GetNumberOfNtTerminateProcess() {
	return RK25GetSysallSSNByCode(RK25GetOSVersionCode(NULL), RK25_SYSCALL_NAME_NTTP);
}

NTSTATUS RK25HookNtTerminateProcess(
	IN	ULONG attr,
	IN	ProtoHookNtTerminateProcess newAddress
) {
	NTSTATUS status = STATUS_SUCCESS;
	USHORT number = RK25GetNumberOfNtTerminateProcess();
	RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Hook NtTerminateProcess...");

	if (number) {
		RK25_DBG_INFO_F(RK25_PROLOG_HOOK_NTTP, "Number of syscall is 0x%04X", number);
		if (attr & RK25_HOOK_SSDT_NTTP_REPLACE) {
			if (attr & RK25_ACTION_ENABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Enable replace hook");
				if (!RK25IsHookSyscall(number)) {
					RK25_SRD restore = { 0 };
					status = RK25HookSyscall(
						number, HookNtTerminateProcess,
						(PVOID)&glOrigNtTerminateProcess,
						&restore
					);
					if (status) return status;
					if (!RK25QueueSyscallAdd(
						number, glOrigNtTerminateProcess,
						attr, &restore)) {
						RK25HookSyscall(
							number, glOrigNtTerminateProcess,
							NULL, NULL);
						return status;
					}
					glHookNtTerminateProcess = newAddress;
				}
				else {
					RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTTP, "Syscall 0x%04X is already hooked", number);
					status = STATUS_UNSUCCESSFUL;
				}
			}
			else if (attr & RK25_ACTION_DISABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Disable replace hook");
				if (RK25IsHookSyscall(number)) {
					status = RK25HookSyscall(
						number, glOrigNtTerminateProcess,
						NULL, NULL
					);
					if (NT_SUCCESS(status)) {
						status = RK25QueueSyscallDel(number);
						glHookNtTerminateProcess = NULL;
						glOrigNtTerminateProcess = NULL;
					}
				}
				else {
					RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTTP, "Syscall 0x%04X is not hooked", number);
					status = STATUS_NOT_FOUND;
				}
			}
			else {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Action is not defined");
				status = STATUS_NOT_SUPPORTED;
			}
		}
		//else if (attr & RK25_HOOK_SSDT_NTTP_SPLICING) {
		//	if (attr & RK25_ACTION_ENABLE) {
		//		RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Enable splicing hook");
		//		if (!RK25IsHookSyscall(number)) {
		//			status = RK25HookSyscallSplicing(
		//				number, RK25JampToHookNtTerminateProcess,
		//				(PVOID)&glOrigNtTerminateProcess,
		//				&glSplicingRestoreDataT
		//			);
		//			if (status) return status;
		//			if (!RK25QueueSyscallAdd(
		//				number, glOrigNtTerminateProcess,
		//				attr, &glSplicingRestoreDataT
		//			)) {
		//				RK25RestoreAfterHook(&glSplicingRestoreDataT);
		//				RtlZeroMemory(&glSplicingRestoreDataT, sizeof(RK25_SRD));
		//				status = STATUS_UNSUCCESSFUL;
		//			}
		//			glHookNtTerminateProcess = newAddress;
		//		}
		//		else {
		//			RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTTP, "Syscall 0x%04X is not hooked", number);
		//			status = STATUS_UNSUCCESSFUL;
		//		}
		//	}
		//	else if (attr & RK25_ACTION_DISABLE) {
		//		RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Disable splicing hook");
		//		if (RK25IsHookSyscall(number)) {
		//			RK25RestoreAfterHook(&glSplicingRestoreDataT);
		//			status = RK25QueueSyscallDel(number);
		//			RtlZeroMemory(&glSplicingRestoreDataT, sizeof(RK25_SRD));
		//		}
		//	}
		//	else {
		//		RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Action is not defined");
		//		status = STATUS_NOT_SUPPORTED;
		//	}
		//}
		else {
			RK25_DBG_ERR(RK25_PROLOG_HOOK_NTTP, "Method is not defined");
			status = STATUS_NOT_SUPPORTED;
		}
	}
	else {
		RK25_DBG_INFO(RK25_PROLOG_HOOK_NTTP, "Number for NtTerminateProcess is not found");
		status = STATUS_NOT_FOUND;
	}
	return status;
}
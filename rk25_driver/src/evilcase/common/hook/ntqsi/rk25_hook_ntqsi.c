#pragma once
#include "rk25_hook_ntqsi.h"

#define RK25_PROLOG_HOOK_NTQSI "Dispatch NTQSI"

RK25_SRD glSplicingRestoreDataQ = { 0 };

ProtoOrigNtQuerySystemInformation glOrigNtQuerySystemInformation = NULL;
ProtoHookNtQuerySystemInformation glHookNtQueryInformation = NULL;

NTSTATUS NTAPI HookNtQuerySystemInformation(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
) {
	NTSTATUS status;

	status = glOrigNtQuerySystemInformation(
		SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength
	);

	if (NT_SUCCESS(status)) {
		if (glHookNtQueryInformation) {
			__try {
				glHookNtQueryInformation(
					SystemInformationClass,
					SystemInformation,
					SystemInformationLength,
					ReturnLength
				);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
				RK25_DBG_EXCEPT(RK25_PROLOG_HOOK_NTQSI, exrec);
			}
		}
	}

	return status;
}

NTSTATUS RK25JampToHookNtQuerySystemInformation(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
) {
	__try {
		NTSTATUS status;
		RK25RestoreAfterHook(&glSplicingRestoreDataQ);
		status = HookNtQuerySystemInformation(
			SystemInformationClass,
			SystemInformation,
			SystemInformationLength,
			ReturnLength
		);
		RK25WriteTrampoline(glSplicingRestoreDataQ.address, RK25JampToHookNtQuerySystemInformation, NULL, NULL);
		return status;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
		RK25_DBG_EXCEPT(RK25_PROLOG_HOOK_NTQSI, exrec);
		return STATUS_UNSUCCESSFUL;
	}
}

USHORT RK25GetNumberOfNtQuerySystemInformation() {
	return RK25GetSysallSSNByCode(RK25GetOSVersionCode(NULL), RK25_SYSCALL_NAME_NTQSI);
}

NTSTATUS RK25HookNtQuerySystemInformation(
	IN	ULONG attr,
	IN	ProtoHookNtQuerySystemInformation newAddress
) {
	NTSTATUS status = STATUS_SUCCESS;
	USHORT number = RK25GetNumberOfNtQuerySystemInformation();
	RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Hook NtQuerySystemInformation...");

	if (number) {
		RK25_DBG_INFO_F(RK25_PROLOG_HOOK_NTQSI, "Number of syscall is 0x%04X", number);
		if (attr & RK25_HOOK_SSDT_NTQSI_REPLACE) {
			if (attr & RK25_ACTION_ENABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Enable replace hook");
				if (!RK25IsHookSyscall(number)) {
					RK25_SRD restore = { 0 };
					status = RK25HookSyscall(
						number, HookNtQuerySystemInformation,
						(PVOID)&glOrigNtQuerySystemInformation,
						&restore
					);
					if (status) return status;
					if (!RK25QueueSyscallAdd(
						number, glOrigNtQuerySystemInformation,
						attr, &restore)) {
						RK25HookSyscall(
							number, glOrigNtQuerySystemInformation,
							NULL, NULL);
						return status;
					}
					glHookNtQueryInformation = newAddress;
				}
				else {
					RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTQSI, "Syscall 0x%04X is already hooked", number);
					status = STATUS_UNSUCCESSFUL;
				}
			}
			else if (attr & RK25_ACTION_DISABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Disable replace hook");
				if (RK25IsHookSyscall(number)) {
					status = RK25HookSyscall(
						number, glOrigNtQuerySystemInformation,
						NULL, NULL
					);
					if (NT_SUCCESS(status)) {
						status = RK25QueueSyscallDel(number);
						glHookNtQueryInformation = NULL;
						glOrigNtQuerySystemInformation = NULL;
					}
				}
				else {
					RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTQSI, "Syscall 0x%04X is not hooked", number);
					status = STATUS_NOT_FOUND;
				}
			}
			else {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Action is not defined");
				status = STATUS_NOT_SUPPORTED;
			}
		}
		else if (attr & RK25_HOOK_SSDT_NTQSI_SPLICING) {
			if (attr & RK25_ACTION_ENABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Enable splicing hook");
				if (!RK25IsHookSyscall(number)) {
					status = RK25HookSyscallSplicing(
						number, RK25JampToHookNtQuerySystemInformation,
						(PVOID)&glOrigNtQuerySystemInformation,
						&glSplicingRestoreDataQ
					);
					if (status) return status;
					if (!RK25QueueSyscallAdd(
						number, glOrigNtQuerySystemInformation,
						attr, &glSplicingRestoreDataQ
					)) {
						RK25RestoreAfterHook(&glSplicingRestoreDataQ);
						RtlZeroMemory(&glSplicingRestoreDataQ, sizeof(RK25_SRD));
						status = STATUS_UNSUCCESSFUL;
					}
					glHookNtQueryInformation = newAddress;
				}
				else {
					RK25_DBG_WARN_F(RK25_PROLOG_HOOK_NTQSI, "Syscall 0x%04X is not hooked", number);
					status = STATUS_UNSUCCESSFUL;
				}
			}
			else if (attr & RK25_ACTION_DISABLE) {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Disable splicing hook");
				if (RK25IsHookSyscall(number)) {
					RK25RestoreAfterHook(&glSplicingRestoreDataQ);
					status = RK25QueueSyscallDel(number);
					RtlZeroMemory(&glSplicingRestoreDataQ, sizeof(RK25_SRD));
				}
			}
			else {
				RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "Action is not defined");
				status = STATUS_NOT_SUPPORTED;
			}
		}
		else {
			RK25_DBG_ERR(RK25_PROLOG_HOOK_NTQSI, "Method is not defined");
			status = STATUS_NOT_SUPPORTED;
		}
	}
	else {
		RK25_DBG_INFO(RK25_PROLOG_HOOK_NTQSI, "NtQuerySystemInformation number is not found");
		status = STATUS_NOT_FOUND;
	}
	return status;
}
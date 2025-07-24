#pragma once
#include "rk25_syscall.h"

#define RK25_PROLOG_HOOK_SYSCALL "Syscall Dispatcher"

PRK25_LC glLCSyscall;

VOID RK25QueueSyscallInit() {
    glLCSyscall = RK25ListInit(sizeof(RK25_SD));
}

VOID RK25RestoreAfterHook(PRK25_SRD restoreData) {
    if (!restoreData) return;
    RK25SuperCopyMemory(restoreData->address, &restoreData->bytes, restoreData->len);
}

VOID RK25QueueSyscallFree() {
    PRK25_SD entry;
    do {
        entry = (PRK25_SD)RK25ListEntryNext(glLCSyscall);
        if (entry) {
            if (entry->number) {
                
                if (entry->attr & RK25_HOOK_SSDT_NTQSI_REPLACE ||
                    entry->attr & RK25_HOOK_SSDT_NTTP_REPLACE) {
                    if (NT_SUCCESS(RK25HookSyscall(entry->number, entry->real, NULL, NULL))) {
                        RK25_DBG_INFO_F(RK25_PROLOG_HOOK_SYSCALL, "Successful unhook syscall number: 0x%04X", entry->number);
                    } else RK25_DBG_INFO_F(RK25_PROLOG_HOOK_SYSCALL, "Unsuccessful unhook syscall number: 0x%04X - mb system fault!!!", entry->number);
                }
                RK25RestoreAfterHook(&entry->restore);
            }
        }
    } while (entry != NULL);
    RK25ListFree(glLCSyscall);
}

PRK25_SD RK25QueueSyscallGet(USHORT number) {
    PRK25_SD entry;
    do {
        entry = (PRK25_SD)RK25ListEntryNext(glLCSyscall);
        if (entry) if (entry->number == number) break;
    } while (entry != NULL);
    RK25ListFlush(glLCSyscall);
    return entry;
}

VOID RK25QueueSyscallDelData(PRK25_SD data) {
    RK25ListEntryDel(glLCSyscall, data);
}

NTSTATUS RK25QueueSyscallDel(USHORT number) {
    NTSTATUS status = STATUS_SUCCESS;
    PRK25_SD data = RK25QueueSyscallGet(number);
    if (data->attr & RK25_HOOK_SSDT_NTQSI_REPLACE)
        status = RK25HookSyscall(data->number, data->real, NULL, NULL);
    RK25RestoreAfterHook(&data->restore);
    RK25ListEntryDel(glLCSyscall, data);
    return status;
}

BOOLEAN RK25IsHookSyscall(USHORT index) {
    return RK25QueueSyscallGet(index) ? TRUE : FALSE;
}

BOOLEAN RK25QueueSyscallAdd(IN USHORT number, IN PVOID real, IN ULONG attr, IN PRK25_SRD restoreData) {
    RK25_SD entry;
    entry.number = number;
    entry.real = real;
    entry.attr = attr;
    if (restoreData) entry.restore = *restoreData;
    return RK25ListAdd(glLCSyscall, &entry);
}

NTSTATUS RK25WriteTrampoline(IN PUCHAR dst, IN PVOID address, OUT PUCHAR buffer, OUT PULONG len) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    if (!dst || !address) return status;

#if defined(_M_X64)
    CONST ULONG size = sizeof(USHORT) + sizeof(ULONG) + sizeof(ULONGLONG); // 2 bytes of opcode jmp + 4 bytes of offset + 8 bytes of address
#else
    CONST ULONG size = sizeof(UCHAR) + sizeof(ULONG) + sizeof(ULONG); // 2 bytes of opcode jmp + 4 bytes of offset + 8 bytes of address
#endif
    PUCHAR bytecode = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, size, 'EDOC');
    if (bytecode) {

#if defined(_M_X64)
        // Write bytecode (?? is address):
        // ff 25 00 00 00 00 ?? ?? ?? ?? ?? ?? ?? ??
        // 
        // Disasm (?? is address):
        // ff 25 00 00 00 00        jmp    QWORD PTR [rip+0x0]      # jmp to [rip + 6 byte opcode]
        // ?? ?? ?? ?? ?? ?? ?? ??                                  # <- rip
        //
        *(PUSHORT)(bytecode) = 0x25FF; // opcode jmp x64
        *(PULONG)(bytecode + sizeof(USHORT)) = 0;
        *(PULONGLONG)(bytecode + sizeof(USHORT) + sizeof(ULONG)) = (ULONGLONG)address;
#else
        *(PUCHAR)(bytecode) = 0xE9; // opcode jmp x86
        *(PULONG)(bytecode + sizeof(PUCHAR)) = 0;
        *(PULONG)(bytecode + sizeof(PUCHAR) + sizeof(ULONG)) = (ULONG)address;
#endif

        if (buffer && len) {
            *len = size;
            RtlCopyMemory(buffer, dst, size);
        }
        status = RK25SuperCopyMemory(dst, bytecode, size);

        ExFreePool(bytecode);
    }
    return status;
}


/*
* @brief Finds an executable memory space in the kernel module
* 
* @param[in] Address for which the offset will be searched
* 
* @return Address of the executable space or NULL if not found
*/
PVOID RK25FindTrampolineSpace(IN PVOID address) {
    if (address == NULL)
        return NULL;

    PIMAGE_NT_HEADERS pImageNtHeaders;
    PIMAGE_SECTION_HEADER first, section;
    USHORT count;
    CONST ULONG size = sizeof(USHORT) + sizeof(ULONG) + sizeof(PVOID);
    
    PUCHAR pKernelBase = RK25GetKernelBaseAddress();
    if (!pKernelBase) return NULL;

    pImageNtHeaders = RtlImageNtHeader(pKernelBase);
    first = (PIMAGE_SECTION_HEADER)(pImageNtHeaders + 1);
    count = pImageNtHeaders->FileHeader.NumberOfSections;
    for (PIMAGE_SECTION_HEADER section = first; section < first + count; section++) {
        if ((PUCHAR)address < pKernelBase + section->VirtualAddress ||
            (PUCHAR)address < pKernelBase + section->VirtualAddress + (ULONG_PTR)PAGE_ALIGN(section->Misc.VirtualSize)) {
            if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE &&
                !(section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) &&
                (*(PULONG)section->Name != 'TINI')) {
                ULONG_PTR offset = 0;
                if ((PUCHAR)address >= pKernelBase + section->VirtualAddress)
                    offset = (PUCHAR)address - pKernelBase - section->VirtualAddress;

                for (ULONG_PTR i = offset, bytes = 0; i < (ULONG_PTR)PAGE_ALIGN(section->Misc.VirtualSize - size); i++) {
                    // int3, nop, or inside unused section space
                    if (pKernelBase[section->VirtualAddress + i] == 0xCC || pKernelBase[section->VirtualAddress + i] == 0x90 || i > section->Misc.VirtualSize - size)
                        bytes++;
                    else bytes = 0;

                    if (bytes >= size) return &pKernelBase[section->VirtualAddress + i - bytes + 1];
                }
            }
        }
    }
    return NULL;
}

NTSTATUS RK25HookSyscall(IN USHORT index, IN PVOID newAddress, OUT PVOID* oldAddress, OUT PRK25_SRD restoreData) {
    if (newAddress == NULL) return STATUS_INVALID_PARAMETER;
    NTSTATUS status = STATUS_SUCCESS;

    PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT = RK25GetSSDTAddress();
    if (!pSSDT) return STATUS_NOT_FOUND;
    
#if defined(_M_IX86)
    if (oldAddress) *oldAddress = pSSDT->ServiceTableBase[index];
    RK25SuperCopyMemory(pSSDT->ServiceTableBase + index, &newAddress, sizeof(PVOID));
#else
    if (oldAddress) *oldAddress = (PUCHAR)pSSDT->ServiceTableBase + (((PLONG)pSSDT->ServiceTableBase)[index] >> 4);
    ULONG_PTR offset = (ULONG_PTR)newAddress - (ULONG_PTR)pSSDT->ServiceTableBase;
    // Use trampline if offset is overflow
    if (offset > 0x07FFFFFF) {
        // Allocate trampoline, if required
        PVOID pTrampoline = RK25FindTrampolineSpace(pSSDT->ServiceTableBase);
        if (!pTrampoline) return STATUS_NOT_FOUND;

        // Write jmp address
        if (restoreData) {
            restoreData->address = pTrampoline;
            status = RK25WriteTrampoline(pTrampoline, newAddress, (PUCHAR)&restoreData->bytes, &restoreData->len);
        }
        else status = RK25WriteTrampoline(pTrampoline, newAddress, NULL, NULL);
        if (status) return status;

        offset = ((((ULONG_PTR)pTrampoline - (ULONG_PTR)pSSDT->ServiceTableBase) << 4) & 0xFFFFFFF0) | (pSSDT->ParamTableBase[index] >> 2);
    }
    else offset = ((offset << 4) & 0xFFFFFFF0) | (pSSDT->ParamTableBase[index] >> 2); // default offset

    status = RK25SuperCopyMemory((PLONG)pSSDT->ServiceTableBase + index, &offset, sizeof(LONG));
#endif
    return status;
}

NTSTATUS RK25HookSyscallSplicing(IN USHORT index, IN PVOID newAddress, OUT PVOID* oldAddress, OUT PRK25_SRD restoreData) {
    if (!newAddress && !restoreData) return STATUS_INVALID_PARAMETER;
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT = RK25GetSSDTAddress();
    if (!pSSDT) return STATUS_NOT_FOUND;

#if defined(_M_IX86)
    restoreData->address = pSSDT->ServiceTableBase[index];
#else
    restoreData->address = (PUCHAR)pSSDT->ServiceTableBase + (((PLONG)pSSDT->ServiceTableBase)[index] >> 4);
#endif
    if (oldAddress) *oldAddress = restoreData->address;
    return RK25WriteTrampoline(restoreData->address, newAddress, (PUCHAR)&restoreData->bytes, &restoreData->len);
}

NTSTATUS RK25HookSyscallSplicingRestore(IN USHORT index) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PRK25_SD data = RK25QueueSyscallGet(index);

    if (data) {
        RK25RestoreAfterHook(&data->restore);
        status = STATUS_SUCCESS;
    }
    return status;
}

VOID RK25PrintEntrySyscall(IN PKSERVICE_TABLE_DESCRIPTOR entry) {
    RK25_DBG_INFO_F(RK25_PROLOG_HOOK_SYSCALL,"Entry SSDT: Base:0x%p\tLimit:%d\tNumber:0x%p", entry->Base, entry->Limit, entry->Number);
    return;
}
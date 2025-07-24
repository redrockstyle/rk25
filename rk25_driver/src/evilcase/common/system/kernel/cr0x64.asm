IFNDEF RAX
.MODEL  FLAT
ENDIF

.CODE

IFDEF RAX
; ULONGLONG AsmClearWP()
AsmClearWP PROC
    xor rax, rax
    mov rax, cr0
    mov rbx, rax
    and rax, 0FFFEFFFFh
    mov cr0, rax
    mov rax, rbx
    ret
AsmClearWP ENDP


; VOID AsmWriteCR0(ULONGLONG reg)
AsmWriteCR0 PROC
    mov rax, QWORD PTR [rcx]
    mov cr0, rax
    ret
AsmWriteCR0 ENDP

; AsmInt
AsmInt PROC
    int 3h
    ret
AsmInt ENDP

ENDIF

END
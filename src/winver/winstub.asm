; WINVER real-mode stub: run from plain DOS, WINVER.EXE prints the Windows
; version instead of "This program requires Microsoft Windows".
;
; Assembled with UASM (MASM-compatible), linked as a DOS .EXE with LINK 5.60,
; and bound in front of WINVER.EXE via the STUB statement in winver.def.

_TEXT   segment para public 'CODE'
        assume  cs:_TEXT, ds:_TEXT

start:
        jmp     short print
        nop
        db      4, 0                    ; Windows version 4.00
        db      1, 0
szVersion       db      'Windows 95', 13, 10, '$'

print:
        mov     dx, offset szVersion
        push    cs
        pop     ds
        mov     ah, 09h                 ; print string
        int     21h
        mov     ax, 4C01h               ; exit(1)
        int     21h
_TEXT   ends

STACK   segment para stack 'STACK'
        db      256 dup (?)
STACK   ends

        end     start

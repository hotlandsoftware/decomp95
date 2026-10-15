; TAPIEXE startup code.
;
; Reconstruction of the medium-model "no C runtime" Windows startup module
; (windows\crt0.asm, as found in MSVC 1.52 MNOCRTW.LIB).  The variant TAPIEXE
; was linked with also calls LockSegment(-1) after InitTask, so it is supplied
; here instead of taking crt0 from the library.  Segments, publics and data are
; laid out exactly as in the library module.
;
; Assembled with UASM (MASM-compatible):  uasm -omf -Cp start.asm

        extrn   __WINFLAGS:abs
        extrn   INITTASK:far, WINMAIN:far, WAITEVENT:far, INITAPP:far
        extrn   DOS3CALL:far, LOCKSEGMENT:far

_TEXT   segment word public 'CODE'
_TEXT   ends
TAPISTR segment para public 'CODE'
TAPISTR ends
NULL    segment para public 'BEGDATA'
NULL    ends
_DATA   segment word public 'DATA'
_DATA   ends
CDATA   segment word common 'DATA'
CDATA   ends
CONST   segment word public 'CONST'
CONST   ends
_BSS    segment word public 'BSS'
_BSS    ends
DGROUP  group   NULL, _DATA, CDATA, CONST, _BSS

        public  __acrtused, __fptaskdata, __sizec, __sized
        public  __aDBdoswp, __aDBused
__acrtused      =       9876h
__fptaskdata    =       9876h
__sizec         =       1
__sized         =       0
__aDBdoswp      =       0D6D6h
__aDBused       =       0D6D6h

; ---------------------------------------------------------------------------
NULL    segment
        public  rsrvptrs, pLocalHeap, pAtomTable, pStackTop, pStackMin, pStackBot
        dw      0, 0
rsrvptrs        dw      5               ; number of reserved pointers
pLocalHeap      dw      0               ; filled in by the kernel
pAtomTable      dw      0
pStackTop       dw      0
pStackMin       dw      0
pStackBot       dw      0
NULL    ends

_DATA   segment
        public  _STKHQQ, __hPrevInstance, __hInstance, __lpszCmdLine
        public  __cmdShow, _aseglo, _aseghi, ___aDBswpflg, ___aDBrterr
_STKHQQ         dw      0               ; stack limit (CX from InitTask + 100h)
__hPrevInstance dw      0
__hInstance     dw      0
__lpszCmdLine   dd      0
__cmdShow       dw      0
_aseglo         dw      1
_aseghi         dw      0FFFFh
___aDBswpflg    dw      0
___aDBrterr     dw      0
_DATA   ends

; ---------------------------------------------------------------------------
_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP
        public  __wflags, _exit, __exit, __astart

__wflags        dw      __WINFLAGS

; void _exit(int);  reached by "push ax / push cs / call near".
_exit:
__exit:
        pop     ax
        pop     ax
        pop     ax                      ; AL = exit code
        mov     ah, 4Ch
        test    __wflags, 1             ; WF_PMODE
        jz      exit_dos
        call    DOS3CALL
        jmp     short __fail
exit_dos:
        int     21h

__fail:
        mov     al, 0FFh
        push    ax
        push    cs
        call    near ptr __exit

__astart:
        xor     bp, bp
        push    bp
        call    INITTASK
        or      ax, ax
        jz      __fail
        add     cx, 100h
        jc      __fail
        mov     _STKHQQ, cx
        mov     __hPrevInstance, si
        mov     __hInstance, di
        mov     word ptr __lpszCmdLine, bx
        mov     word ptr __lpszCmdLine+2, es
        mov     __cmdShow, dx
        mov     ax, 0FFFFh
        push    ax
        call    LOCKSEGMENT             ; lock DGROUP (not in MSVC 1.52's crt0)
        xor     ax, ax
        push    ax
        call    WAITEVENT
        push    __hInstance
        call    INITAPP
        or      ax, ax
        jz      __fail
        push    __hInstance
        push    __hPrevInstance
        push    word ptr __lpszCmdLine+2
        push    word ptr __lpszCmdLine
        push    __cmdShow
        call    WINMAIN
        push    ax
        push    cs
        call    near ptr __exit
_TEXT   ends

; ---------------------------------------------------------------------------
; Section name in WIN.INI whose changes are forwarded to TAPI.DLL.
TAPISTR segment
        public  _szTelephony
_szTelephony    db      'Windows Telephony', 0
TAPISTR ends

        end     __astart

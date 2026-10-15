; TAPIEXE startup code.
;
; Reconstruction of the medium-model "no C runtime" Windows startup module
; (windows\crt0.asm, as found in MSVC 1.52 MNOCRTW.LIB).  The variant TAPIEXE
; was linked with also calls LockSegment(-1) after InitTask, so it is supplied
; here instead of taking crt0 from the library.  Segments, publics and data are
; laid out exactly as in the library module.
;
; Assembled with NASM (-f obj).  Instructions whose MASM encoding differs from
; NASM's default (reg,reg forms of xor/or) are emitted as raw bytes.

%macro XOR_BP_BP 0
        db      33h, 0EDh               ; xor bp,bp   (MASM encoding)
%endmacro
%macro XOR_AX_AX 0
        db      33h, 0C0h               ; xor ax,ax
%endmacro
%macro OR_AX_AX 0
        db      0Bh, 0C0h               ; or  ax,ax
%endmacro

        extern  __WINFLAGS, INITTASK, WINMAIN, WAITEVENT, INITAPP, DOS3CALL
        extern  LOCKSEGMENT

segment _TEXT   public  align=2  class=CODE     use16
segment TAPISTR public  align=16 class=CODE     use16
segment NULL    public  align=16 class=BEGDATA  use16
segment _DATA   public  align=2  class=DATA     use16
segment CDATA   common  align=2  class=DATA     use16
segment CONST   public  align=2  class=CONST    use16
segment _BSS    public  align=2  class=BSS      use16
group   DGROUP  NULL _DATA CDATA CONST _BSS

        global  __acrtused, __fptaskdata, __sizec, __sized
        global  __aDBdoswp, __aDBused
__acrtused      equ     9876h
__fptaskdata    equ     9876h
__sizec         equ     1
__sized         equ     0
__aDBdoswp      equ     0D6D6h
__aDBused       equ     0D6D6h

; ---------------------------------------------------------------------------
segment NULL
        global  rsrvptrs
        dw      0, 0
rsrvptrs:
        dw      5                       ; number of reserved pointers
        global  pLocalHeap, pAtomTable, pStackTop, pStackMin, pStackBot
pLocalHeap      dw      0               ; filled in by the kernel
pAtomTable      dw      0
pStackTop       dw      0
pStackMin       dw      0
pStackBot       dw      0

segment _DATA
        global  _STKHQQ, __hPrevInstance, __hInstance, __lpszCmdLine
        global  __cmdShow, _aseglo, _aseghi, ___aDBswpflg, ___aDBrterr
_STKHQQ         dw      0               ; stack limit (CX from InitTask + 100h)
__hPrevInstance dw      0
__hInstance     dw      0
__lpszCmdLine   dd      0
__cmdShow       dw      0
_aseglo         dw      1
_aseghi         dw      0FFFFh
___aDBswpflg    dw      0
___aDBrterr     dw      0

; ---------------------------------------------------------------------------
segment _TEXT
        global  __wflags, _exit, __exit, __astart

__wflags:
        dw      __WINFLAGS

; void _exit(int);  reached by "push ax / push cs / call near".
_exit:
__exit:
        pop     ax
        pop     ax
        pop     ax                      ; AL = exit code
        mov     ah, 4Ch
        test    word [cs:__wflags], 1   ; WF_PMODE
        jz      .dos
        call    far DOS3CALL
        jmp     short __fail
.dos:
        int     21h

__fail:
        mov     al, 0FFh
        push    ax
        push    cs
        call    __exit

..start:
__astart:
        XOR_BP_BP
        push    bp
        call    far INITTASK
        OR_AX_AX
        jz      __fail
        add     cx, 100h
        jc      __fail
        mov     [_STKHQQ wrt DGROUP], cx
        mov     [__hPrevInstance wrt DGROUP], si
        mov     [__hInstance wrt DGROUP], di
        mov     [__lpszCmdLine wrt DGROUP], bx
        mov     [__lpszCmdLine+2 wrt DGROUP], es
        mov     [__cmdShow wrt DGROUP], dx
        mov     ax, 0FFFFh
        push    ax
        call    far LOCKSEGMENT         ; lock DGROUP (not in MSVC 1.52's crt0)
        XOR_AX_AX
        push    ax
        call    far WAITEVENT
        push    word [__hInstance wrt DGROUP]
        call    far INITAPP
        OR_AX_AX
        jz      __fail
        push    word [__hInstance wrt DGROUP]
        push    word [__hPrevInstance wrt DGROUP]
        push    word [__lpszCmdLine+2 wrt DGROUP]
        push    word [__lpszCmdLine wrt DGROUP]
        push    word [__cmdShow wrt DGROUP]
        call    far WINMAIN
        push    ax
        push    cs
        call    __exit

; ---------------------------------------------------------------------------
; Section name in WIN.INI whose changes are forwarded to TAPI.DLL.
segment TAPISTR
        global  _szTelephony
_szTelephony:
        db      'Windows Telephony', 0

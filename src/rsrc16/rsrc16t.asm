; rsrc16t.asm - 16-bit side of the Resource Meter flat thunk
;
; This is what the Windows 95 thunk compiler generates for a 32->16 thunk
; script with one function (MyGetFreeSystemResources16): the target table,
; the thk_ThunkConnect16 helper that hands THK_THUNKDATA16 to
; KERNEL.ThunkConnect16, and the THK_THUNKDATA16 block itself.

        .286

        extrn   THUNKCONNECT16:far              ; KERNEL.651
        extrn   __WINFLAGS:abs                  ; KERNEL.178
        extrn   MYGETFREESYSTEMRESOURCES16:far

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT

thkTargetTable label dword
        dd      MYGETFREESYSTEMRESOURCES16

; BOOL FAR PASCAL thk_ThunkConnect16(LPSTR pszDll16, LPSTR pszDll32,
;                                    WORD hInst, DWORD dwReason)
        public  THK_THUNKCONNECT16
THK_THUNKCONNECT16 proc far
        pop     ax                              ; return address
        pop     dx
        push    seg THK_THUNKDATA16
        push    offset THK_THUNKDATA16
        push    seg szThunkData32
        push    offset szThunkData32
        push    cs
        push    dx
        push    ax
        jmp     THUNKCONNECT16
THK_THUNKCONNECT16 endp

szThunkData32   db      'thk_ThunkData32', 0
                db      16 dup (0)
                dw      __WINFLAGS
                dw      0, 0
_TEXT   ends

_DATA   segment word public 'DATA'
        public  THK_THUNKDATA16
THK_THUNKDATA16 label byte
        db      'LS01'
        dd      07AFh                           ; thunk script checksum
        dd      thkTargetTable                  ; -> target table
        dd      0
        dd      100h
        dd      0
        dd      0
        dw      1, 0FFFFh
        dd      0
_DATA   ends

DGROUP  group   _DATA


        end

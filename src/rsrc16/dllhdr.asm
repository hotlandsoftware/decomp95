; dllhdr.asm - start of RSRC16.DLL's segments (no C runtime)
;
; Linked first so that _TEXT is the first segment and the instance-data
; header (the NULL segment) starts DGROUP.

_TEXT   segment word public 'CODE'
_TEXT   ends

; Satisfies the C compiler's reference to the startup code (there is none).
        public  __acrtused
__acrtused = 9876h

NULL    segment word public 'BEGDATA'
        dw      0, 0, 5, 0, 0, 0, 0, 0          ; instance data header
NULL    ends

DGROUP  group   NULL

        end

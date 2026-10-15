; "Windows Telephony" (the WIN.INI section whose changes are forwarded to
; TAPI.DLL).  It lives in a paragraph-aligned code-class segment that is linked
; after the startup code and packed into the single code segment.  TAPIEXE.C
; refers to it as an extern __far string.
;
; Assembled with UASM (MASM-compatible):  uasm -omf -Cp telestr.asm

TAPISTR segment para public 'CODE'
        public  _szTelephony
_szTelephony    db      'Windows Telephony', 0
TAPISTR ends

        end

; msmixmgr.asm - Microsoft Audio Mixer Manager (Windows 95, 4.00.950)
;
; MSMIXMGR.DLL only forwards the mixer API: each export jumps to the
; matching MMSYSTEM.DLL ordinal (800-809), where Windows 95 implements it.

        .286

        extrn   MMSYS_MIXERGETNUMDEVS:far       ; MMSYSTEM.800
        extrn   MMSYS_MIXERGETDEVCAPS:far       ; MMSYSTEM.801
        extrn   MMSYS_MIXEROPEN:far             ; MMSYSTEM.802
        extrn   MMSYS_MIXERCLOSE:far            ; MMSYSTEM.803
        extrn   MMSYS_MIXERMESSAGE:far          ; MMSYSTEM.804
        extrn   MMSYS_MIXERGETLINEINFO:far      ; MMSYSTEM.805
        extrn   MMSYS_MIXERGETID:far            ; MMSYSTEM.806
        extrn   MMSYS_MIXERGETLINECONTROLS:far  ; MMSYSTEM.807
        extrn   MMSYS_MIXERGETCONTROLDETAILS:far ; MMSYSTEM.808
        extrn   MMSYS_MIXERSETCONTROLDETAILS:far ; MMSYSTEM.809

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT

; Library entry point: nothing to initialise.
        public  LIBENTRY
LIBENTRY proc   far
        ret
LIBENTRY endp

        public  WEP
WEP     proc    far
        mov     ax, 1
        ret     2
WEP     endp

FORWARD macro name
        public  name
name    proc    far
        jmp     MMSYS_&name
name    endp
        endm

        FORWARD MIXERGETNUMDEVS
        FORWARD MIXERGETDEVCAPS
        FORWARD MIXERGETID
        FORWARD MIXEROPEN
        FORWARD MIXERCLOSE
        FORWARD MIXERMESSAGE
        FORWARD MIXERGETLINEINFO
        FORWARD MIXERGETLINECONTROLS
        FORWARD MIXERGETCONTROLDETAILS
        FORWARD MIXERSETCONTROLDETAILS

_TEXT   ends

; The automatic data segment holds nothing but one uninitialised byte.
_DATA   segment word public 'DATA'
        db      ?
_DATA   ends

DGROUP  group   _DATA

        end     LIBENTRY

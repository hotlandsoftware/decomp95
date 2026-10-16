; npbss.asm - layout of Notepad's uninitialised globals.
;
; In the original these were C "common" variables; LINK 2.50 allocates
; commons in an order that depends on the compiler's symbol hash of their
; (unknown) original names.  To reproduce the layout exactly they are
; defined here instead, in a .bss contribution linked last (after the two
; file-static .bss variables in npmain.c and strings.c).  Offsets are from
; 0x405000.
;
; Assembled with UASM:  uasm -coff -Cp npbss.asm

        .386
        .model  flat

_BSS    segment dword public 'BSS'

        public  _cxPrintChar, _cyPrintPage, _pHeaderBuf, _hAbortDlgWnd
        public  _iTabStops, _iPageNum, _cyPrintLine, _hAbortSysMenu
        public  _fPrintAborted, _dyHeader, _xCharsPerLine, _rtPrintMargin
        public  _chPageTextSave, _chPeriod, _chPageText, _hInstanceNP
        public  _szFilterSpec, _szSearch, _hSysMenuSetup, _FR, _hAccel
        public  _wFRMsg, _hWaitCursor, _szCustFilterSpec, _fp, _hEdit
        public  _OFN, _szFileName, _g_PageSetupDlg, _hFont, _chMerge

; npprint.c
_cxPrintChar        dd  ?                   ; 084
_cyPrintPage        dd  ?                   ; 088
_pHeaderBuf         dd  ?                   ; 08c
_hAbortDlgWnd       dd  ?                   ; 090
_iTabStops          dd  ?                   ; 094
_iPageNum           dd  ?                   ; 098
_cyPrintLine        dd  ?                   ; 09c
_hAbortSysMenu      dd  ?                   ; 0a0
_fPrintAborted      dd  ?                   ; 0a4
_dyHeader           dd  ?                   ; 0a8
_xCharsPerLine      dd  ?                   ; 0ac
_rtPrintMargin      dd  4 dup (?)           ; 0b0  RECT
_chPageTextSave     db  80 dup (?)          ; 0c0  [2][40]
; npinit.c
_chPeriod           db  ?                   ; 110
                    db  15 dup (?)
; npmain.c
_chPageText         db  80 dup (?)          ; 120  [2][40]
_hInstanceNP        dd  ?                   ; 170
                    dd  3 dup (?)
_szFilterSpec       db  64 dup (?)          ; 180
_szSearch           db  32 dup (?)          ; 1c0
_hSysMenuSetup      dd  ?                   ; 1e0
                    dd  3 dup (?)
_FR                 db  28h dup (?)         ; 1f0  FINDREPLACE
_hAccel             dd  ?                   ; 218
_wFRMsg             dd  ?                   ; 21c
_hWaitCursor        dd  ?                   ; 220
                    dd  3 dup (?)
_szCustFilterSpec   db  64 dup (?)          ; 230
_fp                 dd  ?                   ; 270
_hEdit              dd  ?                   ; 274
                    dd  2 dup (?)
_OFN                db  4ch dup (?)         ; 280  OPENFILENAME
                    dd  ?
_szFileName         db  260 dup (?)         ; 2d0
                    db  12 dup (?)
_g_PageSetupDlg     db  54h dup (?)         ; 3e0  PAGESETUPDLG
_hFont              dd  ?                   ; 434
_chMerge            dw  ?                   ; 438

_BSS    ends
        end

; wlcmstr.asm - read-only strings of the Welcome screen
;
; These live in the code section, 4-byte aligned (the C compiler would
; align arrays of 8 bytes or more to 8), at the very start of .text.

        .386
        .model  flat

_TEXT   segment dword public use32 'CODE'
        align   4
        public  _c_szTipsKey
_c_szTipsKey db 'Software\Microsoft\Windows\CurrentVersion\Explorer\Tips', 0
        align   4
        public  _c_szTourKey
_c_szTourKey db 'Software\Microsoft\Windows\CurrentVersion\SETUP\optionalComponents\tour', 0
        align   4
        public  _c_szCurrentVersion
_c_szCurrentVersion db 'Software\Microsoft\Windows\CurrentVersion', 0
        align   4
        public  _c_szRegDone
_c_szRegDone db 'REGDONE', 0
        align   4
        public  _c_szSetupKey
_c_szSetupKey db 'Software\Microsoft\Windows\CurrentVersion\SETUP', 0
        align   4
        public  _c_szRegWizKey
_c_szRegWizKey db 'Software\Microsoft\Shared Tools\Registration Wizard\1.0', 0
        align   4
        public  _c_szNext
_c_szNext db 'Next', 0
        align   4
        public  _c_szShow
_c_szShow db 'Show', 0
        align   4
        public  _c_szInstalled
_c_szInstalled db 'installed', 0
        align   4
        public  _c_szExpostrt
_c_szExpostrt db 'Expostrt', 0
        align   4
        public  _c_szLocation
_c_szLocation db 'Location', 0
        align   4
        public  _c_szTourPath
_c_szTourPath db 'TourPath', 0
_TEXT   ends

        end

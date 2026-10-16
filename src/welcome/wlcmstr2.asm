; wlcmstr2.asm - a read-only string kept in the code section
;
; In the original image this string sits between RunFromSetupSource and the
; dialog code at a 4-byte boundary, so it came from a separate module.

        .386
        .model  flat

_TEXT   segment dword public use32 'CODE'
        public  _c_szSourcePathType
_c_szSourcePathType db 'SourcePathType', 0
_TEXT   ends

        end

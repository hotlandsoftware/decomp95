/*
 * strings.cpp - DBCS-aware string helpers (Windows 95 shell string library;
 * built here in its single-byte form).  The names follow the functions'
 * later public SHLWAPI equivalents.  Compiled as C++ (the register
 * allocation only matches the C++ front end), with C linkage.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define NPPAD 11
#include "nppad.h"

extern "C" {

static WORD wMatch;     /* character searched for by the ChrCmp helpers */

/* Returns FALSE if w1 matches wMatch (case sensitive). */
static BOOL __fastcall ChrCmpA(WORD w1)
{
    return (BYTE)w1 - (BYTE)wMatch != 0;
}

/* Returns FALSE if w1 matches wMatch (case insensitive). */
static BOOL __fastcall ChrCmpIA(WORD w1)
{
    char sz1[3], sz2[3];

    sz1[0] = (char)w1;
    sz1[1] = '\0';
    *(WORD *)sz2 = wMatch;
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
}

/* Returns a pointer to the character at most wLen bytes into lpFirst. */
static LPSTR __fastcall CharAtLength(LPCSTR lpFirst, short wLen)
{
    LPCSTR lpLast = lpFirst + wLen;

    if (*lpFirst) {
        while (lpLast > lpFirst) {
            lpFirst = CharNextA(lpFirst);
            if (!*lpFirst)
                break;
        }
    }
    if (lpLast < lpFirst)
        lpFirst -= 2;
    return (LPSTR)lpFirst;
}

/* memrchr: finds the last ch in the cb bytes ending at lpLast. */
LPSTR WINAPI MemRChr(LPSTR lpLast, DWORD cb, BYTE chFind)
{
    _asm {
        mov     ecx, cb
        jecxz   NotFound
        mov     edi, lpLast
        mov     al, chFind
        std
        repnz   scasb
        jnz     NotFound
        inc     edi
        mov     eax, edi
        jmp     Done
    NotFound:
        xor     eax, eax
    Done:
        cld
    }
}

LPSTR WINAPI StrChrA(LPCSTR lpStart, WORD wMatchIn)
{
    wMatch = wMatchIn;
    for (; *lpStart; lpStart = CharNextA(lpStart)) {
        if (!ChrCmpA(*(WORD *)lpStart))
            return (LPSTR)lpStart;
    }
    return NULL;
}

LPSTR WINAPI StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatchIn)
{
    LPCSTR lpFound = NULL;

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);
    wMatch = wMatchIn;
    for (; lpStart < lpEnd; lpStart = CharNextA(lpStart)) {
        if (!ChrCmpA(*(WORD *)lpStart))
            lpFound = lpStart;
    }
    return (LPSTR)lpFound;
}

LPSTR WINAPI StrChrIA(LPCSTR lpStart, WORD wMatchIn)
{
    wMatch = (BYTE)wMatchIn;
    for (; *lpStart; lpStart = CharNextA(lpStart)) {
        if (!ChrCmpIA(*(WORD *)lpStart))
            return (LPSTR)lpStart;
    }
    return NULL;
}

LPSTR WINAPI StrRChrIA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatchIn)
{
    LPCSTR lpFound = NULL;

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);
    wMatch = (BYTE)wMatchIn;
    for (; lpEnd > lpStart; lpStart = CharNextA(lpStart)) {
        if (!ChrCmpIA(*(WORD *)lpStart))
            lpFound = lpStart;
    }
    return (LPSTR)lpFound;
}

/* Compares the first nChar bytes of two strings (case sensitive). */
short WINAPI StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, short nChar)
{
    char  sc1, sc2;
    LPSTR lpsz1, lpsz2;
    short ret;

    lpsz1 = CharAtLength(lpStr1, nChar);
    sc1 = *lpsz1;
    lpsz2 = CharAtLength(lpStr2, nChar);
    sc2 = *lpsz2;
    *lpsz2 = '\0';
    *lpsz1 = '\0';
    ret = lstrcmpA(lpStr1, lpStr2);
    *lpsz1 = sc1;
    *lpsz2 = sc2;
    return ret;
}

short WINAPI StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, short nChar)
{
    char  sc1, sc2;
    LPSTR lpsz1, lpsz2;
    short ret;

    lpsz1 = CharAtLength(lpStr1, nChar);
    sc1 = *lpsz1;
    lpsz2 = CharAtLength(lpStr2, nChar);
    sc2 = *lpsz2;
    *lpsz2 = '\0';
    *lpsz1 = '\0';
    ret = lstrcmpiA(lpStr1, lpStr2);
    *lpsz1 = sc1;
    *lpsz2 = sc2;
    return ret;
}

/* Alternative implementations walking both strings a character at a time. */
short WINAPI StrCmpNA2(LPSTR lpStr1, LPSTR lpStr2, short nChar)
{
    char  sc1, sc2;
    short i;
    short ret;

    for (i = 0; i < nChar; i++) {
        if (!*lpStr1 || !*lpStr2)
            return lstrcmpA(lpStr1, lpStr2);
        lpStr1 = CharNextA(lpStr1);
        lpStr2 = CharNextA(lpStr2);
    }
    sc1 = *lpStr1;
    sc2 = *lpStr2;
    *lpStr2 = '\0';
    *lpStr1 = '\0';
    ret = lstrcmpA(lpStr1, lpStr2);
    *lpStr1 = sc1;
    *lpStr2 = sc2;
    return ret;
}

short WINAPI StrCmpNIA2(LPSTR lpStr1, LPSTR lpStr2, short nChar)
{
    char  sc1, sc2;
    short i;
    short ret;

    for (i = 0; i < nChar; i++) {
        if (!*lpStr1 || !*lpStr2)
            return lstrcmpiA(lpStr1, lpStr2);
        lpStr1 = CharNextA(lpStr1);
        lpStr2 = CharNextA(lpStr2);
    }
    sc1 = *lpStr1;
    sc2 = *lpStr2;
    *lpStr2 = '\0';
    *lpStr1 = '\0';
    ret = lstrcmpiA(lpStr1, lpStr2);
    *lpStr1 = sc1;
    *lpStr2 = sc2;
    return ret;
}

LPSTR WINAPI StrStrA(LPCSTR lpFirst, LPCSTR lpSrch)
{
    short len = (short)lstrlenA(lpSrch);
    WORD  wMatchCh = *(WORD *)lpSrch;

    for (; (lpFirst = StrChrA(lpFirst, wMatchCh)) != NULL &&
           StrCmpNA(lpFirst, lpSrch, len);
         lpFirst = CharNextA(lpFirst))
        ;
    return (LPSTR)lpFirst;
}

LPSTR WINAPI StrRStrA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch)
{
    short iLen = (short)lstrlenA(lpSrch);
    BYTE  ch = *lpSrch;

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    while (lpLast != lpSource) {
        lpLast--;
        lpLast = MemRChr((LPSTR)lpLast, lpLast - lpSource + 1, ch);
        if (!lpLast)
            return NULL;
        if (!StrCmpNA(lpLast, lpSrch, iLen) &&
            CharAtLength(lpSource, (short)(lpLast - lpSource)) == lpLast)
            return (LPSTR)lpLast;
    }
    return NULL;
}

LPSTR WINAPI StrStrIA(LPCSTR lpFirst, LPCSTR lpSrch)
{
    short len = (short)lstrlenA(lpSrch);
    WORD  wMatchCh = *(WORD *)lpSrch;

    for (; (lpFirst = StrChrIA(lpFirst, wMatchCh)) != NULL &&
           StrCmpNIA(lpFirst, lpSrch, len);
         lpFirst = CharNextA(lpFirst))
        ;
    return (LPSTR)lpFirst;
}

LPSTR WINAPI StrRStrIA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch)
{
    LPCSTR lpFound = NULL;
    LPSTR  lpEnd;
    char   cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    if (lpSource >= lpLast || !*lpSrch)
        return NULL;

    lpEnd = CharAtLength(lpLast, (short)(lstrlenA(lpSrch) - 1));
    cHold = *lpEnd;
    *lpEnd = '\0';
    while ((lpSource = StrStrIA(lpSource, lpSrch)) != NULL && lpSource < lpLast) {
        lpFound = lpSource;
        lpSource = CharNextA(lpSource);
    }
    *lpEnd = cHold;
    return (LPSTR)lpFound;
}

}   /* extern "C" */

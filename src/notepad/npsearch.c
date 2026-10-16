/*
 * npsearch.c - Find, and re-creating the edit control for Word Wrap.
 */

#include "notepad.h"
#define NPPAD 29
#include "nppad.h"

BOOL fCase = FALSE;         /* match case */
BOOL fReverse = FALSE;      /* search up */

void Search(TCHAR *szKey)
{
    HCURSOR hPrevCursor;
    HANDLE  hText;
    TCHAR  *pText;
    TCHAR  *pch;
    DWORD   dwStart, dwEnd;
    UINT    cch;
    int     iLine;
    UINT    ichLine, ichPrev;

    if (!*szKey)
        return;

    hPrevCursor = SetCursor(hWaitCursor);
    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
    cch = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
    if (!(hText = LocalAlloc(LHND, cch)))
        return;
    if (!(pText = (TCHAR *)LocalLock(hText))) {
        LocalFree(hText);
        return;
    }
    SendMessage(hwndEdit, WM_GETTEXT, cch, (LPARAM)pText);

    if (fReverse) {
        /* Search backwards a line at a time. */
        iLine = SendMessage(hwndEdit, EM_LINEFROMCHAR, dwStart, 0);
        ichLine = SendMessage(hwndEdit, EM_LINEINDEX, iLine, 0);
        ichPrev = dwStart;
        for (;;) {
            if (fCase)
                pch = StrRStrA(pText + ichLine, pText + ichPrev, szKey);
            else
                pch = StrRStrIA(pText + ichLine, pText + ichPrev, szKey);
            if (pch)
                break;
            ichPrev = ichLine;
            if (!iLine)
                break;
            iLine--;
            ichLine = SendMessage(hwndEdit, EM_LINEINDEX, iLine, 0);
        }
    } else {
        if (fCase)
            pch = StrStrA(pText + dwEnd, szKey);
        else
            pch = StrStrIA(pText + dwEnd, szKey);
    }

    LocalUnlock(hText);
    SetCursor(hPrevCursor);

    if (!pch) {
        AlertBox(hDlgFind ? hDlgFind : hwndNP, szNN, szCFS, szSearch,
                 MB_OK | MB_ICONASTERISK);
    } else {
        dwStart = pch - pText;
        SendMessage(hwndEdit, EM_SETSEL, dwStart, dwStart + lstrlen(szKey));
        SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
    }
}

/* Re-creates the edit control with or without horizontal scrolling. */
BOOL NpReCreate(BOOL fWrapIt)
{
    LONG   dwStyle;
    UINT   cch;
    HANDLE hText;
    TCHAR *pText;
    HWND   hwndNew;
    RECT   rc;

    dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL;
    if (!fWrapIt)
        dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_NOHIDESEL;

    SendMessage(hwndEdit, EM_FMTLINES, FALSE, 0);
    cch = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    if (!(hText = LocalAlloc(LHND, cch + 1))) {
        if (!fWrapIt)
            SendMessage(hwndEdit, EM_FMTLINES, TRUE, 0);
        return FALSE;
    }

    GetClientRect(hwndNP, &rc);
    pText = (TCHAR *)LocalLock(hText);
    SendMessage(hwndEdit, WM_GETTEXT, cch + 1, (LPARAM)pText);

    hwndNew = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT(""), dwStyle,
                             0, 0, rc.right, rc.bottom, hwndNP, (HMENU)15,
                             hInstanceNP, NULL);
    if (!hwndNew) {
        LocalUnlock(hText);
        LocalFree(hText);
        return FALSE;
    }

    if (!SendMessage(hwndNew, WM_SETTEXT, 0, (LPARAM)pText)) {
        DestroyWindow(hwndNew);
        LocalUnlock(hText);
        LocalFree(hText);
        return FALSE;
    }

    LocalUnlock(hText);
    SendMessage(hwndNew, WM_SETFONT, (WPARAM)hFont, FALSE);
    DestroyWindow(hwndEdit);
    hwndEdit = hwndNew;
    hEdit = hText;
    ShowWindow(hwndNP, SW_SHOW);
    SetTitle(fUntitled ? szUntitled : szFileName);
    if (cch)
        SendMessage(hwndEdit, EM_SETMODIFY, TRUE, 0);
    SetFocus(hwndEdit);
    return TRUE;
}

/*
 * npprint.c - printing, page setup hook, header/footer formatting and
 * Edit/Time-Date.
 */

#include "notepad.h"
#include <string.h>
#define NPPAD 21
#include "nppad.h"

#pragma intrinsic(memset)

HANDLE hHeader = NULL;

/* print state (uninitialised; laid out by npbss.asm) */
int    cxPrintChar;
int    cyPrintPage;
TCHAR *pHeaderBuf;
HWND   hAbortDlgWnd;
int    iTabStops;
int    iPageNum;
int    cyPrintLine;
HMENU  hAbortSysMenu;
BOOL   fPrintAborted;
int    dyHeader;
int    xCharsPerLine;
RECT   rtPrintMargin;
TCHAR  chPageTextSave[2][PT_LEN];

static const DWORD aHelpIDs[] = {
    32, 1001,
    30, 1001,
    33, 1000,
    31, 1000,
    34, (DWORD)-1,
    0, 0
};

BOOL CALLBACK AbortProc(HDC hPrintDC, int nCode)
{
    MSG msg;

    while (!fPrintAborted && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!hAbortDlgWnd || !IsDialogMessage(hAbortDlgWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return !fPrintAborted;
}

BOOL CALLBACK AbortDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        hAbortSysMenu = GetSystemMenu(hwnd, FALSE);
        if (!fUntitled)
            SetDlgItemText(hwnd, 20, PFileInPath(szFileName));
        SetFocus(hwnd);
        return TRUE;

    case WM_COMMAND:
        fPrintAborted = TRUE;
        DestroyAbortWnd();
        return TRUE;

    case WM_INITMENU:
        EnableMenuItem(hAbortSysMenu, SC_CLOSE, MF_GRAYED);
        return TRUE;
    }
    return FALSE;
}

/* nHF: 0 = header, 1 = footer */
void PrintHeaderFooter(HDC hDC, short nHF)
{
    TCHAR buf[80];
    short len;

    lstrcpy(buf, chPageText[nHF]);
    pHeaderBuf = (TCHAR *)GlobalLock(hHeader);
    len = TranslateString(buf);
    if (*pHeaderBuf) {
        if (nHF == 0)
            TabbedTextOut(hDC, rtPrintMargin.left, dyHeader - cyPrintLine, pHeaderBuf, len, 1,
                      &iTabStops, rtPrintMargin.left);
        else
            TabbedTextOut(hDC, rtPrintMargin.left, cyPrintPage - cyPrintLine - dyHeader, pHeaderBuf, len, 1,
                      &iTabStops, rtPrintMargin.left);
    }
    GlobalUnlock(hHeader);
}

int NpPrint(void)
{
    HDC     hPrintDC;
    HANDLE  hText = NULL;
    HANDLE  hLine;
    HFONT   hPrintFont, hOldFont = NULL;
    TEXTMETRIC tm;
    TCHAR   szFaceName[32];
    TCHAR   szTitle[MAX_PATH + 50];
    DOCINFO di;
    SIZE    size;
    int     xPrintRes;
    int     nLinesPerPage;
    int     nPrinted;
    int     iErr;
    int     iLine, nLines;
    int     ichLine, cch, iOffset;
    int     y;
    BOOL    fContinue;
    DWORD   dwCharSet;
    TCHAR  *pText, *pLine;
    HDC     hdc;

    hLine = NULL;
    hOldFont = NULL;
    SetCursor(hWaitCursor);
    fPrintAborted = FALSE;

    hdc = GetDC(hwndNP);
    dwCharSet = GetTextCharset(hdc);
    ReleaseDC(hwndNP, hdc);

    hPrintDC = GetPrinterDC();
    if (!hPrintDC)
        return -5;
    if (hPrintDC == (HDC)-1)
        return -1;

    xPrintRes = GetDeviceCaps(hPrintDC, HORZRES);
    cyPrintPage = GetDeviceCaps(hPrintDC, VERTRES);

    /* Map page-setup units (thousandths of inches / hundredths of mm)
     * onto the printable area. */
    SetMapMode(hPrintDC, MM_ANISOTROPIC);
    SetViewportExtEx(hPrintDC, xPrintRes, cyPrintPage, NULL);
    SetWindowExtEx(hPrintDC, g_PageSetupDlg.ptPaperSize.x,
                   g_PageSetupDlg.ptPaperSize.y, NULL);
    rtPrintMargin = g_PageSetupDlg.rtMargin;
    LPtoDP(hPrintDC, (LPPOINT)&rtPrintMargin, 2);
    SetMapMode(hPrintDC, MM_TEXT);

    SetBkMode(hPrintDC, TRANSPARENT);
    GetTextMetrics(hPrintDC, &tm);
    LoadString(hInstanceNP, 57, szFaceName, sizeof(szFaceName));
    hPrintFont = CreateFont(tm.tmHeight, 0, 0, 0, tm.tmWeight, 0, 0, 0, dwCharSet,
                            OUT_DEFAULT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY,
                            FIXED_PITCH | FF_MODERN, szFaceName);
    if (hPrintFont)
        hOldFont = SelectObject(hPrintDC, hPrintFont);
    GetTextMetrics(hPrintDC, &tm);

    cyPrintLine = tm.tmHeight + tm.tmExternalLeading;
    cxPrintChar = tm.tmAveCharWidth;
    iTabStops = tm.tmAveCharWidth * 8;
    dyHeader = GetDeviceCaps(hPrintDC, LOGPIXELSY) / 4;
    nLinesPerPage = (cyPrintPage - rtPrintMargin.bottom - rtPrintMargin.top) / cyPrintLine;
    GetTextExtentPoint(hPrintDC, TEXT(" "), 1, &size);
    xCharsPerLine = xPrintRes / cxPrintChar - rtPrintMargin.right / size.cx
                                            - rtPrintMargin.left / size.cx;

    if (nLinesPerPage <= 0 || xCharsPerLine <= 0) {
        DeleteDC(hPrintDC);
        return -1;
    }

    SetAbortProc(hPrintDC, (ABORTPROC)AbortProc);
    GetWindowText(hwndNP, szTitle, sizeof(szTitle));
    di.lpszDocName = szTitle;
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;
    di.cbSize = sizeof(DOCINFO);
    iErr = StartDoc(hPrintDC, &di);
    if (iErr < 0) {
        if (GetLastError() == ERROR_PRINT_CANCELLED)
            iErr = -3;
        goto ErrorExit;
    }

    EnableWindow(hwndNP, FALSE);
    hAbortDlgWnd = CreateDialogParam(hInstanceNP, MAKEINTRESOURCE(12), hwndNP,
                                     (DLGPROC)AbortDlgProc, 0);
    if (!hAbortDlgWnd)
        goto NoMem;

    cch = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
    hText = LocalAlloc(LHND, cch);
    if (!hText)
        goto NoMem;
    pText = (TCHAR *)LocalLock(hText);
    if (!pText) {
        LocalFree(hText);
        iErr = -5;
        goto ErrorExit;
    }
    SendMessage(hwndEdit, WM_GETTEXT, cch, (LPARAM)pText);
    LocalUnlock(hText);

    iLine = 0;
    y = 0;
    nPrinted = 0;
    iPageNum = 1;
    hHeader = GlobalAlloc(GHND, xCharsPerLine + 2);
    hLine = GlobalAlloc(GHND, 0x1000);
    if (!hHeader || !hLine)
        goto NoMem;

    pLine = (TCHAR *)GlobalLock(hLine);
    pText = (TCHAR *)LocalLock(hText);
    nLines = SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0);

    while (!fPrintAborted && iLine < nLines) {
        ichLine = SendMessage(hwndEdit, EM_LINEINDEX, iLine++, 0);
        cch = SendMessage(hwndEdit, EM_LINELENGTH, ichLine, 0);
        cch = ExpandTabs(pText + ichLine, cch, pLine, xCharsPerLine);
        iOffset = 0;
        if (xCharsPerLine < cch)
            fContinue = TRUE;
    NewPage:
            if (nPrinted == 0) {
                if ((iErr = StartPage(hPrintDC)) < 0)
                    goto ErrorExit;
                SelectObject(hPrintDC, hPrintFont);
                PrintHeaderFooter(hPrintDC, 0);
            }
            do {
            if (xCharsPerLine < cch) {
                TabbedTextOut(hPrintDC, rtPrintMargin.left,
                              rtPrintMargin.top - cyPrintLine + y,
                              pLine + iOffset, xCharsPerLine, 1, &iTabStops,
                              rtPrintMargin.left);
                cch -= xCharsPerLine;
                iOffset += xCharsPerLine;
            } else {
                TabbedTextOut(hPrintDC, rtPrintMargin.left,
                              rtPrintMargin.top - cyPrintLine + y,
                              pLine + iOffset, cch, 1, &iTabStops,
                              rtPrintMargin.left);
                fContinue = FALSE;
            }
            y += cyPrintLine;
            if (++nPrinted >= nLinesPerPage) {
                y = 0;
                PrintHeaderFooter(hPrintDC, 1);
                if ((iErr = EndPage(hPrintDC)) < 0)
                    goto ErrorExit;
                iPageNum++;
                nPrinted = 0;
                if (fContinue)
                    goto NewPage;
            }
            } while (fContinue);
    }

    LocalUnlock(hText);
    LocalFree(hText);
    GlobalUnlock(hLine);
    GlobalFree(hLine);
    hLine = NULL;

    if (nPrinted)
        PrintHeaderFooter(hPrintDC, 1);
    if (!fPrintAborted) {
        if (nPrinted > 0 && (iErr = EndPage(hPrintDC)) < 0)
            goto ErrorExit;
        if (!fPrintAborted) {
            EndDoc(hPrintDC);
            DestroyAbortWnd();
        }
    }
    DeleteDC(hPrintDC);
    DeleteObject(hPrintFont);
    GlobalFree(hHeader);
    hHeader = NULL;
    return 0;

NoMem:
    iErr = -5;
ErrorExit:
    if (hOldFont)
        SelectObject(hPrintDC, hOldFont);
    AbortDoc(hPrintDC);
    DeleteDC(hPrintDC);
    DeleteObject(hPrintFont);
    if (!fPrintAborted)
        DestroyAbortWnd();
    if (hHeader) {
        GlobalFree(hHeader);
        hHeader = NULL;
    }
    if (hLine)
        GlobalFree(hLine);
    return iErr;
}

/* Copies cch characters of pSrc to pDst expanding tabs (tab stops every
 * 8 columns), wrapping every cchLine characters.  Returns the expanded
 * length. */
int ExpandTabs(TCHAR *pSrc, int cch, TCHAR *pDst, int cchLine)
{
    TCHAR buf[200];
    int   total = 0;
    int   col = 0, nextcol;
    int   i;
    for (i = 0; i < cch; i++) {
        if (col >= cchLine) {
            buf[cchLine] = 0;
            lstrcpy(pDst, buf);
            pDst += cchLine;
            col = 0;
        }
        if (*pSrc == TEXT('\t')) {
            nextcol = (col + 8) / 8 * 8;
            if (nextcol > cchLine)
                nextcol = cchLine;
            if (col < nextcol)
                memset(buf + col, ' ', nextcol - col);
            total += nextcol - col;
            col = nextcol;
        } else {
            buf[col++] = *pSrc;
            total++;
        }
        pSrc++;
    }
    buf[col] = 0;
    lstrcpy(pDst, buf);
    return total;
}

void DestroyAbortWnd(void)
{
    EnableWindow(hwndNP, TRUE);
    DestroyWindow(hAbortDlgWnd);
    hAbortDlgWnd = NULL;
}

UINT CALLBACK PageSetupHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int   id;
    POINT pt;

    switch (msg) {
    case WM_INITDIALOG:
        for (id = 30; id <= 31; id++) {
            SendDlgItemMessage(hWnd, id, EM_LIMITTEXT, PT_LEN - 1, 0);
            SetDlgItemText(hWnd, id, chPageText[id - 30]);
        }
        SendDlgItemMessage(hWnd, 30, EM_SETSEL, 0, MAKELONG(0, PT_LEN - 1));
        return TRUE;

    case WM_DESTROY:
        for (id = 30; id <= 31; id++)
            GetDlgItemText(hWnd, id, chPageTextSave[id - 30], PT_LEN);
        break;

    case WM_HELP:
        id = GetDlgCtrlID(((LPHELPINFO)lParam)->hItemHandle);
        if (id >= 30 && id <= 34) {
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)aHelpIDs);
            return TRUE;
        }
        break;

    case WM_CONTEXTMENU:
        if ((HWND)wParam == hWnd) {
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            wParam = (WPARAM)ChildWindowFromPoint(hWnd, pt);
        }
        id = GetDlgCtrlID((HWND)wParam);
        if (id >= 30 && id <= 34) {
            WinHelp((HWND)wParam, szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)aHelpIDs);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* Expands header/footer codes (&f file, &p page, &t time, &d date,
 * &l &c &r alignment, && ampersand) into pHeaderBuf.  Returns its length. */
int TranslateString(TCHAR *src)
{
    TCHAR  letters[15];
    TCHAR  chBuff[3][80];
    TCHAR  buf[80];
    TCHAR  szDate[28], szTime[28];
    SYSTEMTIME st;
    int    nLen[3];
    int   *pLen;
    int    index = 1;
    int    i, j, k, num;

    nLen[0] = nLen[1] = nLen[2] = 0;

    GetLocalTime(&st);
    wsprintf(szDate, TEXT("%2.2d\\%2.2d\\%2.2d"), st.wMonth, st.wDay, st.wYear);
    wsprintf(szTime, TEXT("%2.2d:%2.2d:%4.4d"), st.wHour, st.wMinute, st.wSecond);
    LoadString(hInstanceNP, 42, letters, 15);

    while (*src) {
        while (*src && *src != TEXT('&'))
            chBuff[index][nLen[index]++] = *src++;
        if (*src == TEXT('&')) {
            src++;
            if (*src == letters[0] || *src == letters[1]) {
                if (fUntitled)
                    lstrcpy(buf, szUntitled);
                else
                    GetFileTitle(szFileName, buf, sizeof(buf));
                pLen = &nLen[index];
                lstrcpy(chBuff[index] + *pLen, buf);
                *pLen += lstrlen(buf);
            } else if (*src == letters[2] || *src == letters[3]) {
                num = 0;
                if (src[1] == TEXT('+')) {
                    src += 2;
                    while (*src >= TEXT('0') && *src <= TEXT('9'))
                        num = num * 10 + *src++ - TEXT('0');
                    src--;
                }
                wsprintf(buf, TEXT("%d"), iPageNum + num);
                pLen = &nLen[index];
                k = *pLen;
                lstrcpy(chBuff[index] + k, buf);
                *pLen = lstrlen(buf) + k;
            } else if (*src == letters[4] || *src == letters[5]) {
                pLen = &nLen[index];
                k = *pLen;
                lstrcpy(chBuff[index] + k, szTime);
                *pLen = k + 8;
            } else if (*src == letters[6] || *src == letters[7]) {
                pLen = &nLen[index];
                k = *pLen;
                lstrcpy(chBuff[index] + k, szDate);
                *pLen = k + 12;
            } else if (*src == TEXT('&')) {
                chBuff[index][nLen[index]++] = TEXT('&');
            } else if (*src == letters[8] || *src == letters[9]) {
                index = 1;
            } else if (*src == letters[10] || *src == letters[11]) {
                index = 2;
            } else if (*src == letters[12] || *src == letters[13]) {
                index = 0;
            }
            src++;
        }
    }

    for (i = 0; i < 3; i++)
        chBuff[i][nLen[i]] = 0;

    for (i = 0; i < xCharsPerLine; i++)
        pHeaderBuf[i] = TEXT(' ');
    for (i = 0; i < nLen[0]; i++)
        pHeaderBuf[i] = chBuff[0][i];
    k = xCharsPerLine - nLen[1];
    for (j = 0; j < nLen[1]; j++)
        pHeaderBuf[k / 2 + j] = chBuff[1][j];
    k = xCharsPerLine - nLen[2];
    for (j = 0; j < nLen[2]; j++)
        pHeaderBuf[k + j] = chBuff[2][j];

    return lstrlen(pHeaderBuf);
}

HDC GetPrinterDC(void)
{
    LPDEVNAMES lpDevNames;
    LPDEVMODE  lpDevMode;
    HDC        hDC;

    if (!g_PageSetupDlg.hDevNames) {
        g_PageSetupDlg.Flags |= PSD_RETURNDEFAULT;
        PageSetupDlg(&g_PageSetupDlg);
        g_PageSetupDlg.Flags &= ~PSD_RETURNDEFAULT;
        if (!g_PageSetupDlg.hDevNames)
            return (HDC)-1;
    }

    lpDevNames = (LPDEVNAMES)GlobalLock(g_PageSetupDlg.hDevNames);
    lpDevMode = NULL;
    if (g_PageSetupDlg.hDevMode)
        lpDevMode = (LPDEVMODE)GlobalLock(g_PageSetupDlg.hDevMode);

    hDC = CreateDC((LPTSTR)lpDevNames + lpDevNames->wDriverOffset,
                   (LPTSTR)lpDevNames + lpDevNames->wDeviceOffset,
                   (LPTSTR)lpDevNames + lpDevNames->wOutputOffset,
                   lpDevMode);

    GlobalUnlock(g_PageSetupDlg.hDevNames);
    if (g_PageSetupDlg.hDevMode)
        GlobalUnlock(g_PageSetupDlg.hDevMode);
    return hDC;
}

static const TCHAR szCRLF[] = TEXT("\r\n");

void InsertDateTime(BOOL fCrlf)
{
    SYSTEMTIME time;
    TCHAR szDate[80];
    TCHAR szTime[80];

    GetLocalTime(&time);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, NULL, szDate, 80);
    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &time, NULL, szTime, 80);

    if (fCrlf)
        SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)szCRLF);
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)szTime);
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)TEXT(" "));
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)szDate);
    if (fCrlf)
        SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)szCRLF);
}

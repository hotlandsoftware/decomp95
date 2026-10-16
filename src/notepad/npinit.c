/*
 * npinit.c - Notepad initialisation, command line handling, and file
 * load/save.
 */

#include "notepad.h"
#define NPPAD 20
#include "nppad.h"

TCHAR chPeriod;

/* "/.SETUP [file]": Notepad run by Setup to show a file. */
BOOL SetupCheck(TCHAR *lpCmdLine)
{
    if (lpCmdLine[0] == TEXT('/') && lpCmdLine[1] == TEXT('.') &&
        (lpCmdLine[2] == TEXT('S') || lpCmdLine[2] == TEXT('s')) &&
        (lpCmdLine[3] == TEXT('E') || lpCmdLine[3] == TEXT('e')) &&
        (lpCmdLine[4] == TEXT('T') || lpCmdLine[4] == TEXT('t')) &&
        (lpCmdLine[5] == TEXT('U') || lpCmdLine[5] == TEXT('u')) &&
        (lpCmdLine[6] == TEXT('P') || lpCmdLine[6] == TEXT('p'))) {

        fRunBySetup = TRUE;
        hSysMenuSetup = GetSystemMenu(hwndNP, FALSE);
        hAccel = LoadAccelerators(hInstanceNP, MAKEINTRESOURCE(2));
        lpCmdLine += 7;

        while (*lpCmdLine == TEXT(' ') || *lpCmdLine == TEXT('\t'))
            lpCmdLine++;

        if (*lpCmdLine) {
            GetFileName(szFileName, lpCmdLine);
            fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fp == INVALID_HANDLE_VALUE) {
                switch (GetLastError()) {
                case ERROR_FILE_NOT_FOUND:
                    if (AlertBox(hwndNP, szNN, szFNF, szFileName,
                                 MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                        fp = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL, NULL);
                    break;
                case ERROR_ACCESS_DENIED:
                    AlertBox(hwndNP, szNN, szACCESSDENY, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                case ERROR_INVALID_NAME:
                    AlertBox(hwndNP, szNN, szNVF, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                default:
                    AlertBox(hwndNP, szNN, szDiskError, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                }
            }
            if (fp == INVALID_HANDLE_VALUE)
                return FALSE;
            if (LoadFile(szFileName, FALSE) & 0x80)
                PostMessage(hwndNP, WM_CLOSE, 0, 0);
        }
        return TRUE;
    }
    return FALSE;
}

/* "/P file": print the file and quit. */
BOOL PrintFileCheck(TCHAR *lpCmdLine, int cmdShow)
{
    int  nErr;
    TCHAR *szMsg;

    if (lpCmdLine[0] == TEXT('/') &&
        (lpCmdLine[1] == TEXT('P') || lpCmdLine[1] == TEXT('p'))) {
        lpCmdLine += 2;
        while (*lpCmdLine == TEXT(' ') || *lpCmdLine == TEXT('\t'))
            lpCmdLine++;
        if (*lpCmdLine) {
            ShowWindow(hwndNP, cmdShow);
            GetFileName(szFileName, lpCmdLine);
            fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fp == INVALID_HANDLE_VALUE) {
                AlertBox(hwndNP, szNN, szCantOpenPrn, szFileName, MB_OK | MB_ICONEXCLAMATION);
                return TRUE;
            }
            if (LoadFile(szFileName, TRUE) & 0x80)
                return TRUE;
            nErr = NpPrint();
            if (nErr < 0 && nErr != -3) {
                if (nErr == -4)
                    szMsg = szNEDSTP;
                else if (nErr == -5)
                    szMsg = szNEMTP;
                else
                    szMsg = szCantPrint;
                AlertBox(hwndNP, szNN, szMsg, fUntitled ? szUntitled : szFileName,
                         MB_OK | MB_ICONEXCLAMATION);
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL NPInit(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            LPSTR lpCmdLine, int cmdShow)
{
    DWORD  dwStart, dwEnd;
    TCHAR *pch;

    if (!NPInitStrings(hInstance))
        return FALSE;

    hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(1));
    if (!hWaitCursor || !hAccel)
        return FALSE;

    if (!hPrevInstance && !NPRegister(hInstance))
        return FALSE;

    chPeriod = TEXT('.');
    hInstanceNP = hInstance;
    InitDefaultFont();

    hwndNP = CreateWindowEx(0, szNotepad, TEXT(""), WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                            NULL, NULL, hInstance, NULL);
    if (!hwndNP)
        return FALSE;

    DragAcceptFiles(hwndNP, TRUE);

    g_PageSetupDlg.hwndOwner = hwndNP;
    g_PageSetupDlg.hDevMode = NULL;
    g_PageSetupDlg.hDevNames = NULL;
    g_PageSetupDlg.hInstance = hInstanceNP;
    g_PageSetupDlg.lStructSize = sizeof(PAGESETUPDLG);
    InitPageSetupDefaults();

    hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT(""),
                              WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                              ES_MULTILINE | ES_NOHIDESEL,
                              0, 0, 600, 400, hwndNP, (HMENU)15, hInstance, NULL);
    if (!hwndEdit)
        return FALSE;

    hFont = GetStockObject(SYSTEM_FIXED_FONT);
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
    NpWinIniChange();

    szSearch[0] = 0;
    hEdit = LocalAlloc(LHND, 1);
    PostMessage(hwndEdit, EM_LIMITTEXT, 0, 0);
    SetTitle(szUntitled);

    if (!SetupCheck(lpCmdLine)) {
        if (PrintFileCheck(lpCmdLine, cmdShow)) {
            PostMessage(hwndNP, WM_CLOSE, 0, 0);
            return TRUE;
        }
        if (*lpCmdLine) {
            GetFileName(szFileName, lpCmdLine);
            fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fp == INVALID_HANDLE_VALUE) {
                switch (GetLastError()) {
                case ERROR_FILE_NOT_FOUND:
                    if (AlertBox(hwndNP, szNN, szFNF, szFileName,
                                 MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                        fp = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL, NULL);
                    break;
                case ERROR_ACCESS_DENIED:
                    AlertBox(hwndNP, szNN, szACCESSDENY, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                case ERROR_INVALID_NAME:
                    AlertBox(hwndNP, szNN, szNVF, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                default:
                    AlertBox(hwndNP, szNN, szDiskError, szFileName, MB_OK | MB_ICONEXCLAMATION);
                    break;
                }
            }
            if (fp != INVALID_HANDLE_VALUE && (LoadFile(szFileName, FALSE) & 0x80)) {
                PostMessage(hwndNP, WM_CLOSE, 0, 0);
                return TRUE;
            }
        }
    }

    if (fRunBySetup)
        SetWindowLong(hwndNP, GWL_STYLE,
                      (WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX) | WS_VSCROLL | WS_HSCROLL);
    ShowWindow(hwndNP, cmdShow);

    /* Filter: "Text Documents\0*.txt\0All Files (*.*)\0*.*\0\0" */
    lstrcpy(szFilterSpec, szTextFiles);
    pch = szFilterSpec + lstrlen(szFilterSpec) + 1;
    lstrcpy(pch, szTXT + 1);
    pch += lstrlen(pch) + 1;
    lstrcpy(pch, szAllFiles);
    pch += lstrlen(pch) + 1;
    lstrcpy(pch, TEXT("*.*"));
    pch[lstrlen(pch) + 1] = 0;

    szCustFilterSpec[0] = 0;
    lstrcpy(szCustFilterSpec + 1, szTXT + 1);

    OFN.lpstrFileTitle    = NULL;
    OFN.lStructSize       = sizeof(OPENFILENAME);
    OFN.hwndOwner         = hwndNP;
    OFN.lpfnHook          = NULL;
    OFN.Flags             = 0;
    OFN.lpstrTitle        = NULL;
    OFN.nMaxCustFilter    = CCHFILTERMAX;
    OFN.lpstrInitialDir   = NULL;
    FR.hwndOwner          = hwndNP;
    OFN.nFilterIndex      = 1;
    FR.wReplaceWithLen    = 0;
    FR.lpstrReplaceWith   = NULL;
    FR.lpfnHook           = NULL;
    OFN.nMaxFile          = MAX_PATH;
    FR.lStructSize        = sizeof(FINDREPLACE);
    FR.Flags              = FR_DOWN | FR_HIDEWHOLEWORD;

    if (!(wFRMsg = RegisterWindowMessage(TEXT("commdlg_FindReplace"))))
        return FALSE;

    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
    SendMessage(hwndEdit, EM_SETSEL, dwStart, dwEnd);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
    return TRUE;
}

BOOL NPRegister(HINSTANCE hInstance)
{
    WNDCLASSEX NPClass;

    NPClass.cbSize        = sizeof(NPClass);
    NPClass.hCursor       = LoadCursor(NULL, GetSystemMetrics(SM_PENWINDOWS) ? IDC_ARROW : IDC_IBEAM);
    NPClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(2));
    NPClass.hIconSm       = NULL;
    NPClass.lpszMenuName  = MAKEINTRESOURCE(1);
    NPClass.hInstance     = hInstance;
    NPClass.lpszClassName = szNotepad;
    NPClass.lpfnWndProc   = NPWndProc;
    NPClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    NPClass.style         = CS_BYTEALIGNCLIENT;
    NPClass.cbClsExtra    = 0;
    NPClass.cbWndExtra    = 0;

    return RegisterClassEx(&NPClass) != 0;
}

void InitDefaultFont(void)
{
}

BOOL SaveFile(HWND hwndParent, TCHAR *szFile, BOOL fSaveAs)
{
    BOOL   fNew = FALSE;
    BOOL   fFmt;
    TCHAR *lpch;
    UINT   nChars;

    SetCursor(hWaitCursor);
    if (!fSaveAs) {
        fp = CreateFile(szFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    } else {
        fp = (HANDLE)_lopen(szFile, OF_READWRITE);
        if ((int)fp < 0) {
            fp = (HANDLE)_lcreat(szFile, 0);
            fNew = (int)fp >= 0;
        }
    }

    if (fp == INVALID_HANDLE_VALUE) {
        if (fSaveAs)
            AlertBox(hwndParent, szNN, szCreateErr, szFile, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    fFmt = SendMessage(hwndEdit, EM_FMTLINES, FALSE, 0);
    nChars = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    hEdit = LocalReAlloc(hEdit, nChars + 1, LHND);
    if (!hEdit) {
        AlertBox(hwndParent, szNN, szWriteErr, szFile, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    lpch = (TCHAR *)LocalLock(hEdit);
    SendMessage(hwndEdit, WM_GETTEXT, nChars + 1, (LPARAM)lpch);
    if (_lwrite((HFILE)fp, lpch, nChars) != nChars) {
        LocalUnlock(hEdit);
        _lclose((HFILE)fp);
        if (fNew)
            DeleteFile(szFile);
        AlertBox(hwndParent, szNN, szWriteErr, szFile, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    _lwrite((HFILE)fp, lpch, 0);        /* truncate */
    SendMessage(hwndEdit, EM_SETMODIFY, FALSE, 0);
    SetTitle(szFile);
    fUntitled = FALSE;
    _lclose((HFILE)fp);
    if (fFmt)
        SendMessage(hwndEdit, EM_FMTLINES, TRUE, 0);
    LocalUnlock(hEdit);
    return TRUE;
}

/* Offers to open (or print) a file that is too large for Notepad in
 * WordPad.  Returns 0x81 if WordPad was started, 0x82 if declined. */
int TryWordPad(HWND hwndParent, TCHAR *szFile, BOOL fPrint)
{
    TCHAR szCmdLine[300];
    TCHAR szWordPad[MAX_PATH];
    STARTUPINFO si;
    UINT  idCmd;

    GetStartupInfo(&si);
    if (MessageBox(hwndParent, szUseWordpad, NULL, MB_YESNO | MB_ICONQUESTION) == IDYES) {
        idCmd = fPrint ? 55 : 54;
        LoadString(hInstanceNP, 56, szWordPad, MAX_PATH);
        LoadString(hInstanceNP, idCmd, szCmdLine, 300);
        lstrcat(szCmdLine, TEXT("\""));
        lstrcat(szCmdLine, szFile);
        lstrcat(szCmdLine, TEXT("\""));
        if ((UINT)ShellExecute(NULL, NULL, szWordPad, szCmdLine, NULL, SW_SHOWNORMAL) <= 32) {
            MessageBox(hwndParent, szWordpadFail, szNN, MB_OK | MB_ICONEXCLAMATION);
            return 0;
        }
        return 0x81;
    }
    return 0x82;
}

int LoadFile(TCHAR *sz, BOOL fPrint)
{
    TCHAR *lpch, *p;
    UINT   len, i;
    BOOL   fLog;
    TCHAR  szSave[MAX_PATH];

    if (fp == INVALID_HANDLE_VALUE) {
        AlertBox(hwndNP, szNN, szDiskError, sz, MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    SetCursor(hWaitCursor);
    len = _llseek((HFILE)fp, 0, 2);
    _llseek((HFILE)fp, 0, 0);

    if ((int)len > 0xFFFF)
        goto TooBig;
    if (!(hEdit = LocalReAlloc(hEdit, len + 1, LHND)))
        goto TooBig;

    SendMessage(hwndEdit, EM_SETSEL, 0, 0);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
    lpch = (TCHAR *)LocalLock(hEdit);
    _lread((HFILE)fp, lpch, len);
    _lclose((HFILE)fp);

    /* Edit controls stop at NULs: turn them into spaces. */
    for (p = lpch, i = len; i; p++, i--)
        if (!*p)
            *p = TEXT(' ');
    lpch[len] = 0;

    if (len && lstrlen(lpch) < (int)(len - 1)) {
        LocalUnlock(hEdit);
        AlertBox(hwndNP, szNN, szInvalidCP, sz, MB_OK | MB_ICONEXCLAMATION);
        New(FALSE);
        return 0;
    }

    fLog = *lpch++ == TEXT('.') && *lpch++ == TEXT('L') &&
           *lpch++ == TEXT('O') && *lpch == TEXT('G');

    LocalUnlock(hEdit);
    lstrcpy(szFileName, sz);
    SetTitle(sz);
    fUntitled = FALSE;

    SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)TEXT(""));
    lpch = (TCHAR *)LocalLock(hEdit);
    fIgnoreErrSpace = TRUE;
    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)lpch);
    LocalUnlock(hEdit);

    if (!fIgnoreErrSpace) {         /* edit control ran out of space */
        SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
        goto UseWordPad;
    }
    fIgnoreErrSpace = FALSE;

    if (fLog) {                     /* ".LOG" files get a time stamp */
        SendMessage(hwndEdit, EM_SETSEL, len, len);
        SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
        InsertDateTime(TRUE);
    }
    SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwndEdit, NULL, TRUE);
    UpdateWindow(hwndEdit);
    return 1;

TooBig:
    _lclose((HFILE)fp);
UseWordPad:
    lstrcpy(szSave, sz);
    New(FALSE);
    return TryWordPad(hwndNP, szSave, fPrint);
}

void New(BOOL fCheck)
{
    if (fCheck && !CheckSave(FALSE))
        return;

    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)TEXT(""));
    fUntitled = TRUE;
    lstrcpy(szFileName, szUntitled);
    SetTitle(szFileName);
    SendMessage(hwndEdit, EM_SETSEL, 0, 0);
    SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
    hEdit = LocalReAlloc(hEdit, 0, LHND);
    szSearch[0] = 0;
}

/* Appends ".txt" if the file name has no extension. */
void AddExt(TCHAR *sz)
{
    TCHAR *pch = sz + lstrlen(sz);
    int    ch = *pch;

    while (ch != TEXT('.') && ch != TEXT('\\') && ch != TEXT(':') && pch > sz) {
        pch = CharPrev(sz, pch);
        ch = *pch;
    }

    if (*pch != TEXT('.'))
        lstrcat(sz, TEXT(".txt"));
}

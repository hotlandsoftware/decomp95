/*
 * npmain.c - Notepad main window: entry point, window procedure, menu
 * commands and common helpers.
 */

#include "notepad.h"
#define NPPAD 31
#include "nppad.h"

HWND    hwndNP = NULL;
HWND    hwndEdit = NULL;
HWND    hDlgFind = NULL;
BOOL    fUntitled = TRUE;
WORD    iEditState = 0;
BOOL    fRunBySetup = FALSE;
BOOL    fWrap = FALSE;
int     iUnused1 = 0;
TCHAR   szNotepad[] = TEXT("Notepad");
BOOL    fInSaveAsDlg = FALSE;
DWORD   dwSelStart = 0;
DWORD   dwSelEnd = 0;
BOOL    fIgnoreErrSpace = FALSE;
BOOL    fInCheckSave = FALSE;

/* Message strings: initialised with their resource IDs, replaced by
 * pointers to the loaded text in NPInitStrings. */
TCHAR *szDiskError   = (TCHAR *)IDS_DISKERROR;
TCHAR *szFNF         = (TCHAR *)IDS_FNF;
TCHAR *szSCBC        = (TCHAR *)IDS_SCBC;
TCHAR *szSaveChanges = (TCHAR *)IDS_SAVECHANGES;
TCHAR *szUntitled    = (TCHAR *)IDS_UNTITLED;
TCHAR *szCFS         = (TCHAR *)IDS_CFS;
TCHAR *szErrSpace    = (TCHAR *)IDS_ERRSPACE;
TCHAR *szNpTitle     = (TCHAR *)IDS_NOTEPAD;
TCHAR *szFTL         = (TCHAR *)IDS_FTL;
TCHAR *szNN          = (TCHAR *)IDS_NN;
TCHAR *szPasteErr    = (TCHAR *)IDS_PASTEERR;
TCHAR *szWriteErr    = (TCHAR *)IDS_WRITEERR;
TCHAR *szInvalidCP   = (TCHAR *)IDS_INVALIDCP;
TCHAR *szCantPrint   = (TCHAR *)IDS_CANTPRINT;
TCHAR *szNVF         = (TCHAR *)IDS_NVF;
TCHAR *szNVF2        = (TCHAR *)IDS_NVF2;
TCHAR *szNEDSTP      = (TCHAR *)IDS_NEDSTP;
TCHAR *szNEMTP       = (TCHAR *)IDS_NEMTP;
TCHAR *szCreateErr   = (TCHAR *)IDS_CREATEERR;
TCHAR *szNoWW        = (TCHAR *)IDS_NOWW;
TCHAR *szMerge       = (TCHAR *)IDS_MERGE1;
TCHAR *szTXT         = (TCHAR *)IDS_TXT;
TCHAR *szHelpFile    = (TCHAR *)IDS_HELPFILE;
TCHAR *szCantOpenPrn = (TCHAR *)IDS_CANTOPENPRN;
TCHAR *szTextFiles   = (TCHAR *)IDS_TEXTFILES;
TCHAR *szAllFiles    = (TCHAR *)IDS_ALLFILES;
TCHAR *szOpenDlgOpen = (TCHAR *)IDS_OPENDLGOPEN;
TCHAR *szLoadDrvFail = (TCHAR *)IDS_LOADDRVFAIL;
TCHAR *szACCESSDENY  = (TCHAR *)IDS_ACCESSDENY;
TCHAR *szUseWordpad  = (TCHAR *)IDS_USEWORDPAD;
TCHAR *szWordpadFail = (TCHAR *)IDS_WORDPADFAIL;

TCHAR **rgsz[] = {
    &szDiskError, &szFNF, &szSCBC, &szSaveChanges, &szUntitled, &szErrSpace,
    &szCFS, &szNpTitle, &szFTL, &szNN, &szPasteErr, &szWriteErr,
    &szInvalidCP, &szCantPrint, &szNVF, &szNVF2, &szNEDSTP, &szNEMTP,
    &szCreateErr, &szNoWW, &szMerge, &szTXT, &szHelpFile, &szCantOpenPrn,
    &szTextFiles, &szAllFiles, &szOpenDlgOpen, &szLoadDrvFail,
    &szACCESSDENY, &szUseWordpad, &szWordpadFail,
    NULL
};

/* Dropped file name (file static, first in .bss). */
static TCHAR szDropFile[128];

/* Uninitialised globals.  These are "common" symbols; the final layout is
 * fixed by the definitions in npbss.asm, which these resolve to. */
TCHAR   chPageText[2][PT_LEN];
HINSTANCE hInstanceNP;
TCHAR   szFilterSpec[CCHFILTERMAX];
TCHAR   szSearch[CCHKEYMAX];
HMENU   hSysMenuSetup;
FINDREPLACE FR;
HACCEL  hAccel;
UINT    wFRMsg;
HCURSOR hWaitCursor;
TCHAR   szCustFilterSpec[CCHFILTERMAX];
HANDLE  fp;
HANDLE  hEdit;
OPENFILENAME OFN;
TCHAR   szFileName[MAX_PATH];
PAGESETUPDLG g_PageSetupDlg;
HFONT   hFont;
SHORT   chMerge;


int ModuleEntry(void)
{
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();
    int ret;

    if (*pszCmdLine == '\"') {
        /* Skip to the closing quote of the program name. */
        while (*++pszCmdLine && (*pszCmdLine != '\"'))
            ;
        if (*pszCmdLine == '\"')
            pszCmdLine++;
    } else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

    /* Skip white space before the first argument. */
    while (*pszCmdLine && (*pszCmdLine <= ' '))
        pszCmdLine++;

    si.dwFlags = 0;
    GetStartupInfo(&si);

    ret = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
                  si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(ret);
    return ret;
}

void FreePageSetupHandles(void)
{
    if (g_PageSetupDlg.hDevMode)
        GlobalFree(g_PageSetupDlg.hDevMode);
    if (g_PageSetupDlg.hDevNames)
        GlobalFree(g_PageSetupDlg.hDevNames);
}

void InitPageSetupDefaults(void)
{
    TCHAR szMeasure[2];

    g_PageSetupDlg.lpfnPageSetupHook = PageSetupHookProc;
    g_PageSetupDlg.lpPageSetupTemplateName = MAKEINTRESOURCE(14);

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szMeasure, 2);
    if (szMeasure[0] == TEXT('1')) {
        /* U.S. system: thousandths of inches */
        g_PageSetupDlg.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES |
                               PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATE;
        g_PageSetupDlg.rtMargin.top = 1000;
        g_PageSetupDlg.rtMargin.bottom = 1000;
        g_PageSetupDlg.rtMargin.left = 750;
        g_PageSetupDlg.rtMargin.right = 750;
    } else {
        /* metric system: hundredths of millimeters */
        g_PageSetupDlg.Flags = PSD_MARGINS | PSD_INHUNDREDTHSOFMILLIMETERS |
                               PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATE;
        g_PageSetupDlg.rtMargin.top = 2500;
        g_PageSetupDlg.rtMargin.bottom = 2500;
        g_PageSetupDlg.rtMargin.left = 2000;
        g_PageSetupDlg.rtMargin.right = 2000;
    }
}

void WINAPI NpResizeEdit(int cx, int cy)
{
    InvalidateRect(hwndEdit, NULL, TRUE);
    MoveWindow(hwndEdit, 0, 0, cx, cy, TRUE);
}

/* Handles WM_COMMAND from the menu and accelerators.  Returns FALSE for
 * commands it does not know, so the caller can pass them on. */
BOOL NPCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    TCHAR szNewName[MAX_PATH] = TEXT("");
    TCHAR *szMsg;
    DWORD dwErr;
    int   ret;
    int   cmd = LOWORD(wParam);

    switch (cmd) {
    case M_SAVE:
        if (fUntitled || !SaveFile(hwndNP, szFileName, FALSE))
            goto SaveAs;
        break;

    case M_HELP:
        if (!WinHelp(hwndNP, szHelpFile, HELP_FINDER, 0))
            AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONHAND);
        break;

    case M_SELECTALL:
        SendMessage(hwndEdit, EM_SETSEL, 0,
                    SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0));
        SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
        break;

    case M_FINDNEXT:
        if (szSearch[0]) {
            Search(szSearch);
            break;
        }
        goto Find;

    case M_NEW:
        New(TRUE);
        break;

    case M_OPEN:
        if (!CheckSave(FALSE))
            break;
        OFN.lpstrFile = szNewName;
        lstrcpy(szNewName, szCustFilterSpec + 1);
        OFN.lpstrFilter = szFilterSpec;
        OFN.lpstrCustomFilter = szCustFilterSpec;
        OFN.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
        OFN.lpstrDefExt = szTXT + 3;
        if (GetOpenFileName(&OFN)) {
            fp = CreateFile(szNewName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            ret = LoadFile(szNewName, FALSE);
            if (ret && ret != 0x81)
                break;
            /* reopen the current file; 0x81 means it went to WordPad */
            fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (ret != 0x81)
                break;

    /* File/Exit shares the close request with the WordPad hand-off above. */
    case M_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        } else if (CommDlgExtendedError()) {
            AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONHAND);
        }
        break;

    case M_ABOUT:
        ShellAbout(hwndNP, szNN, TEXT(""), LoadIcon(hInstanceNP, MAKEINTRESOURCE(2)));
        break;

    case M_DATETIME:
        InsertDateTime(FALSE);
        break;

    case M_PRINT:
        ret = NpPrint();
        if (ret < 0 && ret != -3 && !fPrintAborted) {
            switch (ret) {
            case -5: szMsg = szNEMTP; break;
            case -4: szMsg = szNEDSTP; break;
            default: szMsg = szCantPrint; break;
            }
            AlertBox(hwndNP, szNN, szMsg, fUntitled ? szUntitled : szFileName,
                     MB_OK | MB_ICONEXCLAMATION);
        }
        break;

    case M_UNDO:
        SendMessage(hwndEdit, EM_UNDO, 0, 0);
        break;

    case M_WW:
        if (NpReCreate(!fWrap))
            fWrap = !fWrap;
        else
            AlertBox(hwndNP, szNN, szNoWW, NULL, MB_OK | MB_ICONEXCLAMATION);
        break;

    case WM_CUT:
    case WM_COPY:
    case WM_CLEAR:
        {
            DWORD sel = SendMessage(hwndEdit, EM_GETSEL, 0, 0);
            if (HIWORD(sel) == LOWORD(sel))
                break;
        }
        /* fall through */
    case WM_PASTE:
        {
            HWND hwndFocus = GetFocus();
            if (hwndFocus == hwndEdit || hwndFocus == hwndNP)
                PostMessage(hwndEdit, cmd, 0, 0);
        }
        break;

    case M_FIND:
    Find:
        if (hDlgFind) {
            SetFocus(hDlgFind);
            break;
        }
        FR.lpstrFindWhat = szSearch;
        FR.wFindWhatLen = CCHKEYMAX;
        hDlgFind = FindText(&FR);
        break;

    case M_PAGESETUP:
        for (;;) {
            if (PageSetupDlg(&g_PageSetupDlg)) {
                lstrcpy(chPageText[0], chPageTextSave[0]);
            lstrcpy(chPageText[1], chPageTextSave[1]);
                return TRUE;
            }
            dwErr = CommDlgExtendedError();
            if (dwErr == PDERR_PRINTERNOTFOUND || dwErr == PDERR_DNDMMISMATCH || dwErr == PDERR_DEFAULTDIFFERENT) {
                FreePageSetupHandles();
                g_PageSetupDlg.hDevMode = g_PageSetupDlg.hDevNames = NULL;
            } else {
                if (dwErr == CDERR_DIALOGFAILURE || dwErr == CDERR_INITIALIZATION ||
                dwErr == CDERR_LOADSTRFAILURE || dwErr == CDERR_LOADRESFAILURE ||
                dwErr == PDERR_LOADDRVFAILURE || dwErr == PDERR_GETDEVMODEFAIL)
                AlertBox(hwndNP, szNN, dwErr == PDERR_LOADDRVFAILURE ? szLoadDrvFail : szErrSpace,
                         NULL, MB_OK | MB_ICONHAND);
                return TRUE;
            }
        }

    case M_SAVEAS:
    SaveAs:
        OFN.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
        OFN.lpstrFilter = szFilterSpec;
        OFN.lpstrCustomFilter = szCustFilterSpec;
        OFN.lpstrFile = szNewName;
        OFN.lpstrDefExt = szTXT + 3;
        if (!fUntitled)
            lstrcpy(szNewName, szFileName);
        else
            lstrcpy(szNewName, szUntitled);
        fInSaveAsDlg = TRUE;
        if (GetSaveFileName(&OFN) && SaveFile(hwnd, szNewName, TRUE)) {
            lstrcpy(szFileName, szNewName);
        } else if (CommDlgExtendedError()) {
            AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONHAND);
        }
        fInSaveAsDlg = FALSE;
        break;

    case 15:
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

void OpenDroppedFile(void)
{

    if (!CheckSave(FALSE))
        return;

    fp = CreateFile(szDropFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fp == INVALID_HANDLE_VALUE) {
        switch (GetLastError()) {
        case ERROR_ACCESS_DENIED:
            AlertBox(hwndNP, szNN, szACCESSDENY, szDropFile, MB_OK | MB_ICONEXCLAMATION);
            break;
        case ERROR_INVALID_NAME:
            AlertBox(hwndNP, szNN, szNVF, szDropFile, MB_OK | MB_ICONEXCLAMATION);
            break;
        default:
            AlertBox(hwndNP, szNN, szDiskError, szDropFile, MB_OK | MB_ICONEXCLAMATION);
            break;
        }
        fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    } else {
        switch (LoadFile(szDropFile, FALSE)) {
        case 0:
            fp = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            break;
        case 0x81:
            PostMessage(hwndNP, WM_CLOSE, 0, 0);
            break;
        }
    }
}

void WINAPI NpDropFiles(HDROP hDrop, HWND hwnd)
{
    if (DragQueryFile(hDrop, (UINT)-1, NULL, 0)) {
        DragQueryFile(hDrop, 0, szDropFile, sizeof(szDropFile));
        SetForegroundWindow(hwnd);
        OpenDroppedFile();
    }
    DragFinish(hDrop);
}

/* Asks whether to save changes; returns FALSE if the user cancelled. */
BOOL CheckSave(BOOL fSysModal)
{
    int   mdResult = IDOK;
    TCHAR szNewName[MAX_PATH] = TEXT("");
    TCHAR *szFile;
    HWND  hwndPopup;

    if (!fUntitled || SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0)) {
        if (fInCheckSave) {
            if (!fSysModal)
                return FALSE;
            hwndPopup = GetLastActivePopup(hwndNP);
            ShowWindow(hwndPopup, SW_HIDE);
            EndDialog(hwndPopup, IDCANCEL);
        }
        if (SendMessage(hwndEdit, EM_GETMODIFY, 0, 0)) {
            if (fUntitled)
                szFile = szUntitled;
            else if (*(WORD *)(szFileName + 2) == 0x5c5c)   /* "\\" -- UNC */
                szFile = szFileName + 2;
            else
                szFile = szFileName;
            fInCheckSave = TRUE;
            mdResult = AlertBox(hwndNP, szNN, szSaveChanges, szFile,
                                MB_YESNOCANCEL | MB_ICONEXCLAMATION |
                                (fSysModal ? MB_SYSTEMMODAL : MB_APPLMODAL));
            fInCheckSave = FALSE;
            if (mdResult == IDYES) {
                if (fUntitled) {
                    lstrcpy(szNewName, szCustFilterSpec + 1);
                } else {
                    if (SaveFile(hwndNP, szFileName, FALSE))
                        return TRUE;
                    lstrcpy(szNewName, szFileName);
                }
                OFN.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
                OFN.lpstrFilter = szFilterSpec;
                OFN.lpstrCustomFilter = szCustFilterSpec;
                OFN.lpstrFile = szNewName;
                fInSaveAsDlg = TRUE;
                OFN.lpstrDefExt = szTXT + 3;
                if (GetSaveFileName(&OFN)) {
                    lstrcpy(szNewName, OFN.lpstrFile);
                    if (SaveFile(hwndNP, szNewName, TRUE))
                        lstrcpy(szFileName, szNewName);
                } else {
                    fInCheckSave = TRUE;
                    mdResult = IDCANCEL;
                    if (CommDlgExtendedError())
                        AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONHAND);
                }
            }
        }
        fInCheckSave = FALSE;
        return mdResult != IDCANCEL;
    }
    return TRUE;
}

LRESULT WINAPI NPWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD dwFlags;

    switch (message) {
    case WM_SIZE:
        switch (wParam) {
        case SIZENORMAL:
        case SIZEFULLSCREEN:
            NpResizeEdit(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
            break;
        case SIZEICONIC:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
        if (CheckSave(FALSE)) {
            if (!WinHelp(hwndNP, szHelpFile, HELP_QUIT, 0))
                AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONHAND);
            DestroyWindow(hwndNP);
            DeleteObject(hFont);
        }
        break;

    case WM_ACTIVATE:
        if ((LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) &&
            !IsIconic(hwndNP))
            SetFocus(hwndEdit);
        break;

    case WM_SETFOCUS:
        if (!IsIconic(hwndNP))
            SetFocus(hwndEdit);
        break;

    case WM_KILLFOCUS:
        SendMessage(hwndEdit, message, wParam, lParam);
        break;

    case WM_WININICHANGE:
        NpWinIniChange();
        break;

    case WM_QUERYENDSESSION:
        if (fInSaveAsDlg) {
            MessageBeep(0);
            MessageBeep(0);
            MessageBox(hwndNP, szOpenDlgOpen, szNN, MB_OK);
            return FALSE;
        }
        return CheckSave(TRUE);

    case WM_ACTIVATEAPP:
        if (wParam) {
            if (dwSelStart || dwSelEnd) {
                SendMessage(hwndEdit, EM_SETSEL, dwSelStart, dwSelEnd);
                SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
            }
        } else {
            SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&dwSelStart, (LPARAM)&dwSelEnd);
            if (dwSelStart == dwSelEnd) {
                dwSelStart = 0;
                dwSelEnd = 0;
            } else {
                SendMessage(hwndEdit, EM_SETSEL, dwSelStart, dwSelEnd);
                SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
            }
        }
        break;

    case WM_COMMAND:
        if ((HWND)lParam == hwndEdit &&
            (HIWORD(wParam) == EN_ERRSPACE || HIWORD(wParam) == EN_MAXTEXT)) {
            if (iEditState == 1) {
                iEditState = 2;
            } else {
                if (fIgnoreErrSpace && HIWORD(wParam) == EN_ERRSPACE) {
                    fIgnoreErrSpace = FALSE;
                    break;
                }
                AlertBox(hwndNP, szNN, szErrSpace, NULL, MB_OK | MB_ICONEXCLAMATION);
                break;
            }
        }
        if (!NPCommand(hwnd, wParam, lParam))
            return DefWindowProc(hwnd, message, wParam, lParam);
        break;

    case WM_SYSCOMMAND:
        if (fRunBySetup &&
            (wParam == SC_MINIMIZE || wParam == SC_NEXTWINDOW || wParam == SC_PREVWINDOW))
            break;
        DefWindowProc(hwnd, message, wParam, lParam);
        break;

    case WM_INITMENU:
        NpUpdateMenu(hwnd);
        break;

    case WM_INITMENUPOPUP:
        if (fRunBySetup && HIWORD(lParam))
            EnableMenuItem(hSysMenuSetup, SC_MINIMIZE, MF_GRAYED | MF_DISABLED);
        break;

    case WM_DROPFILES:
        NpDropFiles((HDROP)wParam, hwnd);
        break;

    default:
        if (message == wFRMsg) {
            dwFlags = ((LPFINDREPLACE)lParam)->Flags;
            fReverse = !(dwFlags & FR_DOWN);
            fCase = (dwFlags & FR_MATCHCASE) >> 2;
            if (dwFlags & FR_FINDNEXT)
                Search(szSearch);
            else if (dwFlags & FR_DIALOGTERM)
                hDlgFind = NULL;
            break;
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpAnsiCmdLine, int cmdShow)
{
    MSG msg;

    if (!NPInit(hInstance, hPrevInstance, lpAnsiCmdLine, cmdShow)) {
        msg.wParam = FALSE;
    } else {
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (!hDlgFind || !IsDialogMessage(hDlgFind, &msg)) {
                if (!TranslateAccelerator(hwndNP, hAccel, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        FreePageSetupHandles();
    }
    return msg.wParam;
}

void SetTitle(TCHAR *sz)
{
    TCHAR szWindowText[MAX_PATH + 50 + 2];
    TCHAR szFileName2[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA info;

    if (lstrcmp(sz, szUntitled) == 0) {
        lstrcpy(szWindowText, sz);
    } else {
        hFind = FindFirstFile(sz, &info);
        if (hFind != INVALID_HANDLE_VALUE) {
            lstrcpy(szFileName2, info.cFileName);
            FindClose(hFind);
        } else {
            lstrcpy(szFileName2, sz);
        }
        GetFileTitle(szFileName2, szWindowText, MAX_PATH);
    }
    lstrcat(szWindowText, szNpTitle);
    SetWindowText(hwndNP, szWindowText);
}

TCHAR *PFileInPath(TCHAR *sz)
{
    TCHAR *pch = sz;
    TCHAR *psz;

    for (psz = sz; *psz; psz = CharNext(psz)) {
        if (*psz == TEXT(':') || *psz == TEXT('\\'))
            pch = psz;
    }
    if (pch != sz)
        pch++;
    return pch;
}

void NpUpdateMenu(HWND hwnd)
{
    HMENU hMenu = GetMenu(hwndNP);
    DWORD dwSel;
    BOOL  fNoSel;
    BOOL  fClipText;
    TCHAR szPrinter[20];

    dwSel = SendMessage(hwndEdit, EM_GETSEL, 0, 0);
    fNoSel = (HIWORD(dwSel) == LOWORD(dwSel));
    EnableMenuItem(GetSubMenu(hMenu, 1), WM_CUT, fNoSel);
    EnableMenuItem(GetSubMenu(hMenu, 1), WM_COPY, fNoSel);
    if (OpenClipboard(hwnd)) {
        fClipText = IsClipboardFormatAvailable(CF_TEXT);
        CloseClipboard();
    }
    EnableMenuItem(GetSubMenu(hMenu, 1), WM_PASTE, !fClipText);
    EnableMenuItem(GetSubMenu(hMenu, 1), WM_CLEAR, fNoSel);
    EnableMenuItem(GetSubMenu(hMenu, 1), M_UNDO,
                   !SendMessage(hwndEdit, EM_CANUNDO, 0, 0));
    CheckMenuItem(GetSubMenu(hMenu, 1), M_WW, fWrap ? MF_CHECKED : MF_UNCHECKED);

    EnableMenuItem(hMenu, M_PRINT,
                   (g_PageSetupDlg.hDevNames ||
                    GetProfileString(TEXT("windows"), TEXT("device"), TEXT(""),
                                     szPrinter, sizeof(szPrinter)))
                   ? MF_ENABLED : MF_GRAYED);
}

void NpWinIniChange(void)
{
    if (0)      /* section check compiled out; its string remains */
        lstrcmpi(szFileName, TEXT("intl"));
    InitDefaultFont();
}


/* Copies szSrc to szDst, replacing the first chMerge sequence ("%%") with
 * szMerge.  Returns TRUE if a merge took place. */
BOOL MergeStrings(TCHAR *szSrc, TCHAR *szMerge, TCHAR *szDst)
{
    while (*szSrc != chMerge) {
        *szDst++ = *szSrc;
        if (!*szSrc++)
            return FALSE;
    }

    if (szMerge) {
        while (*szMerge)
            *szDst++ = *szMerge++;
    }
    /* skip the two-character merge sequence */
    szSrc++;
    szSrc++;
    while (*szDst++ = *szSrc++)
        ;
    return TRUE;
}

int AlertBox(HWND hwndParent, TCHAR *szCaption, TCHAR *szText1,
             TCHAR *szText2, WORD style)
{
    TCHAR szMessage[MAX_PATH + 128];

    MergeStrings(szText1, szText2, szMessage);
    return MessageBox(hwndParent, szMessage, szCaption, style);
}

void GetFileName(TCHAR *szFile, TCHAR *szCmdLine)
{
    TCHAR *pch = szFile;
    TCHAR chQuote;
    HANDLE hFind;
    WIN32_FIND_DATA info;

    chQuote = *szCmdLine;
    if (chQuote == TEXT('\"') || chQuote == TEXT('\'')) {
        do {
            if (!*++szCmdLine || *szCmdLine == chQuote)
                break;
            *pch++ = *szCmdLine;
        } while (pch != szFile + MAX_PATH - 1);
        *pch = 0;
    } else {
        lstrcpyn(szFile, szCmdLine, MAX_PATH);
    }

    hFind = FindFirstFile(szFile, &info);
    if (hFind == INVALID_HANDLE_VALUE) {
        AddExt(szFile);
        hFind = FindFirstFile(szFile, &info);
        if (hFind == INVALID_HANDLE_VALUE)
            return;
    }
    FindClose(hFind);
}

/* Turns the '#' separators of a filter string into NULs. */
void FixFilterString(TCHAR *sz)
{
    for (; *sz; sz++)
        if (*sz == TEXT('#'))
            *sz = 0;
}

BOOL NPInitStrings(HINSTANCE hInstance)
{
    TCHAR  ***ppsz;
    TCHAR  *pch;
    HANDLE hStrings;
    int    cchRemaining = 0x1000;
    int    cch;
    int    i;

    hStrings = LocalAlloc(LHND, 0x1000);
    if (!(pch = (TCHAR *)LocalLock(hStrings)))
        return FALSE;

    for (ppsz = rgsz; *ppsz; ppsz++) {
        cch = 1 + LoadString(hInstance, (WORD)**ppsz, pch, cchRemaining);
        **ppsz = pch;
        pch += cch;
        if (cch > cchRemaining)
            MessageBox(NULL, TEXT("Out of RC string space!!"), TEXT("DEV Error!"), MB_OK);
        cchRemaining -= cch;
    }

    /* Header/footer defaults.  (Loads six strings into a two-entry array.) */
    for (i = 0, pch = chPageText[0]; pch < chPageText[6]; i++, pch += PT_LEN)
        LoadString(hInstance, (WORD)(IDS_HEADER + i), pch, PT_LEN);

    LocalReAlloc(hStrings, 0x1000 - cchRemaining, LHND);
    chMerge = *szMerge;
    return TRUE;
}

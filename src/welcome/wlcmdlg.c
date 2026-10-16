/*
 * wlcmdlg.c - the Welcome dialog, command line and entry point
 */

#include "welcome.h"
#define NPPAD 7
#include "nppad.h"

/*
 * Remove a button (moving the ones below it up) unless the registry value
 * says it applies: fShow is inverted when pszValue under pszKey is "1".
 */
void RemoveButtonIf(HWND hDlg, LPCSTR pszKey, LPCSTR pszValue, BOOL fShow, int id)
{
    HKEY  hkey;
    DWORD cb;
    DWORD dwType;
    char  szValue[10];
    HWND  hwndButton;
    HWND  hwnd;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, pszKey, &hkey) == ERROR_SUCCESS) {
        cb = sizeof(szValue);
        if (RegQueryValueEx(hkey, pszValue, NULL, &dwType, szValue, &cb) == ERROR_SUCCESS &&
            szValue[0] == '1')
            fShow = !fShow;
        RegCloseKey(hkey);
    }

    if (!fShow && (hwndButton = GetDlgItem(hDlg, id)) != NULL) {
        for (id++; id <= IDC_NEXTTIP; id++) {
            if ((hwnd = GetDlgItem(hDlg, id)) != NULL)
                SwapWindows(hwndButton, hwnd);
        }
        DestroyWindow(hwndButton);
    }
}

void InitWelcomeDialog(HWND hDlg)
{
    NPLOC18
    NONCLIENTMETRICS ncm;
    char    szTourFile[MAX_PATH];
    BITMAP  bm;
    RECT    rc;
    HGDIOBJ hbrOld;
    HGDIOBJ hbmOld;
    int     y;
    BOOL    fShow;
    BOOL    fFirstTime;
    HBITMAP hbm;
    HANDLE  h;
    HWND    hwnd;
    HDC     hdc;
    HFONT   hfont;

    fShow = TRUE;
    fFirstTime = FALSE;

    h = LoadImage(g_hinst, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    if (h)
        SendDlgItemMessage(hDlg, IDC_HEADER, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)h);

    g_hbrInfo = CreateSolidBrush(GetSysColor(COLOR_INFOBK));

    hwnd = GetDlgItem(hDlg, IDC_PICTURE);
    if (hwnd) {
        fFirstTime = TRUE;

        /* Keep the picture on even coordinates. */
        GetWindowRect(hwnd, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        if (rc.left & 1) {
            rc.left--;
            rc.right--;
        }
        if (rc.top & 1) {
            rc.top--;
            rc.bottom--;
        }
        SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        /* Fill the picture's background with the tooltip colour. */
        hbm = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_PICTURE));
        if (hbm) {
            hdc = CreateCompatibleDC(NULL);
            hbmOld = SelectObject(hdc, hbm);
            hbrOld = SelectObject(hdc, g_hbrInfo);
            GetObject(hbm, sizeof(bm), &bm);
            y = bm.bmHeight - 1;
            ExtFloodFill(hdc, 0, y, GetPixel(hdc, 0, y), FLOODFILLSURFACE);
            SelectObject(hdc, hbrOld);
            SelectObject(hdc, hbmOld);
            DeleteDC(hdc);
            SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
        }
    }

    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        ncm.lfMenuFont.lfWeight = FW_BOLD;
        hfont = CreateFontIndirect(&ncm.lfMenuFont);
        if (hfont)
            SendDlgItemMessage(hDlg, IDC_DIDYOUKNOW, WM_SETFONT, (WPARAM)hfont, 0);
    }

    LoadString(g_hinst, IDS_TOURPATH, szTourFile, sizeof(szTourFile));
    if (!RunFromSetupSource(hDlg, c_szTourPath, szTourFile, FALSE))
        RemoveButtonIf(hDlg, c_szTourKey, c_szInstalled, FALSE, IDC_TOUR);
    RemoveButtonIf(hDlg, c_szCurrentVersion, c_szRegDone, TRUE, IDC_REGISTER);
    RemoveButtonIf(hDlg, c_szSetupKey, c_szExpostrt, FALSE, IDC_CATALOG);

    if (!fFirstTime)
        fShow = ShowNextTip(hDlg);
    if (!fFirstTime && (fShow || g_fShowTips))
        SendDlgItemMessage(hDlg, IDC_SHOWNEXT, BM_SETCHECK, 1, 0);
}

void WelcomeCommand(HWND hDlg, int id)
{
    char  szPath[340];
    BOOL  fOK;

    fOK = FALSE;
    switch (id) {
    case IDOK:
    {
        HWND  hwnd;
        DWORD fShow;
        HKEY  hkey;

        hwnd = GetDlgItem(hDlg, IDC_SHOWNEXT);
        if (hwnd) {
            fShow = SendMessage(hwnd, BM_GETCHECK, 0, 0) != 0;
            if (RegCreateKey(HKEY_CURRENT_USER, c_szTipsKey, &hkey) == ERROR_SUCCESS) {
                RegSetValueEx(hkey, c_szShow, 0, REG_BINARY, (LPBYTE)&fShow, sizeof(fShow));
                RegCloseKey(hkey);
            }
        }
    }
        /* fall through */
    case IDCANCEL:
        EndDialog(hDlg, id);
        break;

    case IDC_TOUR:
        SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        LoadString(g_hinst, IDS_TOURPATH, szPath, sizeof(szPath));
        if (RunFromSetupSource(hDlg, c_szTourPath, szPath, TRUE))
            break;
        GetWindowsDirectory(szPath, sizeof(szPath));
        lstrcat(szPath, "\\tour.exe");
        if (WinExec(szPath, SW_SHOWNORMAL) <= 31)
            ShellMessageBoxA(g_hinst, hDlg, MAKEINTRESOURCE(IDS_NOTOUR),
                             MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION);
        break;

    case IDC_REGISTER:
    {
        HKEY  hkey;
        DWORD cb;
        DWORD dwType;

        SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegWizKey, &hkey) != ERROR_SUCCESS)
            break;
        cb = sizeof(szPath);
        if (RegQueryValueEx(hkey, c_szLocation, NULL, &dwType, szPath, &cb) == ERROR_SUCCESS) {
            PathAppend(szPath, "regwiz.exe");
            lstrcat(szPath, " -i " REGSTR_PATH_SETUP);
            fOK = WinExec(szPath, SW_SHOWNORMAL) > 31;
        }
        if (!fOK) {
            GetWindowsDirectory(szPath, sizeof(szPath));
            lstrcat(szPath, "\\msn\\olregapp.exe");
            fOK = WinExec(szPath, SW_SHOWNORMAL) > 31;
        }
        if (!fOK)
            ShellMessageBoxA(g_hinst, hDlg, MAKEINTRESOURCE(IDS_NOREGWIZ),
                             MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION);
        break;
    }

    case IDC_CATALOG:
        SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        GetWindowsDirectory(szPath, sizeof(szPath));
        lstrcat(szPath, "\\expostrt.exe");
        if (WinExec(szPath, SW_SHOWNORMAL) <= 31)
            ShellMessageBoxA(g_hinst, hDlg, MAKEINTRESOURCE(IDS_NOCATALOG),
                             MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION);
        break;

    case IDC_WHATSNEW:
        SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        WinHelp(hDlg, "31users.hlp>big", HELP_CONTEXT, 30000);
        break;

    case IDC_NEXTTIP:
        SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ShowNextTip(hDlg);
        break;
    }
}

BOOL CALLBACK WelcomeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int id;

    switch (msg) {
    case WM_INITDIALOG:
        InitWelcomeDialog(hDlg);
        break;

    case WM_COMMAND:
        WelcomeCommand(hDlg, wParam);
        break;

    case WM_CTLCOLORSTATIC:
        id = GetWindowLong((HWND)lParam, GWL_ID);
        if (id == IDC_HEADER || id == IDC_SHOWNEXT)
            return FALSE;
        SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)g_hbrInfo);
        if (id == IDC_DIDYOUKNOW || id == IDC_TIPTEXT)
            SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, GetSysColor(COLOR_INFOTEXT));
        return (BOOL)g_hbrInfo;

    case WM_DESTROY:
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

/* "welcome -x[value:]file": run a program from the setup source. */
void RunFromCD(LPSTR pszArgs)
{
    LPSTR pszFile;
    LPSTR pszValue = NULL;

    while (*pszArgs == ' ')
        pszArgs++;
    for (pszFile = pszArgs; *pszFile && *pszFile != ':'; pszFile = CharNext(pszFile))
        ;
    if (*pszFile) {
        pszValue = pszArgs;
        *pszFile++ = 0;
    } else {
        pszFile = pszArgs;
    }

    if (!RunFromSetupSource(NULL, pszValue, pszFile, TRUE))
        ShellMessageBoxA(g_hinst, NULL, MAKEINTRESOURCE(IDS_RUNFAILED), MAKEINTRESOURCE(IDS_TITLE),
                         MB_OK | MB_ICONEXCLAMATION, PathFindFileName(pszFile));
}

void DoWelcome(LPSTR pszCmdLine)
{
    if ((pszCmdLine[0] == '-' || pszCmdLine[0] == '/') &&
        (pszCmdLine[1] == 'x' || pszCmdLine[1] == 'X')) {
        RunFromCD(pszCmdLine + 2);
        return;
    }

    if (!lstrcmpi(pszCmdLine, "-t") || !lstrcmpi(pszCmdLine, "/t"))
        g_fShowTips = TRUE;

    if (!lstrcmpi(pszCmdLine, "-f") || !lstrcmpi(pszCmdLine, "/f"))
        DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_FIRSTTIME), NULL, WelcomeDlgProc, TRUE);
    else
        DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_WELCOME), NULL, WelcomeDlgProc, FALSE);

    if (g_hbrInfo)
        DeleteObject(g_hbrInfo);
}

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hinst = hInstance;
    if (InitApplication(hInstance))
        DoWelcome(lpCmdLine);
    return TRUE;
}

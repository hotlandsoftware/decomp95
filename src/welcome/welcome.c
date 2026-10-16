/*
 * welcome.c - the Windows 95 Welcome screen (WELCOME.EXE, 4.00.950)
 *
 * Shows a "Did you know..." tip from the registry and buttons for the
 * Windows Tour, What's New, Online Registration and the Product Catalog.
 * "welcome -f" shows the first-boot version of the dialog, "welcome -t"
 * forces the "show next time" check box on, and "welcome -x[value:]file"
 * runs a program from the Windows 95 CD-ROM.
 */

#include "welcome.h"
#define NPPAD 0
#include "nppad.h"

HBRUSH    g_hbrInfo = NULL;
BOOL      g_fShowTips = FALSE;
HINSTANCE g_hinst;

/* Exchange the positions of two controls. */
void SwapWindows(HWND hwnd1, HWND hwnd2)
{
    HWND hwndParent = GetParent(hwnd1);
    RECT rc[2];

    GetWindowRect(hwnd1, &rc[0]);
    GetWindowRect(hwnd2, &rc[1]);
    MapWindowPoints(NULL, hwndParent, (LPPOINT)rc, 4);
    SetWindowPos(hwnd1, NULL, rc[1].left, rc[1].top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hwnd2, NULL, rc[0].left, rc[0].top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

BOOL InitApplication(HINSTANCE hInstance)
{
    return TRUE;
}

/* Show the next tip; returns the "show welcome next time" setting. */
BOOL ShowNextTip(HWND hDlg)
{
    BYTE  szTip[256];
    char  szTipName[8];
    HKEY  hkeyTips;
    DWORD dwType;
    BOOL  fShow;
    HKEY  hkey;
    DWORD cb;
    short iTip;

    fShow = TRUE;
    if (RegCreateKey(HKEY_CURRENT_USER, c_szTipsKey, &hkey) == ERROR_SUCCESS) {
        cb = sizeof(iTip);
        if (RegQueryValueEx(hkey, c_szNext, NULL, &dwType, (LPBYTE)&iTip, &cb) != ERROR_SUCCESS)
            iTip = 0;
        wsprintf(szTipName, "%d", iTip);
        if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szTipsKey, &hkeyTips) == ERROR_SUCCESS) {
            cb = sizeof(szTip);
            if (RegQueryValueEx(hkeyTips, szTipName, NULL, &dwType, szTip, &cb) == ERROR_SUCCESS) {
                SetDlgItemText(hDlg, IDC_TIPTEXT, szTip);
                iTip++;
            } else {
                iTip = 0;
            }
        }
        RegSetValueEx(hkey, c_szNext, 0, REG_BINARY, (LPBYTE)&iTip, sizeof(iTip));
        cb = sizeof(fShow);
        if (RegQueryValueEx(hkey, c_szShow, NULL, &dwType, (LPBYTE)&fShow, &cb) != ERROR_SUCCESS)
            fShow = TRUE;
        RegCloseKey(hkey);
    }
    return fShow;
}

/*
 * Find a program that Setup left on the source media.  pszValue names a
 * value under the Setup key holding its full path; failing that, if Windows
 * was installed from CD-ROM, look for pszFile on the CD-ROM drives (asking
 * for the disc if necessary).  With fExec the program is run.
 */
BOOL RunFromSetupSource(HWND hwnd, LPCSTR pszValue, LPCSTR pszFile, BOOL fExec)
{
    char  szPath[MAX_PATH];
    DWORD dwType;
    DWORD dwSourceType;
    HKEY  hkey;
    DWORD cb;
    BOOL  fFound;
    DWORD dwRoot;
    DWORD dwDrives;
    DWORD dwMask;
    int   iDrive;
    LONG  lRet;

    fFound = FALSE;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szSetupKey, &hkey) != ERROR_SUCCESS)
        return FALSE;

    if (pszValue) {
        cb = sizeof(szPath);
        if (RegQueryValueEx(hkey, pszValue, NULL, &dwType, szPath, &cb) == ERROR_SUCCESS &&
            lstrlen(szPath) > 0) {
            if (PathIsUNC(szPath)) {
                fFound = TRUE;
            } else {
                PathBuildRoot((LPSTR)&dwRoot, DRIVEID(szPath));
                if (GetDriveType((LPSTR)&dwRoot) != DRIVE_FIXED || PathFileExists(szPath))
                    fFound = TRUE;
            }
            if (fFound)
                RegCloseKey(hkey);
        }
    }

    if (!fFound) {
        cb = sizeof(dwSourceType);
        lRet = RegQueryValueEx(hkey, c_szSourcePathType, NULL, &dwType, (LPBYTE)&dwSourceType, &cb);
        RegCloseKey(hkey);
        if (lRet != ERROR_SUCCESS || cb != sizeof(dwSourceType) || dwSourceType != 5)
            return FALSE;
        if (!fExec)
            return TRUE;

        dwDrives = GetLogicalDrives();
        while (!fFound && dwDrives) {
            for (dwMask = 1, iDrive = 0; iDrive < 26; iDrive++, dwMask <<= 1) {
                if (dwDrives & dwMask) {
                    PathBuildRoot(szPath, iDrive);
                    if (GetDriveType(szPath) == DRIVE_CDROM) {
                        PathAppend(szPath, pszFile);
                        if (PathFileExists(szPath)) {
                            fFound = TRUE;
                            break;
                        }
                    } else {
                        dwDrives ^= dwMask;
                    }
                }
            }
            if (!fFound) {
                if (!dwDrives) {
                    ShellMessageBoxA(g_hinst, hwnd, MAKEINTRESOURCE(IDS_NOCDROM),
                                     MAKEINTRESOURCE(IDS_TITLE), MB_OK | MB_ICONEXCLAMATION);
                    return FALSE;
                }
                if (ShellMessageBoxA(g_hinst, hwnd, MAKEINTRESOURCE(IDS_INSERTCD),
                                     MAKEINTRESOURCE(IDS_TITLE),
                                     MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
                    return FALSE;
            }
        }
    }

    if (!fExec)
        return TRUE;
    return WinExec(szPath, SW_SHOWNORMAL) > 31;
}


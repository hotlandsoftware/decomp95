/*
 * welcome.h - shared declarations for the Windows 95 Welcome screen
 */

#include <windows.h>

#define REGSTR_PATH_SETUP       "Software\\Microsoft\\Windows\\CurrentVersion"
#define REGSTR_PATH_EXPLORER    REGSTR_PATH_SETUP "\\Explorer"

/* Undocumented SHELL32 exports (by ordinal). */
__declspec(dllimport) LPSTR WINAPI PathBuildRoot(LPSTR szRoot, int iDrive);      /* 30 */
__declspec(dllimport) LPSTR WINAPI PathFindFileName(LPCSTR pszPath);            /* 34 */
__declspec(dllimport) BOOL  WINAPI PathAppend(LPSTR pszPath, LPCSTR pszMore);    /* 36 */
__declspec(dllimport) BOOL  WINAPI PathIsUNC(LPCSTR pszPath);                   /* 39 */
__declspec(dllimport) BOOL  WINAPI PathFileExists(LPCSTR pszPath);              /* 45 */
int _cdecl ShellMessageBoxA(HINSTANCE hAppInst, HWND hWnd, LPCSTR lpcText,
                            LPCSTR lpcTitle, UINT fuStyle, ...);               /* 183 */

#define DRIVEID(path)   ((path[0] - 'A') & 31)

/* Dialogs */
#define DLG_FIRSTTIME   100
#define DLG_WELCOME     101

/* Controls */
#define IDC_TOUR        0x4201
#define IDC_WHATSNEW    0x4202
#define IDC_REGISTER    0x4203
#define IDC_CATALOG     0x4204
#define IDC_NEXTTIP     0x4205
#define IDC_SHOWNEXT    0x4207
#define IDC_TIPICON     0x4208
#define IDC_TIPTEXT     0x4209
#define IDC_HEADER      0x420a
#define IDC_TIPFRAME    0x420c
#define IDC_PICTURE     0x420d
#define IDC_DIDYOUKNOW  0x420e

/* Bitmaps */
#define IDB_HEADER      0x4210
#define IDB_PICTURE     0x4211

/* Strings */
#define IDS_NOTOUR      0x101
#define IDS_NOREGWIZ    0x102
#define IDS_NOCATALOG   0x103
#define IDS_NOCDROM     0x104
#define IDS_INSERTCD    0x105
#define IDS_TOURPATH    0x106
#define IDS_RUNFAILED   0x107
#define IDS_TITLE       0x1000

/* Read-only strings (in the code section). */
extern char c_szTipsKey[];
extern char c_szTourKey[];
extern char c_szCurrentVersion[];
extern char c_szRegDone[];
extern char c_szSetupKey[];
extern char c_szRegWizKey[];
extern char c_szNext[];
extern char c_szShow[];
extern char c_szInstalled[];
extern char c_szExpostrt[];
extern char c_szLocation[];
extern char c_szTourPath[];
extern char c_szSourcePathType[];

extern HBRUSH    g_hbrInfo;
extern BOOL      g_fShowTips;
extern HINSTANCE g_hinst;

void SwapWindows(HWND hwnd1, HWND hwnd2);
BOOL InitApplication(HINSTANCE hInstance);
BOOL ShowNextTip(HWND hDlg);
BOOL RunFromSetupSource(HWND hwnd, LPCSTR pszValue, LPCSTR pszFile, BOOL fExec);

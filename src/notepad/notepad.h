/*
 * notepad.h - shared declarations for the Windows 95 Notepad reconstruction.
 * Names are ours; only behaviour and layout come from the original binary.
 */

#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <shellapi.h>
#include <tchar.h>

/* Menu command IDs */
#define M_OPEN          10
#define M_SAVE          1
#define M_NEW           9
#define M_SAVEAS        2
#define M_PRINT         14
#define M_PAGESETUP     32
#define M_EXIT          28
#define M_UNDO          25
#define M_SELECTALL     7
#define M_DATETIME      12
#define M_WW            27
#define M_FIND          3
#define M_FINDNEXT      8
#define M_HELP          5
#define M_ABOUT         11

/* Resource string IDs */
#define IDS_DISKERROR    1
#define IDS_FNF          3
#define IDS_SCBC         5
#define IDS_SAVECHANGES 10
#define IDS_UNTITLED    11
#define IDS_NOTEPAD     12
#define IDS_CFS         16
#define IDS_ERRSPACE    17
#define IDS_FTL         18
#define IDS_NN          19
#define IDS_PASTEERR    20
#define IDS_WRITEERR    21
#define IDS_INVALIDCP   22
#define IDS_CANTPRINT   25
#define IDS_NVF         26
#define IDS_NVF2        27
#define IDS_NEDSTP      28
#define IDS_NEMTP       29
#define IDS_CREATEERR   30
#define IDS_NOWW        31
#define IDS_MERGE1      32
#define IDS_TXT         33
#define IDS_HELPFILE    34
#define IDS_HEADER      36
#define IDS_FOOTER      37
#define IDS_CANTOPENPRN 43
#define IDS_TEXTFILES   44
#define IDS_ALLFILES    45
#define IDS_OPENDLGOPEN 48
#define IDS_LOADDRVFAIL 50
#define IDS_ACCESSDENY  51
#define IDS_USEWORDPAD  52
#define IDS_WORDPADFAIL 53

#define PT_LEN          40
#define CCHFILTERMAX    64
#define CCHKEYMAX       32

/* .data (npmain.c) */
extern HWND   hwndNP;
extern HWND   hwndEdit;
extern HWND   hDlgFind;
extern BOOL   fUntitled;
extern WORD   iEditState;
extern BOOL   fRunBySetup;
extern BOOL   fWrap;
extern int    iUnused1;
extern TCHAR  szNotepad[];
extern BOOL   fInSaveAsDlg;
extern DWORD  dwSelStart;
extern DWORD  dwSelEnd;
extern BOOL   fIgnoreErrSpace;
extern BOOL   fInCheckSave;

extern TCHAR *szDiskError, *szFNF, *szSCBC, *szSaveChanges, *szUntitled,
             *szCFS, *szErrSpace, *szNpTitle, *szFTL, *szNN, *szPasteErr,
             *szWriteErr, *szInvalidCP, *szCantPrint, *szNVF, *szNVF2,
             *szNEDSTP, *szNEMTP, *szCreateErr, *szNoWW, *szMerge, *szTXT,
             *szHelpFile, *szCantOpenPrn, *szTextFiles, *szAllFiles,
             *szOpenDlgOpen, *szLoadDrvFail, *szACCESSDENY, *szUseWordpad,
             *szWordpadFail;

/* .bss */
extern TCHAR  chPageText[2][PT_LEN];
extern HINSTANCE hInstanceNP;
extern TCHAR  szFilterSpec[CCHFILTERMAX];
extern TCHAR  szSearch[CCHKEYMAX];
extern HMENU  hSysMenuSetup;
extern FINDREPLACE FR;
extern HACCEL hAccel;
extern UINT   wFRMsg;
extern HCURSOR hWaitCursor;
extern TCHAR  szCustFilterSpec[CCHFILTERMAX];
extern HANDLE fp;
extern HANDLE hEdit;
extern OPENFILENAME OFN;
extern TCHAR  szFileName[MAX_PATH];
extern PAGESETUPDLG g_PageSetupDlg;
extern HFONT  hFont;
extern SHORT  chMerge;
extern TCHAR  chPageTextSave[2][PT_LEN];
extern BOOL   fCase, fReverse;
extern TCHAR  chPeriod;
extern BOOL   fPrintAborted;
extern int    dyHeader;
extern int    cxPrintChar, cyPrintPage, iTabStops, iPageNum, cyPrintLine;
extern int    xCharsPerLine;
extern TCHAR *pHeaderBuf;
extern HWND   hAbortDlgWnd;
extern HMENU  hAbortSysMenu;
extern RECT   rtPrintMargin;
extern HANDLE hHeader;

/* npmain.c */
int  ModuleEntry(void);
int  WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WINAPI NPWndProc(HWND, UINT, WPARAM, LPARAM);
void FreePageSetupHandles(void);
void InitPageSetupDefaults(void);
void WINAPI NpResizeEdit(int cx, int cy);
BOOL NPCommand(HWND, WPARAM, LPARAM);
void OpenDroppedFile(void);
void WINAPI NpDropFiles(HDROP, HWND);
BOOL CheckSave(BOOL fSysModal);
void SetTitle(TCHAR *);
TCHAR *PFileInPath(TCHAR *);
void NpUpdateMenu(HWND);
void NpWinIniChange(void);
BOOL MergeStrings(TCHAR *, TCHAR *, TCHAR *);
int  AlertBox(HWND, TCHAR *, TCHAR *, TCHAR *, WORD);
void GetFileName(TCHAR *, TCHAR *);
void FixFilterString(TCHAR *);
BOOL NPInitStrings(HINSTANCE);

/* npinit.c */
BOOL SetupCheck(TCHAR *);
BOOL PrintFileCheck(TCHAR *, int);
BOOL NPInit(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL NPRegister(HINSTANCE);
void InitDefaultFont(void);
BOOL SaveFile(HWND, TCHAR *, BOOL);
int  TryWordPad(HWND, TCHAR *, BOOL);
int  LoadFile(TCHAR *, BOOL);
void New(BOOL);
void AddExt(TCHAR *);

/* npsearch.c */
void Search(TCHAR *);
BOOL NpReCreate(BOOL);

/* strings.c (shell string library) */
LPSTR WINAPI StrStrA(LPCSTR, LPCSTR);
LPSTR WINAPI StrStrIA(LPCSTR, LPCSTR);
LPSTR WINAPI StrRStrA(LPCSTR, LPCSTR, LPCSTR);
LPSTR WINAPI StrRStrIA(LPCSTR, LPCSTR, LPCSTR);

/* npprint.c */
BOOL CALLBACK AbortProc(HDC, int);
BOOL CALLBACK AbortDlgProc(HWND, UINT, WPARAM, LPARAM);
void PrintHeaderFooter(HDC, short);
int  NpPrint(void);
int  ExpandTabs(TCHAR *, int, TCHAR *, int);
void DestroyAbortWnd(void);
UINT CALLBACK PageSetupHookProc(HWND, UINT, WPARAM, LPARAM);
int  TranslateString(TCHAR *);
HDC  GetPrinterDC(void);
void InsertDateTime(BOOL);

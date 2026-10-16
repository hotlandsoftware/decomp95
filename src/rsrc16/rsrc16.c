/*
 * rsrc16.c - Resource Meter 16-bit thunk DLL (Windows 95, 4.00.950)
 *
 * RSRCMTR.EXE is a 32-bit program; it reaches USER's 16-bit
 * GetFreeSystemResources through a flat thunk (RSRC32.DLL -> RSRC16.DLL).
 */

#include <windows.h>

BOOL FAR PASCAL thk_ThunkConnect16(LPSTR pszDll16, LPSTR pszDll32,
                                   WORD hInst, DWORD dwReason);

int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD wHeapSize,
                       LPSTR lpszCmdLine)
{
    if (wHeapSize > 0)
        UnlockData(0);
    return 1;
}

int FAR PASCAL WEP(int nParameter)
{
    return 1;
}

UINT FAR PASCAL __export MyGetFreeSystemResources16(UINT fuSysResource)
{
    return GetFreeSystemResources(fuSysResource);
}

BOOL FAR PASCAL __export DllEntryPoint(DWORD dwReason, WORD hInst, WORD wDS,
                                       WORD wHeapSize, DWORD dwReserved1,
                                       WORD wReserved2)
{
    if (!thk_ThunkConnect16("rsrc16.DLL", "rsrc32.DLL", hInst, dwReason))
        return FALSE;
    return TRUE;
}

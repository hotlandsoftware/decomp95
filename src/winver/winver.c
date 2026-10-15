/*
 * WINVER.EXE - Windows Version program (Windows 95, 4.00.950)
 *
 * Shows a small dialog with the product name ("Windows 95", string 310)
 * and the copyright line.
 */

#include <windows.h>

#define IDS_PRODUCT     310         /* "Windows 95" */
#define IDD_WINVER      100
#define IDC_PRODUCT     100

BOOL FAR PASCAL AppDlgProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_PRODUCT, (LPSTR)lParam);
        return TRUE;

    case WM_COMMAND:
        EndDialog(hDlg, TRUE);
        break;
    }

    return FALSE;
}

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
    char szProduct[128];

    if (hPrevInstance)
        return FALSE;

    LoadString(hInstance, IDS_PRODUCT, szProduct, sizeof(szProduct));
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_WINVER), NULL,
                   MakeProcInstance((FARPROC)AppDlgProc, hInstance),
                   (LONG)(LPSTR)szProduct);
    return TRUE;
}

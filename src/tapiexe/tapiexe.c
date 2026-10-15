/*
 * TAPIEXE.EXE - Windows Telephony DLL hidden app (Windows 95, v1.0.501)
 *
 * Hidden helper task started by TAPI.DLL.  It gives TAPI a task context and
 * a window to post to, and forwards a few notifications back into TAPI.DLL.
 *
 * Build: MSVC 1.52 (C 8.00c), small model, see Makefile for flags.
 */

#include <windows.h>

/* TAPI.DLL private exports (imported by name); spellings from TAPI.SYM (build 490). */
LONG FAR PASCAL TAPI_INITIALIZE_REAL(void);
void FAR PASCAL TAPIINITIALIZEERROR(void);
void FAR PASCAL destroyAbnormalApp(WPARAM wParam);
void FAR PASCAL processIniFileChange(void);
BOOL FAR PASCAL RegisterTAPIAppThread(HINSTANCE hInst, HWND hwnd);
void FAR PASCAL DeRegisterTAPIAppThread(void);

/* KERNEL.491 (Windows 95 only). */
DWORD FAR PASCAL RegisterServiceProcess(DWORD dwProcessId, DWORD dwType);

#define TM_DESTROYABNORMALAPP   (WM_USER + 0x63)
#define TM_SHUTDOWN             (WM_USER + 0x65)

/* Defined in tapistr.c; lives in the code segment, addressed through CS. */
extern char __based(__segname("_CODE")) szClassName[];

/* Own code-class segment; packed into the code segment after the startup code. */
/* Defined in telestr.asm (paragraph-aligned code-class segment after crt0). */
extern char __far szTelephony[];

HINSTANCE ghInst;

LRESULT CALLBACK TapiWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
    case TM_SHUTDOWN:
        DeRegisterTAPIAppThread();
        DestroyWindow(hwnd);
        UnregisterClass(szClassName, ghInst);
        break;

    case WM_WININICHANGE:
        if (lParam == 0L || lstrcmpi((LPCSTR)lParam, szTelephony) == 0)
            processIniFileChange();
        break;

    case TM_DESTROYABNORMALAPP:
        destroyAbnormalApp(wParam);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0L;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hwnd;
    int nQueue;
    int nRet = 0;

    if (hPrevInstance)
        return 0;

    nQueue = 120;
    while (!SetMessageQueue(nQueue))
    {
        if (--nQueue < 8)
        {
            TAPIINITIALIZEERROR();
            goto Exit;
        }
    }

    _fmemset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc   = TapiWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = szClassName;

    if (!RegisterClass(&wc) && !hPrevInstance)
    {
        TAPIINITIALIZEERROR();
        goto Exit;
    }

    hwnd = CreateWindow(szClassName, NULL, WS_MINIMIZE,
                        0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL && !UnregisterClass(szClassName, hInstance))
    {
        TAPIINITIALIZEERROR();
        goto Exit;
    }

    ghInst = hInstance;

    if (TAPI_INITIALIZE_REAL() != 0L)
        goto Exit;

    if (!RegisterTAPIAppThread(ghInst, hwnd))
        goto Exit;

    RegisterServiceProcess(0L, 1L);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    nRet = msg.wParam;

Exit:
    return nRet;
}

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <objbase.h>
#include "MainWnd.hpp"
#include "SteamCAD.rh"

//HANDLE g_hConsole;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR szCmdLine, int iCmdShow)
{
    CoInitialize(NULL);
/*AllocConsole();
g_hConsole = CreateConsoleScreenBuffer(GENERIC_WRITE, FILE_SHARE_READ,
NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
SetConsoleMode(g_hConsole, ENABLE_PROCESSED_OUTPUT);
SetConsoleActiveScreenBuffer(g_hConsole);*/

    INITCOMMONCONTROLSEX ictr;
    ictr.dwSize = sizeof(INITCOMMONCONTROLSEX);
    //ictr.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES |
    //    ICC_TAB_CLASSES | ICC_PAGESCROLLER_CLASS;
    ictr.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&ictr);

    CMainWnd *mw = new CMainWnd(hInstance);
    HWND wnd = mw->DisplayWindow();

    if(!wnd)
    {
        DWORD dwErr = GetLastError();
        wchar_t buf[64];
        swprintf(buf, L"Error: %.8X", dwErr);
        //FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dwErr, 0, buf, 64, 0);
        MessageBox(0, buf, L"Error", MB_OK);
        delete mw;
        ExitProcess(0);
        return(0);
    }

    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(1));
    ShowWindow(wnd, SW_SHOW);

    MSG msg;
    BOOL res, finish = FALSE;

    while(!finish)
    {
        res = GetMessage(&msg, 0, 0, 0);
        if (!TranslateAccelerator(wnd, hAccel, &msg))
        {
            switch (res)
            {
            case -1:
                MessageBox(wnd, L"Error?", L"Debug", MB_OK);
            case 0:
                finish = true;
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    delete mw;

//FreeConsole();
    CoUninitialize();

    ExitProcess(0);
    return(0);
}

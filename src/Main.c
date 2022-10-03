#define UNICODE ///< UTF-8 support
#include <windows.h>
#include "Resource.h"
#include "Procedures.h"

/// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE _$, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR *const wndclass_name = TEXT(STR_CODENAME);
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // Set Window Attributes & register Window class with them
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_ACTIVECAPTION + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wndclass_name;
    if (!RegisterClass(&wc))
        return 0;

    hwnd = CreateWindow(
        wndclass_name,
        TEXT(STR_APPNAME),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);
    if (!hwnd)
        return 0;

    // Message loop, repeats until app terminates
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

/// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define Call_WMHandler(handler_name) handler_name(hwnd, uMsg, wParam, lParam)
    switch (uMsg)
    {
    case WM_CREATE: // when apps started
        Call_WMHandler(wmCreate);
        return 0;

    case AWM_TRAYICONCLICKED: // when icon is clicked
        Call_WMHandler(wmTrayIconClicked);
        return 0;

    case WM_COMMAND: // when menu item is clicked
        Call_WMHandler(wmMenuItemClicked);
        return 0;

    case AWM_LLKEYHOOKED: // when toggle key is pressed
        Call_WMHandler(wmLLKeyHooked);
        return 0;

    case WM_SETTINGCHANGE: // when theme mode has changed
        Call_WMHandler(wmThemeChanged);
        return 0;

    case WM_QUERYENDSESSION: // when leaves the session
        Call_WMHandler(wmOnExit);
        return TRUE;

    case WM_CLOSE: // when closes window
        Call_WMHandler(wmOnExit);
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY: // when destroy window
        Call_WMHandler(wmDestroy);
        return 0;
    } // switch (uMsg)

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#undef Call_WMHandler
}

#define UNICODE                     // UTF-8 Standard
#define NTDDI_VERSION NTDDI_VISTA   // Targeted OS is Vista or later
#include <windows.h>                // Where it all begins...
#include <windowsx.h>               // GET_X(,Y)_LPARAM
#include "Resource.h"

// Num of the icons of keys to observe
#define NOTIFYICON_TOTAL    4

// Toggle key active state
#define KS_TOGGLEACTIVE     1

// Reg key and value name containing current UI theme (Dark or Light)
#define REGKEYPATH_THEMEMODEsz  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define REG_VALUENAME           "AppsUseLightTheme"

/// Application-defined Window Messages
/// The values over WM_APP can be used.
typedef enum tagAPPDEFINEDWINDOWMESSAGE {
    WM_NOTIFYICONCLICKED = ((WM_APP) + 100),
    WM_UITHEMECHANGED,
    WM_LLKEYHOOKED,
    WM_REDRAWNOTIFYICON
} ADWM;

// ID of the items in the tray
enum tagNOTIFYICONID {
    NIID_NUMLOCK,
    NIID_CAPSLOCK,
    NIID_SCROLLLOCK,
    NIID_INSERT
};

// Struct: Store the data of each key
typedef struct tagTHEKEYINFO {
    INT nIrid;
    INT nMiid;
    INT nNiid;
    INT nVkey;
    INT nFlagMask;
} THEKEYINFO;

// <!-- Global Variables

HANDLE hConsole;    // For console debugging
HWND hWindow;       // For KeyHookProc

// --->

// <!--- Prototype declaration ---

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);
static void DetectUITheme(DWORD *pdwBufSize, DWORD *pfLightTheme);

// --->

/// Entry point
int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE null, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR szAppName[] = TEXT(RES_CODENAMEsz);
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // Set Window Attributes
    wc.style        = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc  = WndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.hInstance    = hInstance;
    wc.hIcon        = LoadIcon(hInstance, MAKEINTRESOURCE(IRID_APPICON));
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground= (HBRUSH) (COLOR_ACTIVECAPTION + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName= szAppName;

    // Register Window class
    if (!RegisterClass(&wc)) return 0;

    // Make a window
    hWindow = hwnd = CreateWindow(
        szAppName, TEXT(RES_APPNAMEsz),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd) return 0;

#ifdef _ENABLE_APP_DEBUG_
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
#endif

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

/// Window Procedure Func.
LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
){
    static const THEKEYINFO tki[NOTIFYICON_TOTAL] = {
        {IRID_DNUMD, MIID_NUMLOCK, NIID_NUMLOCK, VK_NUMLOCK, 0},
        {IRID_DCAPSD, MIID_CAPSLOCK, NIID_CAPSLOCK, VK_CAPITAL, 0},
        {IRID_DSCROLLD, MIID_SCROLLLOCK, NIID_SCROLLLOCK, VK_SCROLL, 0},
        {IRID_DINSERTD, MIID_INSERT, NIID_INSERT, VK_INSERT, 0}
    };
    static HMENU hMenu, hSubMenu;                   // Icon Context Menu
    static UINT uIconCounter;                       // Number of the key icons currently shown
    static NOTIFYICONDATA nid[NOTIFYICON_TOTAL];    // Notification Icon data
    static DWORD fLightThemeUsed;                   // Flag being set if Light Theme used
    static HHOOK hKeyHook;                          // Handle to keyboard hook
    TCHAR szTemp[256] = {0};                        // Multipurppose string buffer
    MENUITEMINFO mii;                               // Menu item to check its state
    DWORD dwRegValueSize;                           // Reg value size to obtain

    switch (uMsg)
    {
    //////////////////////////////////////////
    // ----- THE APPLICATION LAUNCHED ----- //
    //////////////////////////////////////////
    case WM_CREATE:
        // Get Handle to Console with Standard output
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        // if (hConsole != INVALID_HANDLE_VALUE); // Error handle. skipped
        // WriteConsole(hConsole, L"HELLO WORLD\n", numchars, &len, NULL);
        // ^ (Console handle, String variable, NUM OF its chars, Ptr to num of actually written chars, RESERVED)
        hMenu = LoadMenu(NULL, MAKEINTRESOURCE(MID_NIMENU_DUMMYPARENT));
        // ^ (Inst. containing Resource, ResName or MKINTRES(num))
        hSubMenu = GetSubMenu(hMenu, 0);
        // ^ (Parent, Relative Position in parent of sub)
        uIconCounter = 4;
        DetectUITheme(&dwRegValueSize, &fLightThemeUsed);

        for (INT i=0; i < 4; i++) {
            nid[i].cbSize = sizeof(NOTIFYICONDATA);
            nid[i].hWnd = hwnd;
            nid[i].uCallbackMessage = WM_NOTIFYICONCLICKED;     // App-defined WMes from icons
            nid[i].uFlags = NIF_ICON | NIF_MESSAGE;// | NIF_TIP | NIF_SHOWTIP | NIF_INFO;
            nid[i].uVersion = NOTIFYICON_VERSION_4;             // Easy to get clicked position, or use GetRect()
            nid[i].dwInfoFlags = NIIF_INFO;                     // Balloon Icon & Sound attr.
            INT rid = tki[i].nIrid + (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE) * IRID_ACTIVEOFFSET
                        + fLightThemeUsed * IRID_LIGHTOFFSET;   // Icon resource ID
            nid[i].uID = tki[i].nNiid;                          // ID for tray items
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                            MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            // ^ LoadIcon() wouldn't work. For LoadIconMetric() including `commctrl.h` & link `comctl32` is needed
            // lstrcpy(nid[i].szTip, TEXT("Key State appears here"));
            // lstrcpy(nid[i].szInfoTitle, TEXT(RES_APPNAMEsz));
            // lstrcpy(nid[i].szInfo, TEXT("Key State appears here"));
            Shell_NotifyIcon(NIM_ADD, &nid[i]);                 // Add an item to tray
            Shell_NotifyIcon(NIM_SETVERSION, &nid[i]);          // Apply uVersion
        }

        hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
        // ^ (HookID, fnHookProc, Inst. containing HookProc or NULL for this Inst., ThreadID to be hooked or 0 for global)
        if (!hKeyHook) {
            MessageBox(NULL, L"Hook was not set properly!", TEXT(RES_APPNAMEsz), MB_ICONERROR);
            DestroyWindow(hwnd);
        }
        return 0;

    ////////////////////////////////////////
    // ----- NOTIFY ICON IS CLICKED ----- //
    ////////////////////////////////////////
    case WM_NOTIFYICONCLICKED:
        if ( LOWORD(lParam) == WM_CONTEXTMENU )
        {   // : Context menu appeared
            POINT pt = {};                  // To get Mouse position when an icon is clicked
            pt.x = GET_X_LPARAM(wParam);
            pt.y = GET_Y_LPARAM(wParam);
            // : Obtained X,Y seems INCORRECT with HiDPI enabled
            // ClientToScreen(hwnd, &pt);
            SetForegroundWindow(hwnd);          // Prevent Menu from remaining or disappearing
            TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                            pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);   // Same as above
        }
        else if ( LOWORD(lParam) == WM_LBUTTONUP )
        {   // : Icons are clicked by LEFT button to toggle keys
            INPUT inputs[4] = {};               // To send KeyInput to toggle their states
            ZeroMemory(inputs, sizeof(inputs));
            WORD niid = HIWORD(lParam);         // NIID of the clicked icon
            for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
                if (tki[i].nNiid != niid)   continue;
                inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
                inputs[0].ki.wVk = inputs[1].ki.wVk = tki[i].nVkey;
                inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
                INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE)
                            + fLightThemeUsed * IRID_LIGHTOFFSET;
                nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                                MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
                Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
                break;
            }
        }
        return 0;

    /////////////////////////////////////////
    // ----- CONTEXT MENU IS PRESSED ----- //
    /////////////////////////////////////////
    case WM_COMMAND: {
        WORD wmId = LOWORD(wParam);     // Which MenuItem is selected
        // ZeroMemory(&mii, sizeof(mii));// Not needed
        // mii.cbSize = sizeof(MENUITEMINFO);      // --> Needed before obtain MenuItem
        // mii.fMask = MIIM_STATE;     // --> Must specify the information to get first
        // GetMenuItemInfo(hSubMenu, wmId, FALSE, &mii);   // Errno 0x57 returned if hMenu is incorrect
        // mii.fState ^= MFS_CHECKED;  // --> Toggle Checked State
        // SetMenuItemInfo(hSubMenu, wmId, FALSE, &mii);   // Override the menu item
        // DWORD fDisplayFlag = ((mii.fState & MFS_CHECKED) ? NIM_ADD : NIM_DELETE);
        switch (wmId)
        {
        // case MIID_NUMLOCK:
        //     Shell_NotifyIcon(fDisplayFlag, &(nid[0]));
        //     return 0;
        // case MIID_CAPSLOCK:
        //     Shell_NotifyIcon(fDisplayFlag, &(nid[1]));
        //     return 0;
        // case MIID_SCROLLLOCK:
        //     Shell_NotifyIcon(fDisplayFlag, &(nid[2]));
        //     return 0;
        // case MIID_INSERT:
        //     Shell_NotifyIcon(fDisplayFlag, &(nid[3]));
        //     return 0;

        // case MIID_SENDNOTIFY:
        //     for (int i=0; i < NOTIFYICON_TOTAL; i++)
        //         nid[i].uFlags ^= NIF_INFO;
        //     EnableMenuItem( hSubMenu, MIID_NOTIFYSOUND,
        //                     (mii.fState & MFS_CHECKED) ? MF_DEFAULT : MF_GRAYED);
        //     return 0;
        // case MIID_NOTIFYSOUND:
        //     for (int i=0; i < NOTIFYICON_TOTAL; i++)
        //         nid[i].dwInfoFlags ^= NIIF_NOSOUND;
        //     return 0;
        // case MIID_AUTOSTART:
        //     return 0;

        case MIID_ABOUT:
            MessageBox( NULL,
                        TEXT("Version:\t" RES_APPVERsz "\n"
                            "Release:\t" RES_RELEASEDATEsz "\n"
                            "Author:\t" RES_AUTHORsz),
                        TEXT(RES_APPNAMEsz), MB_OK | MB_ICONINFORMATION);
            return 0;
        case MIID_EXIT: DestroyWindow(hwnd);
            return 0;
        }
        return 0;
    }

    // case WM_REDRAWNOTIFYICON:
    //     for (INT i=0; i < NOTIFYICON_TOTAL; i++)
    //     {
    //         if (tki[i].nVkey != (INT)wParam)   continue;
    //         INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE)
    //                     + fLightThemeUsed * IRID_LIGHTOFFSET;
    //         nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
    //                                         MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
    //         Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
    //     }
    //     return 0;

    case WM_LLKEYHOOKED:
        for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
            if (tki[i].nVkey != (INT)wParam)   continue;
            INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE)
                        + fLightThemeUsed * IRID_LIGHTOFFSET;
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                            MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
            break;
        }
        return 0;

    case WM_UITHEMECHANGED:
        for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
            INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE)
                        + fLightThemeUsed * IRID_LIGHTOFFSET;
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                            MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
        }
        return 0;

    // _ Theme color change detection
    case WM_SETTINGCHANGE:
        if (!lstrcmp((LPCWSTR) lParam, L"ImmersiveColorSet")) {
            DetectUITheme(&dwRegValueSize, &fLightThemeUsed);
            PostMessage(hwnd, WM_UITHEMECHANGED, 0, 0);
        }
        return 0;

    // _ Window close message
    case WM_CLOSE: DestroyWindow(hwnd);
        return 0;

    // _ Window destroy message
    case WM_DESTROY:
        UnhookWindowsHookEx(hKeyHook);
        DestroyMenu(hMenu);
        for (int i=0; i < NOTIFYICON_TOTAL; i++) {
            Shell_NotifyIcon(NIM_DELETE, &nid[i]);      // Remove Icons from the tray
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// _ Need to add compile option `-ladvapi32` to use Reg*()
static void DetectUITheme(DWORD *pdwBufSize, DWORD *pfLightTheme) {
    *pdwBufSize = sizeof(*pfLightTheme);
    RegGetValue(HKEY_CURRENT_USER,
                TEXT(REGKEYPATH_THEMEMODEsz), TEXT(REG_VALUENAME),
                RRF_RT_REG_DWORD, NULL, pfLightTheme, pdwBufSize);
}

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT *pKeyStruct;
    if (nCode == HC_ACTION && wParam == WM_KEYUP) {
        pKeyStruct = (KBDLLHOOKSTRUCT *)lParam;
        if (pKeyStruct->flags & LLKHF_UP) {
            PostMessage(hWindow, WM_LLKEYHOOKED, pKeyStruct->vkCode, 0);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

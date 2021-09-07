#define UNICODE         /// UTF-8 based
#define NTDDI_VERSION NTDDI_VISTA           /// For `NOTIFYICONDATA.uVersion = 4'
#define _WIN32_WINNT _WIN32_WINNT_VISTA     /// For Registry functions
// #include <PathCch.h>    /// For PathCchRemoveFileSpec (preferred but link error, retry in a while)
#include <Shlwapi.h>    /// For PathRemoveFileSpec (deprecated though)
#include <Windows.h>
#include <windowsx.h>   /// GET_X(,Y)_LPARAM
#include "Resource.h"   /// My resource definitions
#include "Main.h"       /// etc.

// #define _CONSOLE_DEBUG

// <!-- Global Variables -->

HANDLE hConsole;    // For console debugging
HWND hWindow;       // For KeyHookProc
static HMENU hMenu, hSubMenu;                   // Icon Context Menu
static TCHAR szConfPath[MAX_PATH];

// <!--- Prototype declaration --->

/// Check/Uncheck Menu Item
/// @param uMenuItemId Menu item ID
/// @param fChecked Checked state to set (TRUE=checked, FALSE=unchecked)
static void SetMenuItemCheckState(UINT uMenuItemId, BOOL fChecked);

/// Detect which UI Theme currently applied, Dark or Light
/// @deprecated This function will be replaced with the one returning boolean.
/// @param pdwBufSize
/// @param pfLightTheme
static void DetectUITheme(DWORD *pdwBufSize, DWORD *pfLightTheme);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);


/// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE null, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR szAppName[] = TEXT(RES_CODENAME_STR);
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // Set Window Attributes
    wc.style        = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc  = WndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.hInstance    = hInstance;
    wc.hIcon        = NULL;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground= (HBRUSH) (COLOR_ACTIVECAPTION + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName= szAppName;

    // Register Window class
    if (!RegisterClass(&wc)) return 0;

    // Make a window
    hWindow = hwnd = CreateWindow(
        szAppName, TEXT(RES_APPNAME_STR),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd) return 0;

#ifdef _CONSOLE_DEBUG
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
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static const THEKEYINFO tki[NOTIFYICON_TOTAL] = {
        {IRID_DNUMD, MIID_NUMLOCK, NIID_NUMLOCK, VK_NUMLOCK, 0, L"Num Lock"},
        {IRID_DCAPSD, MIID_CAPSLOCK, NIID_CAPSLOCK, VK_CAPITAL, 0, L"Caps Lock"},
        {IRID_DSCROLLD, MIID_SCROLLLOCK, NIID_SCROLLLOCK, VK_SCROLL, 0, L"Scroll Lock"},
        {IRID_DINSERTD, MIID_INSERT, NIID_INSERT, VK_INSERT, 0, L"Insert"}
    };
    static UINT uIconCounter = 4;                   // Number of the key icons currently shown
    static NOTIFYICONDATA nid[NOTIFYICON_TOTAL];    // Notification Icon data
    static DWORD fLightThemeUsed;           // Flag being set if Light Theme used
    static HHOOK hKeyHook;                          // Handle to keyboard hook
    static BOOL fNotify = FALSE;                    // Notification switch
    static BOOL fAutostart = FALSE;                 // Autostart switch
    TCHAR szTemp[MAX_PATH] = {0};                        // Multipurppose string buffer
    DWORD dwRegValueSize;                           // RegValue size to obtain
    WCHAR lpExePath[MAX_PATH];                      // Path to executable, to register Autostart

    switch (uMsg)
    {
    //////////////////////////////////////////
    // ----- THE APPLICATION LAUNCHED ----- //
    //////////////////////////////////////////
    case WM_CREATE:
        // Get Handle to Console with Standard output
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#ifdef _CONSOLE_DEBUG
        if (hConsole == INVALID_HANDLE_VALUE)
            MessageBox(NULL, L"Console handle couldn't Obtained!!", L"", MB_ICONERROR);
        GetCurrentDirectory(MAX_PATH, szTemp);
        WriteConsole(hConsole, szTemp, lstrlen(szTemp), NULL, NULL);
#endif
        /// Get absolute path to the config file, for autostart
        GetModuleFileName(NULL, lpExePath, MAX_PATH);
        // PathCchRemoveFileSpec(lpExePath, lstrlen(lpExePath));
        PathRemoveFileSpec(lpExePath);
        wsprintf(szConfPath, TEXT("%s%s"), lpExePath, TEXT(FN_CONFIGFILE));
        // MessageBox(NULL, szConfPath, TEXT("d"), MB_OK);     /// For debug purpose

        hMenu = LoadMenu(NULL, MAKEINTRESOURCE(MID_NIMENU_DUMMYPARENT));
        // ^ (Inst. containing Resource, ResName or MKINTRES(num))
        hSubMenu = GetSubMenu(hMenu, 0);
        // ^ (Parent, Relative Position in parent of sub)
        DetectUITheme(&dwRegValueSize, &fLightThemeUsed);
        for (INT i=0; i < 4; i++) {
            nid[i].cbSize = sizeof(NOTIFYICONDATA);
            nid[i].hWnd = hwnd;
            nid[i].uCallbackMessage = WM_NOTIFYICONCLICKED;     // App-defined WMes from icons
            nid[i].uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
            nid[i].uVersion = NOTIFYICON_VERSION_4;             // Easy to get clicked position, or use GetRect()
            nid[i].dwStateMask = NIS_HIDDEN;
            if ( GetPrivateProfileInt(TEXT(FN_CONFIGSECT), tki[i].szTip, 0, szConfPath) )
            {
                nid[i].dwState = NIS_HIDDEN;
                uIconCounter--;
                SetMenuItemCheckState(tki[i].nMiid, FALSE);
            }
            nid[i].dwInfoFlags = NIIF_INFO;                     // Balloon Icon & Sound attr.
            INT rid = tki[i].nIrid + (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE) * IRID_ACTIVEOFFSET
                        + fLightThemeUsed * IRID_LIGHTOFFSET;   // Icon resource ID
            nid[i].uID = tki[i].nNiid;                          // ID for tray items
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                            MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, LR_SHARED);
            // ^ LoadIcon() wouldn't work. For LoadIconMetric() including `commctrl.h` & link `comctl32` is needed
            lstrcpy(nid[i].szTip, tki[i].szTip);
            lstrcpy(nid[i].szInfoTitle, TEXT(RES_APPNAME_STR));
            Shell_NotifyIcon(NIM_ADD, &nid[i]);                 // Add an item to tray
            Shell_NotifyIcon(NIM_SETVERSION, &nid[i]);          // Apply uVersion
        }

        hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
        // ^ (HookID, fnHookProc, Inst. containing HookProc or NULL for this Inst., ThreadID to be hooked or 0 for global)
        if (!hKeyHook) {
            MessageBox(NULL, L"An error has been occurred with SetWindowsHookEx() and the program will be terminated.", TEXT(RES_APPNAME_STR), MB_ICONERROR);
            SendMessage(hwnd, WM_DESTROY, 0, 0);
        }

        /// Test whether Autostart is enabled
        dwRegValueSize = sizeof(lpExePath);
        GetModuleFileName(NULL, szTemp, sizeof(szTemp));
        RegGetValue(HKEY_CURRENT_USER, TEXT(REGSUBKEY_USERSTARTUP), TEXT(REGVALUE_LOKI), RRF_RT_REG_SZ, NULL, lpExePath, &dwRegValueSize);
        if ( !lstrcmp(lpExePath, szTemp) )
        {
            fAutostart = TRUE;
            SetMenuItemCheckState(MIID_AUTOSTART, TRUE);
        }

        /// Get "Send Notification" flag value
        fNotify = GetPrivateProfileInt(TEXT(FN_NOTIFYSECT), TEXT(FN_NTFYSECKEY), 0, szConfPath);
        if (fNotify) {
            SetMenuItemCheckState(MIID_SENDNOTIFY, TRUE);
        }
        return 0;

    ////////////////////////////////////////
    // ----- NOTIFY ICON IS CLICKED ----- //
    ////////////////////////////////////////
    case WM_NOTIFYICONCLICKED:
        if ( LOWORD(lParam) == WM_CONTEXTMENU )
        {   // : Context menu appeared
            POINT pt = {};                  // To get Mouse Click position on icons
            pt.x = GET_X_LPARAM(wParam);    // Manifest solved the HiDPI problem
            pt.y = GET_Y_LPARAM(wParam);
#ifdef _CONSOLE_DEBUG
            wsprintf(szTemp, L"x = %d, y = %d\n", pt.x, pt.y);
            WriteConsole(hConsole, szTemp, lstrlen(szTemp), NULL, NULL);
#endif
            SetForegroundWindow(hwnd);          // Prevent Menu from remaining or disappearing
            TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                            pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);   // Same as above
        }
        else if ( LOWORD(lParam) == WM_LBUTTONUP )
        {   // : Icons are Left clicked to toggle keys
            INPUT inputs[4];                    // To send KeyInput to toggle their states
            ZeroMemory(inputs, sizeof(inputs));
            WORD niid = HIWORD(lParam);         // NIID of the clicked icon
            for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
                if (tki[i].nNiid != niid)   continue;
                inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
                inputs[0].ki.wVk = inputs[1].ki.wVk = tki[i].nVkey;
                inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
                /// Changing icon state here may cause an bug...?
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
        // BOOL fSetCheck = FALSE;
        switch (wmId)
        {
        case MIID_NUMLOCK:
        case MIID_CAPSLOCK:
        case MIID_SCROLLLOCK:
        case MIID_INSERT:
            for (int i=0; i<NOTIFYICON_TOTAL; i++) {
                if (tki[i].nMiid == wmId) {
                    INT niid = tki[i].nNiid;
                    if (nid[niid].dwState & NIS_HIDDEN) {
                        uIconCounter++;
                        SetMenuItemCheckState(wmId, TRUE);
                    } else if (uIconCounter == 1) {
                        MessageBox(NULL, L"At least one icon must be kept.", TEXT(RES_APPNAME_STR), MB_ICONEXCLAMATION | MB_OK);
                        break;
                    } else {
                        uIconCounter--;
                        SetMenuItemCheckState(wmId, FALSE);
                    }
                    nid[niid].dwState ^= NIS_HIDDEN;
                    Shell_NotifyIcon(NIM_MODIFY, &(nid[niid]));
                    break;
                }
            }
            break;

        case MIID_SENDNOTIFY:
            fNotify = !fNotify;
            SetMenuItemCheckState(wmId, fNotify);
            break;

        case MIID_AUTOSTART:
            if (fAutostart) {
                RegDeleteKeyValue(HKEY_CURRENT_USER, TEXT(REGSUBKEY_USERSTARTUP), TEXT(REGVALUE_LOKI));
                SetMenuItemCheckState(wmId, FALSE);
            } else {
                dwRegValueSize = sizeof(lpExePath);
                GetModuleFileName(NULL, lpExePath, sizeof(lpExePath));
                if (RegSetKeyValue(HKEY_CURRENT_USER,
                    TEXT(REGSUBKEY_USERSTARTUP), TEXT(REGVALUE_LOKI),
                    RRF_RT_REG_SZ, lpExePath, dwRegValueSize) == ERROR_SUCCESS)
                {
                    SetMenuItemCheckState(wmId, TRUE);
                } else {
                    MessageBox(NULL, L"Autostart registration failed.", TEXT(RES_APPNAME_STR), MB_ICONERROR);
                }
            }
            fAutostart = !fAutostart;
            break;

        case MIID_ABOUT:
            MessageBox( NULL,
                        TEXT("Version:\t" RES_APPVER_STR "\n"
                            "Release:\t" RES_RELEASEDATE_STR "\n"
                            "Author:\t" RES_AUTHOR_STR),
                        TEXT(RES_APPNAME_STR), MB_OK | MB_ICONINFORMATION);
            break;
        case MIID_EXIT: SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        }
        return 0;
    }

    case WM_LLKEYHOOKED:
        for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
            if (tki[i].nVkey != (INT)wParam)   continue;
            SHORT fKeyState = GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE;
            INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * fKeyState
                        + fLightThemeUsed * IRID_LIGHTOFFSET;
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                            MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            if (fNotify) {
                nid[i].uFlags |= NIF_INFO;
                wsprintf(szTemp, L"%s: %s", tki[i].szTip, fKeyState ? L"On" : L"Off");
                lstrcpy(nid[i].szInfo, szTemp);
            }
            Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
            nid[i].uFlags &= ~NIF_INFO;
            break;
        }
        return 0;

    case WM_UITHEMECHANGED:
        for (INT i=0; i < NOTIFYICON_TOTAL; i++) {
            INT rid = tki[i].nIrid + IRID_ACTIVEOFFSET * (GetKeyState(tki[i].nVkey) & KS_TOGGLEACTIVE) + fLightThemeUsed * IRID_LIGHTOFFSET;
            nid[i].hIcon = (HICON)LoadImage((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
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

    // _ Window close & Session end message
    case WM_QUERYENDSESSION:
    case WM_CLOSE: {
        for (int i=0; i<NOTIFYICON_TOTAL; i++) {
            LPCTSTR flagstr = (nid[i].dwState & NIS_HIDDEN) ? TEXT("1") : TEXT("0");
            WritePrivateProfileString( TEXT(FN_CONFIGSECT), tki[i].szTip, flagstr, szConfPath );
        }
        WritePrivateProfileString(
            TEXT(FN_NOTIFYSECT), TEXT(FN_NTFYSECKEY),
            fNotify ? TEXT("1") : TEXT("0"), szConfPath);
        WritePrivateProfileString(NULL, NULL, NULL, NULL);  // Flush buffer
        if (uMsg == WM_QUERYENDSESSION) return TRUE;
        DestroyWindow(hwnd);
        return 0;
    }

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

/// Need to add compile option `-ladvapi32` to use Reg*() --> Not needed?
static void DetectUITheme(DWORD *pdwBufSize, DWORD *pfLightTheme) {
    *pdwBufSize = sizeof(*pfLightTheme);
    RegGetValue(
        HKEY_CURRENT_USER,
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

static void SetMenuItemCheckState(UINT uMenuItemId, BOOL fChecked)
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);      // Needed before obtain MenuItem
    mii.fMask = MIIM_STATE;                 // Must specify first the information to get
    GetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);   // Errno 0x57 returned if hMenu is incorrect
    if (fChecked) {
        mii.fState |= MFS_CHECKED;
    } else {
        mii.fState &= ~MFS_CHECKED;
    }
    SetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);
}

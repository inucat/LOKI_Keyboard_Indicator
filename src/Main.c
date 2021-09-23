/* TRANQUIL_TURTLE */
#define UNICODE         /// UTF-8 based
#include <sdkddkver.h>
#define NTDDI_VERSION NTDDI_WIN10           /// For `NOTIFYICONDATA.uVersion = 4'
#define _WIN32_WINNT _WIN32_WINNT_WIN10     /// For Registry functions
// #include <PathCch.h>    /// For PathCchRemoveFileSpec (preferred but link error, retry in a while)
#include <Shlwapi.h>    /// For PathRemoveFileSpec (deprecated though)
#include <Windows.h>
#include <windowsx.h>   /// GET_X(,Y)_LPARAM
#include "Resource.h"   /// My resource definitions
#include "Main.h"       /// etc.
// #define _CONSOLE_DEBUG

static HANDLE hConsole;    // For console debugging
static HWND hWindow;       // For KeyHookProc
static HMENU hMenu, hSubMenu;       // Icon Context Menu
static TCHAR szConfPath[MAX_PATH];  // Absolute path to the INI


/// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE null, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR szAppName[] = TEXT(STR_CODENAME);
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    #ifdef _CONSOLE_DEBUG
    // Get Handle to Console with Standard output
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        MessageBox(NULL, L"Console handle couldn't Obtained!!", L"", MB_ICONERROR);
    GetCurrentDirectory(MAX_PATH, szTemp);
    WriteConsole(hConsole, szTemp, lstrlen(szTemp), NULL, NULL);
    #endif

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
        szAppName,
        TEXT(STR_APPNAME),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd) return 0;

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

/// Keyboard Hook Proc.
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

/// Window Procedure Func.
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static const THEKEYINFO tki[TRAYICON_NUM] = {
        {ICID_NUML_DF, MIID_NUML, NIDID_NUML, VK_NUMLOCK, 0, L"Num Lock"},
        {ICID_CAPL_DF, MIID_CAPL, NIDID_CAPL, VK_CAPITAL, 0, L"Caps Lock"},
        {ICID_SCRL_DF, MIID_SCRL, NIDID_SCRL, VK_SCROLL, 0, L"Scroll Lock"},
        {ICID_INS_DF, MIID_INS, NIDID_INS, VK_INSERT, 0, L"Insert"}
    };
    static UINT uIconCount = 4;                 // Number of the key icons currently shown
    static NOTIFYICONDATA nid[TRAYICON_NUM];    // Notification Icon data
    static DWORD fLightThemeUsed;               // Flag being set if Light Theme used
    static HHOOK hKeyHook;                      // Handle to keyboard hook
    static BOOL fNotify = FALSE;                // Notification switch
    static BOOL fAutostart = FALSE;             // Autostart switch
    DWORD dwRegValueSize;                       // RegValue size to obtain
    TCHAR lpExePath[MAX_PATH] = {0};            // Path to executable, to register Autostart
    TCHAR szTemp[MAX_PATH] = {0};               // Versatile string buffer

    switch (uMsg)
    {
    //////////////////////////////////////////
    // ----- THE APPLICATION LAUNCHED ----- //
    //////////////////////////////////////////
    case WM_CREATE:
        /* (HookID, fnHookProc, Inst. having HookProc (NULL: this), Hooked ThreadID (0: global)) */
        hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
        if (!hKeyHook) {
            MessageBox(NULL, L"An error has been occurred with SetWindowsHookEx() and the program will be terminated.", TEXT(STR_APPNAME), MB_ICONERROR);
            SendMessage(hwnd, WM_DESTROY, 0, 0);
            return 0;
        }

        /* Get absolute path to the config file (needed when autostart) */
        GetModuleFileName(NULL, lpExePath, MAX_PATH);
        PathRemoveFileSpec(lpExePath);      // PathCchRemoveFileSpec(lpExePath, lstrlen(lpExePath));
        wsprintf(szConfPath, TEXT("%s%s"), lpExePath, TEXT(CONF_FILENAME));

        /* Adding icons to System tray */
        DetectUITheme(&fLightThemeUsed);
        for (INT i=0; i < 4; i++) {
            INT rid = GetIconResourceID(&tki[i], fLightThemeUsed);
                // tki[i].iconID + (GetKeyState(tki[i].virtkeyID) & GKS_IS_TOGGLED) * ICID_ON_OFFSET + fLightThemeUsed * ICID_LIGHT_OFFSET;   // Icon resource ID
            nid[i].cbSize = sizeof(NOTIFYICONDATA);
            nid[i].hWnd = hwnd;
            nid[i].uID = tki[i].nidID;                          // ID for tray items
            nid[i].uCallbackMessage = WM_TRAYICONCLICKED;       // App-defined Window Message
            nid[i].uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
            nid[i].uVersion = NOTIFYICON_VERSION_4;             // Make it easy to get clicked position
            nid[i].dwInfoFlags = NIIF_INFO;                     // Balloon Icon & Sound attr.
            nid[i].dwStateMask = NIS_HIDDEN;                    // Enable the Hidden state toggle
            nid[i].hIcon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, LR_SHARED);
            // ^ LoadIcon() wouldn't work. For LoadIconMetric() including `commctrl.h` & link `comctl32` is needed
            lstrcpy(nid[i].szTip, tki[i].szTip);                // Tip text
            lstrcpy(nid[i].szInfoTitle, TEXT(STR_APPNAME)); // Balloon Notification Title

            /* Test if the hidden flag is set in INI */
            if ( GetPrivateProfileInt(TEXT(CONF_HIDESECT), tki[i].szTip, 0, szConfPath) )
            {
                if (uIconCount != 1) {    // prevent lost of all four icons while reading config
                    nid[i].dwState = NIS_HIDDEN;
                    uIconCount--;
                    SetMenuItemCheckState(tki[i].menuitemID, FALSE);
                }
            }

            Shell_NotifyIcon(NIM_ADD, &nid[i]);                 // Add an item to tray
            Shell_NotifyIcon(NIM_SETVERSION, &nid[i]);          // Apply uVersion
        }

        /* Context menu on the icons */
        hMenu = LoadMenu(NULL, MAKEINTRESOURCE(MID_DUMMYPARENT));
        hSubMenu = GetSubMenu(hMenu, 0);

        /* Is autostart enabled? */
        dwRegValueSize = sizeof(lpExePath);
        GetModuleFileName(NULL, szTemp, sizeof(szTemp));
        RegGetValue(HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN), RRF_RT_REG_SZ, NULL, lpExePath, &dwRegValueSize);
        if ( !lstrcmp(lpExePath, szTemp) )
        {
            fAutostart = TRUE;
            SetMenuItemCheckState(MIID_AUTOSTART, TRUE);
        }

        /* Is toggle notification enabled? */
        fNotify = GetPrivateProfileInt(TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY), 0, szConfPath);
        if (fNotify) {
            SetMenuItemCheckState(MIID_SENDNOTIFY, TRUE);
        }

        return 0;

    ////////////////////////////////////////
    // ----- NOTIFY ICON IS CLICKED ----- //
    ////////////////////////////////////////
    case WM_TRAYICONCLICKED:
        if ( LOWORD(lParam) == WM_CONTEXTMENU )     /* Right-clicked, then menu appeared */
        {
            POINT pt;                               // Cursor position
            pt.x = GET_X_LPARAM(wParam);
            pt.y = GET_Y_LPARAM(wParam);
            SetForegroundWindow(hwnd);              // Prevent Menu from remaining or disappearing
            TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);       // Same as above
            #ifdef _CONSOLE_DEBUG
                wsprintf(szTemp, L"x = %d, y = %d\n", pt.x, pt.y);
                WriteConsole(hConsole, szTemp, lstrlen(szTemp), NULL, NULL);
            #endif
        }
        else if ( LOWORD(lParam) == WM_LBUTTONUP )  /* Left-clicked */
        {
            WORD niid = HIWORD(lParam);             // NID_ID of the clicked icon
            INPUT inputs[4];                        // Keystrokes
            ZeroMemory(inputs, sizeof(inputs));
            for (INT i=0; i < TRAYICON_NUM; i++) {
                if (tki[i].nidID == niid) {
                    inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
                    inputs[0].ki.wVk = inputs[1].ki.wVk = tki[i].virtkeyID;
                    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
                    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

                    /// @note It's unknown whether SendInput() has been accepted before the icon change.
                    INT rid = GetIconResourceID(&tki[i], fLightThemeUsed);
                        // tki[i].iconID + ICID_ON_OFFSET * (GetKeyState(tki[i].virtkeyID) & GKS_IS_TOGGLED) + fLightThemeUsed * ICID_LIGHT_OFFSET;
                    nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
                    Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
                    break;
                }
            }
        }
        return 0;

    /////////////////////////////////////////
    // ----- CONTEXT MENU IS PRESSED ----- //
    /////////////////////////////////////////
    case WM_COMMAND: {
        WORD menuitemID = LOWORD(wParam);     // Get clicked menu item id
        switch (menuitemID)
        {
        case MIID_SENDNOTIFY:
            fNotify = !fNotify;
            SetMenuItemCheckState(menuitemID, fNotify);
            break;

        case MIID_AUTOSTART:
            if (fAutostart) {
                RegDeleteKeyValue(HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN));
            } else {
                dwRegValueSize = sizeof(lpExePath);
                GetModuleFileName(NULL, lpExePath, sizeof(lpExePath));
                LONG lresult = RegSetKeyValue(
                    HKEY_CURRENT_USER,
                    TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN),
                    RRF_RT_REG_SZ, lpExePath, dwRegValueSize);
                if (lresult != ERROR_SUCCESS) {
                    MessageBox(NULL, L"Autostart registration failed.", TEXT(STR_APPNAME), MB_ICONERROR);
                    break;
                }
            }
            fAutostart = !fAutostart;
            SetMenuItemCheckState(menuitemID, fAutostart);
            break;

        case MIID_ABOUT: MessageBox( NULL, TEXT(STR_ABOUT), TEXT(STR_APPNAME), MB_OK | MB_ICONINFORMATION);
            break;
        case MIID_EXIT: SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        default:
            for (int i=0; i<TRAYICON_NUM; i++)
            {
                if (tki[i].menuitemID == menuitemID)
                {
                    INT niid = tki[i].nidID;
                    if (nid[niid].dwState & NIS_HIDDEN) {
                        uIconCount++;
                        SetMenuItemCheckState(menuitemID, TRUE);
                    } else if (uIconCount == 1) {
                        MessageBox(NULL, L"At least one icon must be kept.", TEXT(STR_APPNAME), MB_ICONWARNING | MB_OK);
                        break;
                    } else {
                        uIconCount--;
                        SetMenuItemCheckState(menuitemID, FALSE);
                    }
                    nid[niid].dwState ^= NIS_HIDDEN;
                    Shell_NotifyIcon(NIM_MODIFY, &(nid[niid]));
                    break;
                }
            }
            break;
        }   // switch
        return 0;
    }   // case WM_COMMAND

    case WM_LLKEYHOOKED: {
        INT virtkey = (INT) wParam;
        for (INT i=0; i < TRAYICON_NUM; i++)
        {
            if (tki[i].virtkeyID == virtkey)
            {
                SHORT fKeyState = GetKeyState(tki[i].virtkeyID) & GKS_IS_TOGGLED;
                INT rid = GetIconResourceID(&tki[i], fLightThemeUsed);
                    // tki[i].iconID + ICID_ON_OFFSET * fKeyState + fLightThemeUsed * ICID_LIGHT_OFFSET;
                nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
                if (fNotify) {
                    nid[i].uFlags |= NIF_INFO;
                    wsprintf(szTemp, L"%s: %s", tki[i].szTip, fKeyState ? L"On" : L"Off");
                    lstrcpy(nid[i].szInfo, szTemp);
                }
                Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
                nid[i].uFlags &= ~NIF_INFO;
                break;
            }
        }
        return 0;
    }   // WM_LLKEYHOOKED

    case WM_UITHEMECHANGED:
        for (INT i=0; i < TRAYICON_NUM; i++) {
            INT rid = GetIconResourceID(&tki[i], fLightThemeUsed);
            //  tki[i].iconID + ICID_ON_OFFSET * (GetKeyState(tki[i].virtkeyID) & GKS_IS_TOGGLED) + fLightThemeUsed * ICID_LIGHT_OFFSET;
            nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
        }
        return 0;

    // _ Theme color change detection
    case WM_SETTINGCHANGE:
        if (!lstrcmp((LPCWSTR) lParam, L"ImmersiveColorSet")) {
            DetectUITheme(&fLightThemeUsed);
            PostMessage(hwnd, WM_UITHEMECHANGED, 0, 0);
        }
        return 0;

    // _ Window close & Session end message
    case WM_QUERYENDSESSION:
    case WM_CLOSE: {
        for (int i=0; i<TRAYICON_NUM; i++) {
            LPCTSTR flagstr = (nid[i].dwState & NIS_HIDDEN) ? TEXT("1") : TEXT("0");
            WritePrivateProfileString( TEXT(CONF_HIDESECT), tki[i].szTip, flagstr, szConfPath );
        }
        WritePrivateProfileString(
            TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY),
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
        for (int i=0; i < TRAYICON_NUM; i++) {
            Shell_NotifyIcon(NIM_DELETE, &nid[i]);          // Remove Icons from the tray
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void DetectUITheme(DWORD *pfLightTheme) {
    DWORD dwBufferSize = sizeof(*pfLightTheme);
    RegGetValue(
        HKEY_CURRENT_USER,
        TEXT(REGK_UITHEME), TEXT(REGVN_UITHEME),
        RRF_RT_REG_DWORD, NULL, pfLightTheme, &dwBufferSize);
}

static void SetMenuItemCheckState(UINT uMenuItemId, BOOL fChecked)
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);      // Needed before obtain MenuItem
    mii.fMask = MIIM_STATE;                 // Must specify first the information to get
    GetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);   // Error 0x57 if hMenu is NULL
    if (fChecked) {
        mii.fState |= MFS_CHECKED;
    } else {
        mii.fState &= ~MFS_CHECKED;
    }
    SetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);
}

static INT GetIconResourceID(const THEKEYINFO *ptki, BOOL fLightTheme) {
    // INT rid = tki[i].iconID + ICID_ON_OFFSET * (GetKeyState(tki[i].virtkeyID) & GKS_IS_TOGGLED) + fLightThemeUsed * ICID_LIGHT_OFFSET;
    BOOL fIsKeyOn = (GetKeyState(ptki->virtkeyID) & GKS_IS_TOGGLED) && TRUE;
    return ptki->iconID + ICID_ON_OFFSET * fIsKeyOn + fLightTheme * ICID_LIGHT_OFFSET;
}
/* TRANQUIL_TURTLE */

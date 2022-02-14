#define UNICODE         /// UTF-8 support
#include <sdkddkver.h>
#define NTDDI_VERSION NTDDI_WIN10           /// `NOTIFYICONDATA.uVersion = 4'
#define _WIN32_WINNT _WIN32_WINNT_WIN10     /// Registry functions behave for Win10
#include <Shlwapi.h>    /// PathRemoveFileSpec()
#include <Windows.h>
#include <windowsx.h>   /// GET_X_LPARAM(), GET_Y_LPARAM()
#include "Resource.h"
#include "Main.h"

static HANDLE hConsole;    // For console debugging
static HWND hWindow;       // For KeyHookProc
static HMENU hMenu, hSubMenu;       // Icon Context Menu
static TCHAR szConfPath[MAX_PATH];  // Absolute path to the INI

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KeyHookProc(int, WPARAM, LPARAM);

/// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE null, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR szAppName[] = TEXT(STR_CODENAME);
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // Get console handle when debug flag is set
    #ifdef _CONSOLE_DEBUG
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        MessageBox(NULL, L"Failed to get console handle!", L"", MB_ICONERROR);
    GetCurrentDirectory(MAX_PATH, szTemp);
    WriteConsole(hConsole, szTemp, lstrlen(szTemp), NULL, NULL);
    #endif

    // Set Window Attributes & register Window class with them
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
    if (!RegisterClass(&wc)) return 0;

    hWindow = hwnd = CreateWindow(
        szAppName,
        TEXT(STR_APPNAME),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd) return 0;

    // Message loop, repeats until app terminates
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

/// Keyboard Hook Procedure
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

/// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static const struct key_icon_struct key_icons[NUM_TRAYICONS] = {
        {ICID_NUML_DF, MIID_NUML, NIDID_NUML, VK_NUMLOCK, 0, L"Num Lock"},
        {ICID_CAPL_DF, MIID_CAPL, NIDID_CAPL, VK_CAPITAL, 0, L"Caps Lock"},
        {ICID_SCRL_DF, MIID_SCRL, NIDID_SCRL, VK_SCROLL,  0, L"Scroll Lock"},
        {ICID_INS_DF,  MIID_INS,  NIDID_INS,  VK_INSERT,  0, L"Insert"}
    };
    static UINT uIconCount = 4;                 // Current number of the icons
    static NOTIFYICONDATA nid[NUM_TRAYICONS];   // Notification Icon data
    static HHOOK hKeyHook;
    static BOOL fThemeIsLight;
    static BOOL fNotifyIsEnabled = FALSE;
    static BOOL fAutostartIsEnabled = FALSE;
    DWORD dwRegValueSize;                       // RegValue size to obtain
    TCHAR lpExePath[MAX_PATH] = {0};            // Path to executable, to register Autostart
    TCHAR szTemp[MAX_PATH] = {0};               // Multi-purpose string buffer

    switch (uMsg)
    {
    // ----- THE APPLICATION LAUNCHED ----- //
    case WM_CREATE:
        // キーボードフックを登録し、トグルキー入力監視を始める。
        // Register keyboard hook and start watching toggle keys input.
        hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
        if (!hKeyHook) {
            MessageBox(NULL, L"SetWindowsHookEx() failed! Aborted.", TEXT(STR_APPNAME), MB_ICONERROR);
            SendMessage(hwnd, WM_DESTROY, 0, 0);
            return 0;
        }

        /* Absolute path of .ini needed when auto-started */
        GetModuleFileName(NULL, lpExePath, MAX_PATH);
        PathRemoveFileSpec(lpExePath);
        wsprintf(szConfPath, TEXT("%s%s"), lpExePath, TEXT(CONF_FILENAME));

        /* Context menu on the icons */
        hMenu = LoadMenu(NULL, MAKEINTRESOURCE(MID_DUMMYPARENT));
        hSubMenu = GetSubMenu(hMenu, 0);

        /* Adding icons to System tray */
        fThemeIsLight = check_theme_is_light();
        for (INT i=0; i < 4; i++) {
            INT rid = get_icon_rsrc_id(&key_icons[i], fThemeIsLight);
            nid[i].cbSize = sizeof(NOTIFYICONDATA);
            nid[i].hWnd = hwnd;
            nid[i].uID = key_icons[i].nidID;                    // ID for tray items
            nid[i].uCallbackMessage = WM_TRAYICONCLICKED;       // App-defined Window Message
            nid[i].uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
            nid[i].uVersion = NOTIFYICON_VERSION_4;             // Needed to get mouse position easily
            nid[i].dwInfoFlags = NIIF_INFO;                     // Balloon Icon & Sound attr.
            nid[i].dwStateMask = NIS_HIDDEN;                    // Enable the Hidden state toggle
            nid[i].hIcon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, LR_SHARED);
            lstrcpy(nid[i].szTip, key_icons[i].szTip);          // Tip text
            lstrcpy(nid[i].szInfoTitle, TEXT(STR_APPNAME));     // Balloon Notification Title

            // INIファイルで隠しフラグが立っているアイコンを隠す。ただし最低1つは残す。
            // Hides icons if their hidden flag is set in INI, but preserves at least one.
            if ( GetPrivateProfileInt(TEXT(CONF_HIDESECT), key_icons[i].szTip, 0, szConfPath) )
            {
                if (uIconCount != 1) {
                    nid[i].dwState = NIS_HIDDEN;
                    uIconCount--;
                    set_mitem_check_state(key_icons[i].menuitemID, FALSE);
                }
            }

            // Add to tray & apply uVersion
            Shell_NotifyIcon(NIM_ADD, &nid[i]);
            Shell_NotifyIcon(NIM_SETVERSION, &nid[i]);
        }

        /* Is toggle notification enabled? */
        fNotifyIsEnabled = GetPrivateProfileInt(TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY), 0, szConfPath);
        if (fNotifyIsEnabled) {
            set_mitem_check_state(MIID_SENDNOTIFY, TRUE);
        }

        /* Is autostart enabled? */
        dwRegValueSize = sizeof(lpExePath);
        GetModuleFileName(NULL, szTemp, sizeof(szTemp));
        RegGetValue(HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN), RRF_RT_REG_SZ, NULL, lpExePath, &dwRegValueSize);
        if ( !lstrcmp(lpExePath, szTemp) )
        {
            fAutostartIsEnabled = TRUE;
            set_mitem_check_state(MIID_AUTOSTART, TRUE);
        }

        return 0;

    // ----- NOTIFY ICON IS CLICKED ----- //
    case WM_TRAYICONCLICKED:
        /* Right-clicked, then menu appeared */
        if ( LOWORD(lParam) == WM_CONTEXTMENU )
        {
            POINT pt;                               // Cursor position
            pt.x = GET_X_LPARAM(wParam);
            pt.y = GET_Y_LPARAM(wParam);
            SetForegroundWindow(hwnd);              // Prevent Menu from remaining or disappearing
            TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);       // Same as above
        }
        /* Left-clicked, then toggle the key */
        else if ( LOWORD(lParam) == WM_LBUTTONUP )
        {
            WORD niid = HIWORD(lParam);             // NID_ID of the clicked icon
            INPUT inputs[4];                        // Keystrokes
            ZeroMemory(inputs, sizeof(inputs));
            for (INT i=0; i < NUM_TRAYICONS; i++) {
                if (key_icons[i].nidID == niid) {
                    inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
                    inputs[0].ki.wVk = inputs[1].ki.wVk = key_icons[i].virtkeyID;
                    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
                    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

                    /// @note It's not sure that SendInput() has been accepted before the icon change.
                    INT rid = get_icon_rsrc_id(&key_icons[i], fThemeIsLight);
                    nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
                    Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
                    break;
                }
            }
        }
        return 0;

    // ----- CONTEXT MENU IS PRESSED ----- //
    case WM_COMMAND: {
        WORD menuitemID = LOWORD(wParam);     // Get clicked menu item id
        switch (menuitemID)
        {
        case MIID_SENDNOTIFY:
            fNotifyIsEnabled = !fNotifyIsEnabled;
            set_mitem_check_state(menuitemID, fNotifyIsEnabled);
            break;

        case MIID_AUTOSTART:
            if (fAutostartIsEnabled) {
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
            fAutostartIsEnabled = !fAutostartIsEnabled;
            set_mitem_check_state(menuitemID, fAutostartIsEnabled);
            break;

        case MIID_ABOUT: MessageBox( NULL, TEXT(STR_ABOUT), TEXT(STR_APPNAME), MB_OK | MB_ICONINFORMATION);
            break;
        case MIID_EXIT: SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        default:
            for (int i=0; i<NUM_TRAYICONS; i++)
            {
                if (key_icons[i].menuitemID == menuitemID)
                {
                    INT niid = key_icons[i].nidID;
                    if (nid[niid].dwState & NIS_HIDDEN) {
                        uIconCount++;
                        set_mitem_check_state(menuitemID, TRUE);
                    } else if (uIconCount == 1) {
                        MessageBox(NULL, L"At least one icon must be present.", TEXT(STR_APPNAME), MB_ICONWARNING | MB_OK);
                        break;
                    } else {
                        uIconCount--;
                        set_mitem_check_state(menuitemID, FALSE);
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
        for (INT i=0; i < NUM_TRAYICONS; i++)
        {
            if (key_icons[i].virtkeyID == virtkey)
            {
                SHORT fKeyState = GetKeyState(key_icons[i].virtkeyID) & GKS_IS_TOGGLED;
                INT rid = get_icon_rsrc_id(&key_icons[i], fThemeIsLight);
                nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
                if (fNotifyIsEnabled) {
                    nid[i].uFlags |= NIF_INFO;
                    wsprintf(szTemp, L"%s: %s", key_icons[i].szTip, fKeyState ? L"On" : L"Off");
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
        for (INT i=0; i < NUM_TRAYICONS; i++) {
            INT rid = get_icon_rsrc_id(&key_icons[i], fThemeIsLight);
            nid[i].hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            Shell_NotifyIcon(NIM_MODIFY, &nid[i]);
        }
        return 0;

    // _ Theme color change detection
    case WM_SETTINGCHANGE:
        if (!lstrcmp((LPCWSTR) lParam, L"ImmersiveColorSet")) {
            fThemeIsLight = check_theme_is_light();
            PostMessage(hwnd, WM_UITHEMECHANGED, 0, 0);
        }
        return 0;

    // _ Window close & Session end message
    case WM_QUERYENDSESSION:
    case WM_CLOSE: {
        for (int i=0; i<NUM_TRAYICONS; i++) {
            LPCTSTR flagstr = (nid[i].dwState & NIS_HIDDEN) ? TEXT("1") : TEXT("0");
            WritePrivateProfileString( TEXT(CONF_HIDESECT), key_icons[i].szTip, flagstr, szConfPath );
        }
        WritePrivateProfileString(
            TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY),
            fNotifyIsEnabled ? TEXT("1") : TEXT("0"), szConfPath);
        WritePrivateProfileString(NULL, NULL, NULL, NULL);  // Flush buffer
        if (uMsg == WM_QUERYENDSESSION) return TRUE;
        DestroyWindow(hwnd);
        return 0;
    }

    // _ Window destroy message
    case WM_DESTROY:
        UnhookWindowsHookEx(hKeyHook);
        DestroyMenu(hMenu);

        // Remove Icons from the tray
        for (int i=0; i < NUM_TRAYICONS; i++) {
            Shell_NotifyIcon(NIM_DELETE, &nid[i]);
        }

        PostQuitMessage(0);
        return 0;
    } // switch

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static BOOL check_theme_is_light(void) {
    DWORD f_theme_is_light = 0;
    DWORD reg_value_size = sizeof(f_theme_is_light);
    RegGetValue(
        HKEY_CURRENT_USER,
        TEXT(REGK_UITHEME), TEXT(REGVN_UITHEME),
        RRF_RT_REG_DWORD, NULL, &f_theme_is_light, &reg_value_size);
    return f_theme_is_light;
}

static void set_mitem_check_state(UINT uMenuItemId, BOOL fChecked)
{
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);      // Needed before getting MenuItem instance
    mii.fMask = MIIM_STATE;                 // Specifies the info to get
    GetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);   // Error 0x57 if hMenu is NULL
    if (fChecked) {
        mii.fState |= MFS_CHECKED;
    } else {
        mii.fState &= ~MFS_CHECKED;
    }
    SetMenuItemInfo(hSubMenu, uMenuItemId, FALSE, &mii);
}

static INT get_icon_rsrc_id(const struct key_icon_struct *pkis, BOOL fLightTheme) {
    // TODO: && TRUE is not necessary, so remove this
    BOOL fKeyIsOn = (GetKeyState(pkis->virtkeyID) & GKS_IS_TOGGLED) && TRUE;
    return pkis->iconID + ICID_ON_OFFSET * fKeyIsOn + fLightTheme * ICID_LIGHT_OFFSET;
}

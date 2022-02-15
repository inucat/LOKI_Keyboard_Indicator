#define UNICODE                             // UTF-8 support
#include <sdkddkver.h>
#define NTDDI_VERSION NTDDI_WIN10           // `NOTIFYICONDATA.uVersion = 4'
#define _WIN32_WINNT _WIN32_WINNT_WIN10     // Makes registry functions behave for Win10
#include <shlwapi.h>    // PathRemoveFileSpec()
#include <windows.h>
#include <windowsx.h>   // GET_X_LPARAM(), GET_Y_LPARAM()
#include "Resource.h"
#include "Procedures.h"

struct key_icon_struct {
    INT icon_rid;       // Icon resource ID
    INT mitem_id;       // Menu item ID
    INT vkey_id;        // Virtual Key ID
    LPCTSTR tip_text;   // Icon tool tip text
    NOTIFYICONDATA *nidata;
};

static void set_mitem_check_state(UINT, BOOL);
static BOOL check_theme_is_light(void);
INT get_icon_rsrc_id(const struct key_icon_struct*, BOOL);
LRESULT CALLBACK KeyHookProc(int, WPARAM, LPARAM);

static struct key_icon_struct key_icons[MAX_TRAYICONS] = {
    {ICID_NUML_DF, MIID_NUML, VK_NUMLOCK, L"Num Lock"},
    {ICID_CAPL_DF, MIID_CAPL, VK_CAPITAL, L"Caps Lock"},
    {ICID_SCRL_DF, MIID_SCRL, VK_SCROLL,  L"Scroll Lock"},
    {ICID_INS_DF,  MIID_INS,  VK_INSERT,  L"Insert"}
};

static HWND hWnd_g;
static HHOOK hKeyHook_g;
static HMENU hMenu_g, hSubMenu_g;
static BOOL themeIsLight_g;
static BOOL notifyEnabled_g;
static BOOL autostartEnabled_g;
static TCHAR iniPath_g[MAX_PATH];
static UINT iconCount_g = 4;

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYUP) {
        KBDLLHOOKSTRUCT *pKeyStruct = (KBDLLHOOKSTRUCT *)lParam;
        if (pKeyStruct->flags & LLKHF_UP) {
            PostMessage(hWnd_g, AWM_LLKEYHOOKED, pKeyStruct->vkCode, 0);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

Define_WMHandler(wmCreate) {
    // キーボードフックを登録し、トグルキー入力監視を始める。
    // Register keyboard hook and start watching toggle keys input.
    hWnd_g  = hwnd;
    hKeyHook_g = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
    if (!hKeyHook_g) {
        MessageBox(NULL, L"SetWindowsHookEx() failed! Aborted.", TEXT(STR_APPNAME), MB_ICONERROR);
        SendMessage(hwnd, WM_DESTROY, 0, 0);
        return;
    }

    /* Context menu on the icons */
    hMenu_g = LoadMenu(NULL, MAKEINTRESOURCE(MID_DUMMYPARENT));
    hSubMenu_g = GetSubMenu(hMenu_g, 0);

    // Checks autostart is enabled by looking up the entry with EXE path in a registry key
    TCHAR path_to_exe[MAX_PATH] = {0};
    TCHAR path_to_exe_on_reg[MAX_PATH] = {0};
    DWORD reg_value_size = sizeof(path_to_exe_on_reg);
    GetModuleFileName(NULL, path_to_exe, sizeof(path_to_exe));
    RegGetValue(HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN), RRF_RT_REG_SZ, NULL,
                path_to_exe_on_reg, &reg_value_size);
    autostartEnabled_g = !lstrcmp(path_to_exe, path_to_exe_on_reg);
    if ( autostartEnabled_g ) {
        set_mitem_check_state(MIID_AUTOSTART, TRUE);
    }

    // Path needed for Autostart registration & INI Load
    TCHAR * path_to_exe_dir = path_to_exe;
    PathRemoveFileSpec(path_to_exe_dir);
    wsprintf(iniPath_g, TEXT("%s/%s"), path_to_exe_dir, TEXT(CONF_FILENAME));

    // Checks notification is enabled
    notifyEnabled_g = GetPrivateProfileInt(TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY), 0, iniPath_g);
    if (notifyEnabled_g) {
        set_mitem_check_state(MIID_SENDNOTIFY, TRUE);
    }

    /* Adding icons to System tray */
    themeIsLight_g = check_theme_is_light();
    for (INT i=0; i < MAX_TRAYICONS; i++) {
        NOTIFYICONDATA *p_nidata = key_icons[i].nidata;
        INT rid = get_icon_rsrc_id(&key_icons[i], themeIsLight_g);
        p_nidata->cbSize            = sizeof(NOTIFYICONDATA);
        p_nidata->hWnd              = hwnd;
        p_nidata->uID               = i;                        // Tray item ID
        p_nidata->uCallbackMessage  = AWM_TRAYICONCLICKED;      // WndMsg sent when icons clicked
        p_nidata->uFlags            = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
        p_nidata->uVersion          = NOTIFYICON_VERSION_4;     // For precise click position
        p_nidata->dwInfoFlags       = NIIF_INFO;                // Balloon Icon & Sound attr.
        p_nidata->dwStateMask       = NIS_HIDDEN;               // Enable the hidden state switch
        p_nidata->hIcon             = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, LR_SHARED);
        lstrcpy(p_nidata->szTip, key_icons[i].tip_text);           // Tip text
        lstrcpy(p_nidata->szInfoTitle, TEXT(STR_APPNAME));      // Balloon Notification Title

        // INIファイルで隠しフラグが立っているアイコンを隠す。ただし最低1つは残す。
        // Hides icons if their hidden flag is set in INI, but preserves at least one.
        UINT hidden_flag_is_set =
            GetPrivateProfileInt(TEXT(CONF_HIDESECT), key_icons[i].tip_text, 0, iniPath_g);
        if ( hidden_flag_is_set && iconCount_g > 1 ) {
            p_nidata->dwState = NIS_HIDDEN;
            iconCount_g--;
            set_mitem_check_state(key_icons[i].mitem_id, FALSE);
        }

        // Adds to tray & apply uVersion
        Shell_NotifyIcon(NIM_ADD, p_nidata);
        Shell_NotifyIcon(NIM_SETVERSION, p_nidata);
    }
}

Define_WMHandler(wmTrayIconClicked) {
    // Right-click to show menu
    if ( LOWORD(lParam) == WM_CONTEXTMENU ) {
        POINT pt;
        pt.x = GET_X_LPARAM(wParam);
        pt.y = GET_Y_LPARAM(wParam);
        SetForegroundWindow(hwnd);          // Prevents menu from remaining or disappearing
        TrackPopupMenu( hSubMenu_g, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL );
        PostMessage(hwnd, WM_NULL, 0, 0);   // Same as above
    }

    // Left-click to toggle the key
    if ( LOWORD(lParam) == WM_LBUTTONUP ) {
        WORD clicked_icon_id = HIWORD(lParam);  // nidata->uID == `key_icons[]` index of the clicked icon
        struct key_icon_struct *clicked_icon = &key_icons[clicked_icon_id];
        INPUT inputs[2];                        // Keystrokes sent to Windows
        ZeroMemory(inputs, sizeof(inputs));
        inputs[0].type       = inputs[1].type   = INPUT_KEYBOARD;
        inputs[0].ki.wVk     = inputs[1].ki.wVk = clicked_icon->vkey_id;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

        /// @note It's not sure that SendInput() has been accepted before the icon change.
        INT rid = get_icon_rsrc_id(clicked_icon, themeIsLight_g);
        clicked_icon->nidata->hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
        Shell_NotifyIcon(NIM_MODIFY, clicked_icon->nidata);
        return;
    }
}

Define_WMHandler(wmMenuItemClicked) {
    WORD clicked_item = LOWORD(wParam);
    switch (clicked_item) {
    case MIID_SENDNOTIFY:
        notifyEnabled_g = !notifyEnabled_g;
        set_mitem_check_state(clicked_item, notifyEnabled_g);
        return;

    case MIID_AUTOSTART:
        if (autostartEnabled_g) {
            RegDeleteKeyValue(HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN));
        } else {
            DWORD reg_value_size;
            TCHAR path_to_exe[MAX_PATH] = {0};
            reg_value_size = sizeof(path_to_exe);
            GetModuleFileName(NULL, path_to_exe, sizeof(path_to_exe));
            LONG errcode =
                RegSetKeyValue( HKEY_CURRENT_USER, TEXT(REGK_USERAUTORUN), TEXT(REGVN_AUTORUN),
                                RRF_RT_REG_SZ, path_to_exe, reg_value_size );
            if (errcode != ERROR_SUCCESS) {
                MessageBox(NULL, L"Autostart registration failed.", TEXT(STR_APPNAME), MB_ICONERROR);
                return;
            }
        }
        autostartEnabled_g = !autostartEnabled_g;
        set_mitem_check_state(clicked_item, autostartEnabled_g);
        return;

    case MIID_ABOUT:
        MessageBox( NULL, TEXT(STR_ABOUT), TEXT(STR_APPNAME), MB_OK | MB_ICONINFORMATION );
        return;
    case MIID_EXIT:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        return;

    // The clicked item is for icon switch
    default:
        for (int i=0; i<MAX_TRAYICONS; i++) {
            if (key_icons[i].mitem_id == clicked_item) {
                NOTIFYICONDATA *target_nidata = key_icons[i].nidata;
                BOOL is_checked;
                if (target_nidata->dwState & NIS_HIDDEN) {
                    iconCount_g++;
                    is_checked = TRUE;
                } else {
                    if (iconCount_g == 1) {
                        MessageBox(NULL, L"At least one icon must be present.", TEXT(STR_APPNAME), MB_ICONWARNING | MB_OK);
                        return;
                    }
                    iconCount_g--;
                    is_checked = FALSE;
                }
                set_mitem_check_state(clicked_item, is_checked);
                target_nidata->dwState ^= NIS_HIDDEN;
                Shell_NotifyIcon(NIM_MODIFY, target_nidata);
                return;
            }
        }
        return;
    } // switch (clicked_item)
}

Define_WMHandler(wmLLKeyHooked) {
    INT pressed_key = (INT) wParam;
    for (INT i=0; i < MAX_TRAYICONS; i++) {
        if (key_icons[i].vkey_id == pressed_key) {
            struct key_icon_struct *clicked_icon = &key_icons[i];
            SHORT fKeyState = GetKeyState(clicked_icon->vkey_id) & GKS_IS_TOGGLED;
            INT rsrc_id = get_icon_rsrc_id(clicked_icon, themeIsLight_g);
            clicked_icon->nidata->hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rsrc_id), IMAGE_ICON, 0, 0, 0);
            if (notifyEnabled_g) {
                TCHAR notify_message[ sizeof(clicked_icon->nidata->szInfo) ] = {0};
                clicked_icon->nidata->uFlags |= NIF_INFO;
                wsprintf(notify_message, L"%s: %s", key_icons[i].tip_text, fKeyState ? L"On" : L"Off");
                lstrcpy(clicked_icon->nidata->szInfo, notify_message);
            }
            Shell_NotifyIcon(NIM_MODIFY, clicked_icon->nidata);
            clicked_icon->nidata->uFlags &= ~NIF_INFO;
            return;
        }
    }
}

Define_WMHandler(wmThemeChanged) {
    int theme_has_changed = !lstrcmp( (LPCWSTR) lParam, L"ImmersiveColorSet" );
    if ( theme_has_changed ) {
        themeIsLight_g = check_theme_is_light();
        for (INT i=0; i < MAX_TRAYICONS; i++) {
            INT rid = get_icon_rsrc_id(&key_icons[i], themeIsLight_g);
            key_icons[i].nidata->hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(rid), IMAGE_ICON, 0, 0, 0);
            Shell_NotifyIcon(NIM_MODIFY, key_icons[i].nidata);
        }
    }
}

Define_WMHandler(wmDestroy) {
    UnhookWindowsHookEx(hKeyHook_g);
    DestroyMenu(hMenu_g);

    // Remove Icons from the tray
    for (int i=0; i < MAX_TRAYICONS; i++) {
        Shell_NotifyIcon(NIM_DELETE, key_icons[i].nidata);
    }

    PostQuitMessage(0);
}

Define_WMHandler(wmOnExit) {
    // Saves configurations
    for (int i=0; i<MAX_TRAYICONS; i++) {
        LPCTSTR flagstr = (key_icons[i].nidata->dwState & NIS_HIDDEN) ? TEXT("1") : TEXT("0");
        WritePrivateProfileString( TEXT(CONF_HIDESECT), key_icons[i].tip_text, flagstr, iniPath_g );
    }
    WritePrivateProfileString(TEXT(CONF_NOTISECT), TEXT(CONF_NOTISKEY), notifyEnabled_g ? TEXT("1") : TEXT("0"), iniPath_g);
    WritePrivateProfileString(NULL, NULL, NULL, NULL);  // Flush buffer
}

/// Checks which Windows theme currently applied, Dark or Light
static BOOL check_theme_is_light(void) {
    DWORD theme_is_light = 0;
    DWORD reg_value_size = sizeof(theme_is_light);
    RegGetValue(
        HKEY_CURRENT_USER, TEXT(REGK_UITHEME), TEXT(REGVN_UITHEME),
        RRF_RT_REG_DWORD, NULL, &theme_is_light, &reg_value_size);
    return theme_is_light;
}

/// Check/Uncheck Menu Item
static void set_mitem_check_state(UINT mitem_id, BOOL is_checked) {
    MENUITEMINFO miinfo = {0};
    miinfo.cbSize = sizeof(MENUITEMINFO);
    miinfo.fMask = MIIM_STATE;
    GetMenuItemInfo(hSubMenu_g, mitem_id, FALSE, &miinfo);
    if ( is_checked ) {
        miinfo.fState |= MFS_CHECKED;
    } else {
        miinfo.fState &= ~MFS_CHECKED;
    }
    SetMenuItemInfo(hSubMenu_g, mitem_id, FALSE, &miinfo);
}

/// Get icon resource ID corresponding to the key state & Win theme
INT get_icon_rsrc_id(const struct key_icon_struct *kicon_struct, BOOL theme_is_light) {
    // `&& TRUE` is to ensure that the expression is 0 or 1 regardless of what GetKeyState() returns
    BOOL key_is_on = ( GetKeyState(kicon_struct->vkey_id) & GKS_IS_TOGGLED ) && TRUE;
    INT rsrc_id = kicon_struct->icon_rid;
    rsrc_id += ICID_ON_OFFSET * key_is_on;
    rsrc_id += ICID_LIGHT_OFFSET * theme_is_light;
    return rsrc_id;
}

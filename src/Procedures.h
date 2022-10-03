#pragma once

#include <windows.h>

// Tray icons total
#define MAX_TRAYICONS 4

// Bit mask for GetKeyState() toggled state
#define GKS_IS_TOGGLED 1

/* --- CONFig file definitions --- */
#define CONF_FILENAME "loki.ini"
#define CONF_HIDESECT "HiddenFlag"
#define CONF_NOTISECT "Notification"
#define CONF_NOTISKEY "Enabled"

/* --- REGistry Keys & Value Names --- */
/* Current UI theme (Dark or Light) */
#define REGK_UITHEME "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define REGVN_UITHEME "AppsUseLightTheme"
/* User's auto run entry */
#define REGK_USERAUTORUN "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGVN_AUTORUN "LOKI-{48D10A91-BF2A-40A4-AE41-55BA0662EC6E}"

#define STR_ABOUT "Version:\t" STR_APPVER "\n"      \
                  "Release:\t" STR_RELEASEDATE "\n" \
                  "Author:\t" STR_AUTHOR

/// App-specific Window Messages
/// @note The values over WM_APP can be used.
enum APP_DEFINED_WINDOW_MESSAGE
{
    AWM_TRAYICONCLICKED = ((WM_APP) + 100), ///< Emitted when a tray icon is clicked.
    // AWM_UITHEMECHANGED,
    AWM_LLKEYHOOKED ///< Key hook is triggered.
};

// IDs to distinguish each NOTIFYICONDATA
enum NID_ID
{
    NIDID_NUML,
    NIDID_CAPL,
    NIDID_SCRL,
    NIDID_INS
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#define Define_WMHandler(handler_name) void handler_name(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
Define_WMHandler(wmTrayIconClicked);
Define_WMHandler(wmCreate);
Define_WMHandler(wmMenuItemClicked);
Define_WMHandler(wmLLKeyHooked);
Define_WMHandler(wmThemeChanged);
Define_WMHandler(wmOnExit);
Define_WMHandler(wmDestroy);

#pragma once

// Tray icons total
#define TRAYICON_NUM    4

// Bit mask for GetKeyState() toggled state
#define GKS_IS_TOGGLED  1

/* --- CONFig file definitions --- */
#define CONF_FILENAME       "\\loki.ini"
#define CONF_HIDESECT       "HiddenFlag"        // Values will be their respective hidden state
#define CONF_NOTISECT       "Notification"
#define CONF_NOTISKEY       "Enabled"

/* --- REGistry Keys and Value Names --- */
/* Current UI theme (Dark or Light) */
#define REGK_UITHEME    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define REGVN_UITHEME   "AppsUseLightTheme"
/* User specific auto run */
#define REGK_USERAUTORUN    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGVN_AUTORUN       "LOKI-{48D10A91-BF2A-40A4-AE41-55BA0662EC6E}"       // Value data will be the path to LOKI.exe

#define STR_ABOUT "Version:\t" STR_APPVER "\n" \
                  "Release:\t" STR_RELEASEDATE "\n" \
                  "Author:\t" STR_AUTHOR

/// Application-defined Window Messages
/// @note The values over WM_APP can be used.
enum APP_DEFINED_WINDOW_MESSAGE {
    WM_TRAYICONCLICKED = ((WM_APP) + 100),
    WM_UITHEMECHANGED,
    WM_LLKEYHOOKED
};

// IDs to distinguish each NOTIFYICONDATA
enum NID_ID {
    NIDID_NUML,
    NIDID_CAPL,
    NIDID_SCRL,
    NIDID_INS
};

// Arrange the key and icon data
typedef struct tagTHEKEYINFO {
    INT iconID;     // Icon (resource) ID
    INT menuitemID; // Menu item ID
    INT nidID;      // NOTIFYICONDATA ID
    INT virtkeyID;  // Virtual Key ID
    INT nFlagMask;  // not used
    LPCTSTR szTip;  // Icon tool tip text
} THEKEYINFO;


// <!--- Prototype declaration --->

/// Check/Uncheck Menu Item
/// @param uMenuItemId Menu item ID
/// @param fChecked State to set (TRUE=checked, FALSE=unchecked)
static void SetMenuItemCheckState(UINT uMenuItemId, BOOL fChecked);

/// Detect which UI Theme currently applied, Dark or Light
/// @param pdwBufSize   Not used.
/// @param pfLightTheme Receives the value.  1 if Light theme is selected and 0 otherwise.
static void DetectUITheme(DWORD *pfLightTheme);

/// Get icon resource ID corresponding to the key state & UI theme
/// @param ptki Pointer to the key's THEKEYINFO
/// @param fLightTheme Flag of UI theme mode
/// @return Icon resource ID
static INT GetIconResourceID(const THEKEYINFO *ptki, BOOL fLightTheme);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);

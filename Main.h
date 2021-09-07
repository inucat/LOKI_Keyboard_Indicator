#pragma once

/// Config Filename
#define FN_CONFIGFILE       "\\loki.ini"
#define FN_CONFIGSECT       "HiddenFlag"
#define FN_NOTIFYSECT       "Notification"
#define FN_NTFYSECKEY       "Enabled"

// Num of the icons of keys to observe
#define NOTIFYICON_TOTAL    4

// Toggle key active state
#define KS_TOGGLEACTIVE     1

// Reg key and value name containing current UI theme (Dark or Light)
#define REGKEYPATH_THEMEMODEsz  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define REG_VALUENAME           "AppsUseLightTheme"

#define REGSUBKEY_USERSTARTUP   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGVALUE_LOKI           "LOKI-{48D10A91-BF2A-40A4-AE41-55BA0662EC6E}"       // Value name:value is the path to LOKI.exe

/// Application-defined Window Messages
/// The values over WM_APP can be used.
enum tagAPPDEFINEDWINDOWMESSAGE {
    WM_NOTIFYICONCLICKED = ((WM_APP) + 100),
    WM_UITHEMECHANGED,
    WM_LLKEYHOOKED
};

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
    LPCTSTR szTip;
} THEKEYINFO;

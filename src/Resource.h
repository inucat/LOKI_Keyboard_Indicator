/// @file Resource.h
/// @brief Definitions of resource ID
#ifndef _RESOURCE_H_
#define _RESOURCE_H_

/// <!-- Check and modify at every new release

#define STR_APPVER      "0.4.2"
#define STR_RELEASEDATE "2021/09/07"
#define NUM_APPVER      0,4,2,15

/// -->

#define STR_APPNAME     "LOKI - the Keyboard Indicator"
#define STR_APPFILENAME "LOKI.exe"
#define STR_CODENAME    "TRANQUIL_TURTLE"   // Used for Window Class name
#define STR_AUTHOR      "inucat"

// _ Icon Resource ID
#define ICID_APPICON        100
// _ +4 for `ACTIVE`, +8 for `LIGHT` variants respectively
#define ICID_ON_OFFSET   4
#define ICID_LIGHT_OFFSET    8

#define ICID_NUML_DF    (429)
#define ICID_NUML_DN    ((ICID_NUML_DF) + 4)
#define ICID_NUML_LF    ((ICID_NUML_DF) + 8)
#define ICID_NUML_LN    ((ICID_NUML_DF) + 12)
#define ICID_CAPL_DF    ((ICID_NUML_DF) + 1)
#define ICID_CAPL_DN    ((ICID_NUML_DF) + 5)
#define ICID_CAPL_LF    ((ICID_NUML_DF) + 9)
#define ICID_CAPL_LN    ((ICID_NUML_DF) + 13)
#define ICID_SCRL_DF    ((ICID_NUML_DF) + 2)
#define ICID_SCRL_DN    ((ICID_NUML_DF) + 6)
#define ICID_SCRL_LF    ((ICID_NUML_DF) + 10)
#define ICID_SCRL_LN    ((ICID_NUML_DF) + 14)
#define ICID_INS_DF     ((ICID_NUML_DF) + 3)
#define ICID_INS_DN     ((ICID_NUML_DF) + 7)
#define ICID_INS_LF     ((ICID_NUML_DF) + 11)
#define ICID_INS_LN     ((ICID_NUML_DF) + 15)

// _ Menu ID
#define MID_DUMMYPARENT 118

// _ Menu Item ID
#define MIID_NUML           219
#define MIID_CAPL           220
#define MIID_SCRL           221
#define MIID_INS            222
#define MIID_SENDNOTIFY     223
#define MIID_NOTIFYSOUND    224
#define MIID_AUTOSTART      225
#define MIID_ABOUT          226
#define MIID_EXIT           227

#endif

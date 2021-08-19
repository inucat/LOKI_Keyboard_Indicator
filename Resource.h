/// @file Resource.h
/// @brief Definitions of resource ID
#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#define RES_APPNAMEsz       "LOKI - the Keyboard Indicator"
#define RES_CODENAMEsz      "TRANQUIL_TURTLE"
#define RES_APPVERsz        "0.1.0-BETA"
#define RES_VERINFOnum      0,1,0,11
#define TO_TEXT(val)        #val
#define RES_RELEASEDATEsz   "2021/08/19"
#define RES_AUTHORsz        "inucat"

#define MRID_MANIFEST   999

// _ Icon Resource ID
#define IRID_APPICON    544
// _ +4 for `ACTIVE`, +8 for `LIGHT` variants respectively
#define IRID_ACTIVEOFFSET   4
#define IRID_LIGHTOFFSET    8

#define IRID_DNUMD      (429)
#define IRID_DCAPSD     ((IRID_DNUMD) + 1)
#define IRID_DSCROLLD   ((IRID_DNUMD) + 2)
#define IRID_DINSERTD   ((IRID_DNUMD) + 3)
#define IRID_DNUMA      ((IRID_DNUMD) + 4)
#define IRID_DCAPSA     ((IRID_DNUMD) + 5)
#define IRID_DSCROLLA   ((IRID_DNUMD) + 6)
#define IRID_DINSERTA   ((IRID_DNUMD) + 7)
#define IRID_LNUMD      ((IRID_DNUMD) + 8)
#define IRID_LCAPSD     ((IRID_DNUMD) + 9)
#define IRID_LSCROLLD   ((IRID_DNUMD) + 10)
#define IRID_LINSERTD   ((IRID_DNUMD) + 11)
#define IRID_LNUMA      ((IRID_DNUMD) + 12)
#define IRID_LCAPSA     ((IRID_DNUMD) + 13)
#define IRID_LSCROLLA   ((IRID_DNUMD) + 14)
#define IRID_LINSERTA   ((IRID_DNUMD) + 15)
// : 429 + 15 = 444

// _ Menu ID
#define MID_NIMENU_DUMMYPARENT   118

// _ Menu Item ID
#define MIID_NUMLOCK    219
#define MIID_CAPSLOCK   220
#define MIID_SCROLLLOCK 221
#define MIID_INSERT     222
#define MIID_SENDNOTIFY 223
#define MIID_NOTIFYSOUND    224
#define MIID_AUTOSTART  225
#define MIID_ABOUT      226
#define MIID_EXIT       227

// _ Version Info Resource ID
#define VIID_VERSION    328

#endif

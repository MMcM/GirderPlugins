/* Zoom Player LCD interface */

#include "stdafx.h"
#include "plugin.h"

void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow)
{
  HWND hwndZP = (HWND)strtoul(data, NULL, 16);

  char szName[128];
  GetWindowText(hMonitorWindow, szName, sizeof(szName));
  ATOM nName = GlobalAddAtom(szName);
  // Tell ZoomPlayer to send LCD messages to our monitor window.
  SendMessage(hwndZP, WM_APP+49, nName, 200);
  GlobalDeleteAtom(nName);
}

enum LCDMessageType { TYPE_NONE, TYPE_ATOM, TYPE_INT, TYPE_STATE, TYPE_MODE };

struct LCDMessage {
  WPARAM wParam;                // Code number.
  const char *szName;           // Event suffix.
  LCDMessageType type;          // Data type.
} LCDMessages[] = {
  // Messages as of ZPLCD120.
  { 1000, "State", TYPE_STATE },
  { 1100, "Time", TYPE_ATOM },
  { 1200, "OSD", TYPE_ATOM },
  { 1201, "OSD.Off", TYPE_NONE },
  { 1300, "Mode", TYPE_MODE },
  { 1400, "DVD.Title", TYPE_INT },
  { 1500, "DVD.Chapter", TYPE_INT },
  { 1600, "DVD.Audio", TYPE_ATOM },
  { 1700, "DVD.Sub", TYPE_ATOM },
  { 1800, "File", TYPE_ATOM },
  { 1900, "Position", TYPE_ATOM },
  { 2000, "Resolution", TYPE_ATOM },
  { 2100, "FrameRate", TYPE_ATOM },
  { 2200, "AspectRatio", TYPE_ATOM },
};

#define countof(x) sizeof(x)/sizeof(x[0])

void ZoomPlayerLCD(WPARAM wParam, LPARAM lParam)
{
  for (int i = 0; i < countof(LCDMessages); i++) {
    if (wParam == LCDMessages[i].wParam) {
      char szEvent[128], szPayload[256];
      strcpy(szEvent, "ZoomPlayer.LCD.");
      strcat(szEvent, LCDMessages[i].szName);
      PCHAR pszPayload = szPayload;
      *pszPayload++ = 1;
      *pszPayload = '\0';       // Start empty in case not filled.
      switch (LCDMessages[i].type) {
      case TYPE_ATOM:
        GlobalGetAtomName((ATOM)lParam, pszPayload, sizeof(szPayload)-2);
        break;
      case TYPE_INT:
        sprintf(pszPayload, "%d", lParam);
        break;
      case TYPE_STATE:
        switch (lParam) {
        case 0:
          strcpy(pszPayload, "Closed");
          break;
        case 1:
          strcpy(pszPayload, "Stopped");
          break;
        case 2:
          strcpy(pszPayload, "Paused");
          break;
        case 3:
          strcpy(pszPayload, "Playing");
          break;
        }
        break;
      case TYPE_MODE:
        switch (lParam) {
        case 0:
          strcpy(pszPayload, "DVD");
          break;
        case 1:
          strcpy(pszPayload, "Media");
          break;
        }
        break;
      }
      GirderEvent(szEvent, szPayload, strlen(pszPayload) + 2);
      return;
    }
  }
}

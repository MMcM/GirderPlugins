/* Zoom Player LCD interface */

#include "stdafx.h"
#include "plugin.h"

static HWND g_hwndSpy, g_hwndZP;

// We initiate this process by seeing the creation of the ZoomPlayer main window.
// At that time, we cannot yet send the rendezvous message.  We have to wait for
// things to settle down a little bit first.  1sec seems enough; .5 sec does not.
// Since Zoom Player remembers the name in its registry settings, this only really
// needs to work once right after installing.
DWORD WINAPI ZoomPlayerInitThread(LPVOID param)
{
  Sleep(1000);

  char szName[128];
  GetWindowText(g_hwndSpy, szName, sizeof(szName));
  ATOM nName = GlobalAddAtom(szName);
  // Tell ZoomPlayer to send LCD messages to our monitor window.
  SendMessage(g_hwndZP, WM_APP+49, nName, 200);
  GlobalDeleteAtom(nName);

  return 0;
}

void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow)
{
  g_hwndSpy = hMonitorWindow;
  g_hwndZP = (HWND)strtoul(data, NULL, 16);

  DWORD dwThreadId;
  HANDLE hThread = CreateThread(NULL, 0, ZoomPlayerInitThread, NULL, 0, &dwThreadId);
  CloseHandle(hThread);
}

enum LCDMessageType { TYPE_NONE, TYPE_ATOM, TYPE_INT, TYPE_STATE, TYPE_MODE };

struct LCDMessage {
  WPARAM wParam;                // Code number.
  const char *szName;           // Event suffix.
  LCDMessageType type;          // Data type.
} LCDMessages[] = {
  // Messages as of ZPLCD130.
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

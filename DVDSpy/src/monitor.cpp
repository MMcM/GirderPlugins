/* Monitor thread 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

#ifdef _DEBUG
#define _TRACE
#endif

HANDLE g_hMonitorThread = NULL;
DWORD g_dwMonitorThreadId = 0;
HWND g_hMonitorWindow = NULL;
BOOL g_bRunning = FALSE, g_bStartHook = TRUE, g_bOleInited = FALSE;

void GirderEvent(PCHAR event, PCHAR payload, size_t pllen)
{
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "Girder event: '%s' pld='%s'.\n", event, payload);
    OutputDebugString(dbuf);
  }
#endif
  SF.send_event(event, payload, pllen, PLUGINNUM);
}

size_t GetDVDPayload(char cd, PCHAR payload)
{
  char cdpath[4], *pb;

  DWORD volser = 0;
  ULONGLONG discid = 0;
  char volnam[MAX_PATH] = {'\0'};
  const char *aspect, *display, *standard;
  char regions[9];
  BOOL attribs;

  cdpath[0] = cd;
  cdpath[1] = ':';
  cdpath[2] = '\\';
  cdpath[3] = '\0';
  GetDVDTitle(cdpath, volnam, sizeof(volnam), &volser);
  if (volser == 0) return 0;
  attribs = GetDVDAttribs(cdpath, aspect, display, standard, regions);
  if (attribs) {
    if (!g_bOleInited)
      CoInitialize(NULL);
    GetDVDDiscID(cdpath, &discid);
  }

  pb = payload;
  *pb++ = ((attribs) ? 7 : 2);
  strcpy(pb, volnam);
  pb += strlen(pb) + 1;
  sprintf(pb, "%08X", volser);
  pb += strlen(pb) + 1;
  if (attribs) {
    strcpy(pb, aspect);
    pb += strlen(pb) + 1;
    strcpy(pb, display);
    pb += strlen(pb) + 1;
    strcpy(pb, standard);
    pb += strlen(pb) + 1;
    strcpy(pb, regions);
    pb += strlen(pb) + 1;
    sprintf(pb, "%016I64X", discid);
    pb += strlen(pb) + 1;
  }

  return (pb - payload);
}

const UINT CONTENTS_TIMER = 1001;

LRESULT CALLBACK MonitorWindow(HWND hwnd,  UINT uMsg, 
                               WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_CREATE:  
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window started.\n");
    OutputDebugString(dbuf);
  }
#endif
    break;	
  case WM_DESTROY: 
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window ended.\n");
    OutputDebugString(dbuf);
  }
#endif
    PostQuitMessage(0); 
    break;
  case WM_TIMER:
    if (CONTENTS_TIMER == wParam) {
      KillTimer(hwnd, wParam);

      DWORD dwMask = GetLogicalDrives();
      for (int i = 0; i < 26; i++) {
        if (dwMask & (1<<i)) {
          char cd = 'A' + i;
          char cdpath[4];

          cdpath[0] = cd;
          cdpath[1] = ':';
          cdpath[2] = '\\';
          cdpath[3] = '\0';
          if (DRIVE_CDROM != GetDriveType(cdpath))
            continue;
        
          char event[16], payload[1024], *pend;
          size_t pllen = 0;
          strcpy(event, "Disc.Contents.");
          pend = event + strlen(event);
          *pend++ = cd;
          *pend++ = '\0';
          pllen = GetDVDPayload(cd, payload);
          if (pllen > 0)
            GirderEvent(event, payload, pllen);
        }
      }

      return 0;
    }
    break;
  case WM_DEVICECHANGE:
    {
#ifdef _TRACE
      {
        char dbuf[1024];
        sprintf(dbuf, "WM_DEVICECHANGE received: wParam=%X.\n", wParam);
        OutputDebugString(dbuf);
      }
#endif
      if ((DBT_DEVICEARRIVAL != wParam) && (DBT_DEVICEREMOVECOMPLETE != wParam))
        break;
      PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
#ifdef _TRACE
      {
        char dbuf[1024];
        sprintf(dbuf, "PDEV_BROADCAST_HDR.dbch_devicetype=%X.\n", lpdb->dbch_devicetype);
        OutputDebugString(dbuf);
      }
#endif
      if (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME)
        break;
      PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
#ifdef _TRACE
      {
        char dbuf[1024];
        sprintf(dbuf, "PDEV_BROADCAST_VOLUME.dbcv_flags=%X  .dbcv_unitmask=%X.\n", lpdbv->dbcv_flags, lpdbv->dbcv_unitmask);
        OutputDebugString(dbuf);
      }
#endif
      if (!(lpdbv->dbcv_flags & DBTF_MEDIA))
        break;
      for (int i = 0; i < 26; i++) {
        if (lpdbv->dbcv_unitmask & (1<<i)) {
          char cd = 'A' + i;
          char event[16], payload[1024], *pend;
          size_t pllen = 0;
          switch(wParam) {
          case DBT_DEVICEARRIVAL:
            strcpy(event, "Disc.Insert.");
            pend = event + strlen(event);
            *pend++ = cd;
            *pend++ = '\0';
            pllen = GetDVDPayload(cd, payload);
            GirderEvent(event, payload, pllen);
            break;
          case DBT_DEVICEREMOVECOMPLETE:
            strcpy(event, "Disc.Eject.");
            pend = event + strlen(event);
            *pend++ = cd;
            *pend++ = '\0';
            GirderEvent(event);
            break;
          }
        }
      }
      return TRUE;
    }
  case WM_COPYDATA:
    {
      // This allows another process to cause an event to come from this plug-in.
      PCOPYDATASTRUCT pcd = (PCOPYDATASTRUCT)lParam;
      LPSTR event = (LPSTR)pcd->lpData;
      size_t elen = strlen(event) + 1;
      LPSTR payload = NULL;
      size_t pllen = pcd->cbData - elen;
      if (pllen > 0)
        payload = event + elen;
      GirderEvent(event, payload, pllen);
      return 0;
    }
  case WM_APP+444:
    ZoomPlayerLCD(wParam, lParam);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL SpecialEvent(LPCSTR szEvent, LPCSTR szVal)
{
  if (!strcmp(szEvent, "ZoomPlayer.Init")) {
    ZoomPlayerInit(szVal, g_hMonitorWindow);
    return TRUE;                // Don't send to Girder.
  }
  else if (!strcmp(szEvent, "Eugenes.Init")) {
    EugenesInit(szVal, g_hMonitorWindow);
    return TRUE;
  }
  else if (!strcmp(szEvent, "Eugenes.Close")) {
    EugenesClose();
    return FALSE;               // Also send to Girder.
  }
  return FALSE;
}

DWORD WINAPI MonitorThread(LPVOID param)
{
  g_bOleInited = FALSE;

#define WCNAME "Girder DVDSpy Monitor Window"

  WNDCLASS wclass;
  memset(&wclass, 0, sizeof(wclass));
  wclass.lpfnWndProc = MonitorWindow;
  wclass.hInstance = g_hInstance;
  wclass.lpszClassName = WCNAME;
  RegisterClass(&wclass);

  g_hMonitorWindow = CreateWindow(WCNAME, WCNAME, 0, 0, 0, 0, 0, 0, 0, g_hInstance, NULL);

  // Check current disc contents in a second.
  SetTimer(g_hMonitorWindow, CONTENTS_TIMER, 1000, NULL);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (WM_NEWDISPLAY == msg.message) {
      size_t nMatch, nIndex;
      char szVal[512];
      PCHAR pszVal = szVal;
      *pszVal++ = 1;
      while (DS_GetNext(&nMatch, &nIndex, pszVal, sizeof(szVal)-1)) {
        char szEvent[256];
        DS_GetName(nMatch, nIndex, szEvent, sizeof(szEvent));
        if (!SpecialEvent(szEvent, pszVal))
          GirderEvent(szEvent, szVal, strlen(pszVal) + 2);
      }
    }
    else
      DispatchMessage(&msg);
  }

  if (g_bOleInited)
    CoUninitialize();

  return 0;
}

BOOL StartMonitor()
{
  if (g_bRunning) 
    return TRUE;

  g_hMonitorThread = CreateThread(NULL, 0, MonitorThread, NULL, 0, &g_dwMonitorThreadId);
  if (NULL == g_hMonitorThread) {
    MessageBox(0, "Cannot create hookthread.", "Error", MB_OK);
    return FALSE;
  }

  g_bStartHook = TRUE;
  HKEY hkey;
  if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, 
                                  "Software\\Girder3\\HardPlugins\\DVDSpy", 
                                  &hkey)) {
    char buf[32];
    DWORD dtype, len;
    len = sizeof(buf);
    if (ERROR_SUCCESS == RegQueryValueEx(hkey, "StartHook", NULL, 
                                         &dtype, (LPBYTE)buf, &len)) {
      switch (dtype) {
      case REG_SZ:
        g_bStartHook = !_stricmp(buf, "True");
        break;
      case REG_DWORD:
        g_bStartHook = !!*(DWORD*)buf;
        break;
      }
    }
    RegCloseKey(hkey);
  }
  
  if (g_bStartHook)
    DS_StartHook(g_dwMonitorThreadId);

  g_bRunning = TRUE;

  return TRUE;
}

void StopMonitor()
{
  if (!g_bRunning)
    return;

  if (g_bStartHook)
    DS_EndHook(g_dwMonitorThreadId);

  if (NULL != g_hMonitorWindow)
    SendMessage(g_hMonitorWindow, WM_DESTROY, 0, 0);
  else
    while (!PostThreadMessage(g_dwMonitorThreadId, WM_QUIT, 0, 0))
      Sleep(100);
  WaitForSingleObject(g_hMonitorThread, INFINITE);
  CloseHandle(g_hMonitorThread);

  g_bRunning = FALSE;
}

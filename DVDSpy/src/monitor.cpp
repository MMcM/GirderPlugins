/* Monitor thread */

#include "stdafx.h"
#include "plugin.h"

HANDLE g_hHookThread = NULL;
DWORD g_dwHookThreadId = 0;
HWND g_hMonitorWindow = NULL;
BOOL g_bRunning = FALSE;

void GirderEvent(PCHAR event, PCHAR payload, size_t pllen)
{
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "Girder event: '%s' pld='%s'.\n", event, payload);
    OutputDebugString(dbuf);
  }
#endif
  SF.send_event(event, payload, pllen, PLUGINNUM);
}

LRESULT CALLBACK MonitorWindow(HWND hwnd,  UINT uMsg, 
                               WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_CREATE:  
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window started.\n");
    OutputDebugString(dbuf);
  }
#endif
    break;	
  case WM_DESTROY: 
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window ended.\n");
    OutputDebugString(dbuf);
  }
#endif
    PostQuitMessage(0); 
    break;
  case WM_DEVICECHANGE:
    {
#ifdef _DEBUG
      {
        char dbuf[1024];
        sprintf(dbuf, "WM_DEVICECHANGE received: wParam=%X.\n", wParam);
        OutputDebugString(dbuf);
      }
#endif
      if ((DBT_DEVICEARRIVAL != wParam) && (DBT_DEVICEREMOVECOMPLETE != wParam))
        break;
      PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
#ifdef _DEBUG
      {
        char dbuf[1024];
        sprintf(dbuf, "PDEV_BROADCAST_HDR.dbch_devicetype=%X.\n", lpdb->dbch_devicetype);
        OutputDebugString(dbuf);
      }
#endif
      if (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME)
        break;
      PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
#ifdef _DEBUG
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
          char event[16], payload[1024], *pb;
          DWORD volser = 0;
          char volnam[MAX_PATH] = {'\0'};
          const char *aspect, *display, *standard;
          BOOL attribs;
          switch(wParam) {
          case DBT_DEVICEARRIVAL:
            event[0] = cd;
            event[1] = ':';
            event[2] = '\\';
            event[3] = '\0';
            GetDVDTitle(event, volnam, sizeof(volnam), &volser);
            attribs = GetDVDVideoAttribs(event, &aspect, &display, &standard);
            strcpy(event, "Disc.Insert.");
            pb = event + strlen(event);
            *pb++ = cd;
            *pb++ = '\0';
            pb = payload;
            *pb++ = ((attribs) ? 5 : 2);
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
            }
            GirderEvent(event, payload, pb - payload);
            break;
          case DBT_DEVICEREMOVECOMPLETE:
            strcpy(event, "Disc.Eject.");
            pb = event + strlen(event);
            *pb++ = cd;
            *pb++ = '\0';
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
    return TRUE;
  }
  return FALSE;
}

DWORD WINAPI HookThread(LPVOID param)
{
#define WCNAME "Girder DVDSpy Monitor Window"

  WNDCLASS wclass;
  memset(&wclass, 0, sizeof(wclass));
  wclass.lpfnWndProc = MonitorWindow;
  wclass.hInstance = g_hInstance;
  wclass.lpszClassName = WCNAME;
  RegisterClass(&wclass);

  g_hMonitorWindow = CreateWindow(WCNAME, WCNAME, 0, 0, 0, 0, 0, 0, 0, g_hInstance, NULL);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (WM_NEWDISPLAY == msg.message) {
      size_t nMatch, nIndex;
      char szVal[129];
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
  return 0;
}

BOOL StartMonitor()
{
  if (g_bRunning) 
    return TRUE;

  g_hHookThread = CreateThread(NULL, 0, HookThread, NULL, 0, &g_dwHookThreadId);
  if (NULL == g_hHookThread) {
    MessageBox(0, "Cannot create hookthread.", "Error", MB_OK);
    return FALSE;
  }

  DS_StartHook(g_dwHookThreadId);

  g_bRunning = TRUE;

  return TRUE;
}

void StopMonitor()
{
  if (!g_bRunning)
    return;

  DS_EndHook(g_dwHookThreadId);

  if (NULL != g_hMonitorWindow)
    SendMessage(g_hMonitorWindow, WM_DESTROY, 0, 0);
  else
    while (!PostThreadMessage(g_dwHookThreadId, WM_QUIT, 0, 0))
      Sleep(100);
  WaitForSingleObject(g_hHookThread, INFINITE);
  CloseHandle(g_hHookThread);

  g_bRunning = FALSE;
}

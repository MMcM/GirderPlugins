/* Eugene's DVD interface 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

#ifdef _DEBUG
#define _TRACE
#endif

struct DVD_INFO_STRUCT {
  char szDiskName[16];
  char szCurrentTime[8];
  char szTotalTime[8];
  WORD wTitle;
  WORD wChapter;
};

struct DVD_INFO_STRUCT_EX {
  char szDiskName[17];
  char szCurrentTime[9];
  char szTotalTime[9];
  char szStatus[10];
  WORD wTitle;
  WORD wChapter;
} g_lastInfo;

void StringEvent(PCHAR szEvent, PCHAR szData)
{
  char buf[512];
  buf[0] = 1;
  strncpy(buf+1, szData, sizeof(buf)-1);
  GirderEvent(szEvent, buf, strlen(buf)+2);
}

void IntEvent(PCHAR szEvent, int nData)
{
  char buf[32];
  sprintf(buf, "%d", nData);
  StringEvent(szEvent, buf);
}

LRESULT CALLBACK EugenesWindow(HWND hwnd,  UINT uMsg, 
                               WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_CREATE:  
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy Eugene's window started.\n");
    OutputDebugString(dbuf);
  }
#endif
    break;	
  case WM_DESTROY: 
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy Eugene's window ended.\n");
    OutputDebugString(dbuf);
  }
#endif
    break;
  case WM_COPYDATA:
    {
      PCOPYDATASTRUCT pcd = (PCOPYDATASTRUCT)lParam;
      if (pcd->cbData == sizeof(DVD_INFO_STRUCT)) {
        DVD_INFO_STRUCT *pInfo = (DVD_INFO_STRUCT *)pcd->lpData;	
        char buffer[20];        // Copy strings here and force null termination.

        memcpy(buffer, pInfo->szDiskName, sizeof(pInfo->szDiskName));
        buffer[sizeof(pInfo->szDiskName)] = 0;
        if (strcmp(buffer, g_lastInfo.szDiskName)) {
          StringEvent("Eugenes.Name", buffer);
          strcpy(g_lastInfo.szDiskName, buffer);
        }
			
        memcpy(buffer, pInfo->szCurrentTime, sizeof(pInfo->szCurrentTime));
        buffer[sizeof(pInfo->szCurrentTime)] = 0;
        if (strcmp(buffer, g_lastInfo.szCurrentTime)) {
          StringEvent("Eugenes.Elapsed", buffer);
          strcpy(g_lastInfo.szCurrentTime, buffer);
        }
			
        memcpy(buffer, pInfo->szTotalTime, sizeof(pInfo->szTotalTime));
        buffer[sizeof(pInfo->szTotalTime)] = 0;
        if (strcmp(buffer, g_lastInfo.szTotalTime)) {
          StringEvent("Eugenes.Duration", buffer);
          strcpy(g_lastInfo.szTotalTime, buffer);
        }

        if (pInfo->wTitle != g_lastInfo.wTitle) {
          IntEvent("Eugenes.TitleNo", pInfo->wTitle);
          g_lastInfo.wTitle = pInfo->wTitle;
        }

        if (pInfo->wChapter != g_lastInfo.wChapter) {
          IntEvent("Eugenes.Chapter", pInfo->wChapter);
          g_lastInfo.wChapter = pInfo->wChapter;
        }
      }
      //This part is not supported. Just a place holder for eventual new implementation
      else if (pcd->cbData == sizeof(DVD_INFO_STRUCT_EX)) {
        DVD_INFO_STRUCT_EX *pInfo = (DVD_INFO_STRUCT_EX *)pcd->lpData;
        if (strcmp(pInfo->szDiskName, g_lastInfo.szDiskName))
          StringEvent("Eugenes.Name", pInfo->szDiskName);
        if (strcmp(pInfo->szCurrentTime, g_lastInfo.szCurrentTime))
          StringEvent("Eugenes.Elapsed", pInfo->szCurrentTime);
        if (strcmp(pInfo->szTotalTime, g_lastInfo.szTotalTime))
          StringEvent("Eugenes.Duration", pInfo->szTotalTime);
        if (strcmp(pInfo->szStatus, g_lastInfo.szStatus))
          StringEvent("Eugenes.Status", pInfo->szTotalTime);
        if (pInfo->wTitle != g_lastInfo.wTitle)
          IntEvent("Eugenes.TitleNo", pInfo->wTitle);
        if (pInfo->wChapter != g_lastInfo.wChapter)
          IntEvent("Eugenes.Chapter", pInfo->wChapter);

        g_lastInfo = *pInfo;
      }
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static BOOL g_bClassInit = FALSE;
static HWND g_hwndMonChild = NULL;

void EugenesInit(LPCSTR data, HWND hwndMonitor)
{
  memset(&g_lastInfo, 0, sizeof(g_lastInfo));

  HWND hwndPlayer = (HWND)strtoul(data, NULL, 16);

#define WCNAME "Girder DVDSpy Eugene's Window"

  if (!g_bClassInit) {
    WNDCLASS wclass;
    memset(&wclass, 0, sizeof(wclass));
    wclass.lpfnWndProc = EugenesWindow;
    wclass.hInstance = g_hInstance;
    wclass.lpszClassName = WCNAME;
    RegisterClass(&wclass);
    g_bClassInit = TRUE;
  }

  if (NULL == g_hwndMonChild)
    g_hwndMonChild = CreateWindow(WCNAME, WCNAME, 0, 0, 0, 0, 0, hwndMonitor, NULL, 
                                  g_hInstance, NULL);

  UINT nMsg = RegisterWindowMessage("WM_SUBSCRIBEINFO");
  PostMessage(hwndPlayer, nMsg, (WPARAM)g_hwndMonChild, (LPARAM)0);
}

void EugenesClose()
{
  // Since the player is closing, there is no real point in sending
  // WM_UNSUBSCRIBEINFO to it.

  if (NULL != g_hwndMonChild) {
    SendMessage(g_hwndMonChild, WM_DESTROY, 0, 0);
    g_hwndMonChild = NULL;
  }
}

// DVDSpy.cpp : Interface to Girder via DVDSpy
#include "stdafx.h"
#include "DVDSpy.h"

HWND g_dvdspy = NULL;
BOOL g_checked = FALSE;

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

BOOL FindDVDSpy()
{
  // TODO: Consider checking more often and doing an IsWindow on any
  // previous handle.
  if (!g_checked) {
    EnumWindows(FindMonitorWindow, (LPARAM)&g_dvdspy);
    ATLTRACE2(atlTraceUser, 3, _T("DVDSpy window %X\n"), (DWORD)g_dvdspy);
    g_checked = TRUE;
  }
  return (NULL != g_dvdspy);
}

size_t copyBSTR(LPSTR into, BSTR bstr, size_t size)
{
  return WideCharToMultiByte(CP_ACP, 0, bstr, SysStringLen(bstr), into, size, 
                             NULL, NULL);
}

void GirderEvent(BSTR event, BSTR pld)
{
  ATLTRACE2(atlTraceUser2, 3, _T("Event=%S; Payload=%S\n"), event, pld);

  if (!FindDVDSpy()) return;

  char buf[1024];
  LPSTR pbuf = buf;

  pbuf += copyBSTR(pbuf, event, sizeof(buf) - (pbuf - buf));
  *pbuf++ = '\0';
  if (NULL != pld) {
    *pbuf++ = 1;
    pbuf += copyBSTR(pbuf, pld, sizeof(buf) - (pbuf - buf));
    *pbuf++ = '\0';
  }

  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = buf;
  cd.cbData = pbuf - buf;
  SendMessage(g_dvdspy, WM_COPYDATA, 0, (LPARAM)&cd);
}

void GirderEvent(LPCSTR event, BSTR pld)
{
  ATLTRACE2(atlTraceUser2, 3, _T("Event=%s; Payload=%S\n"), event, pld);

  if (!FindDVDSpy()) return;

  char buf[1024];
  LPSTR pbuf = buf;

  strncpy(pbuf, event, sizeof(buf));
  pbuf += strlen(pbuf) + 1;
  if (NULL != pld) {
    *pbuf++ = 1;
    pbuf += copyBSTR(pbuf, pld, sizeof(buf) - (pbuf - buf));
    *pbuf++ = '\0';
  }

  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = buf;
  cd.cbData = pbuf - buf;
  SendMessage(g_dvdspy, WM_COPYDATA, 0, (LPARAM)&cd);
}

void GirderEvent(LPCSTR event, LPCSTR pld)
{
  ATLTRACE2(atlTraceUser2, 3, _T("Event=%s; Payload=%s\n"), event, pld);

  if (!FindDVDSpy()) return;

  char buf[1024];
  LPSTR pbuf = buf;

  strncpy(pbuf, event, sizeof(buf));
  pbuf += strlen(pbuf) + 1;
  if (NULL != pld) {
    *pbuf++ = 1;
    strncpy(pbuf, pld, sizeof(buf) - (pbuf - buf));
    pbuf += strlen(pbuf) + 1;  
  }

  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = buf;
  cd.cbData = pbuf - buf;
  SendMessage(g_dvdspy, WM_COPYDATA, 0, (LPARAM)&cd);
}

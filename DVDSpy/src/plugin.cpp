/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.0 Plugin                                                                  */
/*  DLL implementation                                                                 */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbt.h>

#define GIRDER_CPP
#include "plugin.h"
#include "ui.h"

#include "DisplaySpyHook.h"

HANDLE ConfigThreadHandle=NULL;
HANDLE LearnThreadHandle=NULL;
HANDLE HookThreadHandle=NULL;
DWORD HookThreadId=0;
HWND hMonitorWindow=NULL;
bool Running=FALSE;

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

LRESULT CALLBACK MonitorWindow(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
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
          switch(wParam) {
          case DBT_DEVICEARRIVAL:
            event[0] = cd;
            event[1] = ':';
            event[2] = '\\';
            event[3] = '\0';
            GetDVDTitle(event, volnam, sizeof(volnam), &volser);
            strcpy(event, "Disc.Insert.");
            pb = event + strlen(event);
            *pb++ = cd;
            *pb++ = '\0';
            pb = payload;
            *pb++ = 2;
            strcpy(pb, volnam);
            pb += strlen(pb) + 1;
            sprintf(pb, "%08X", volser);
            pb += strlen(pb) + 1;
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
    ZoomPlayerInit(szVal, hMonitorWindow);
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
  wclass.hInstance = hInstance;
  wclass.lpszClassName = WCNAME;
  RegisterClass(&wclass);

  hMonitorWindow = CreateWindow(WCNAME, WCNAME, 0, 0, 0, 0, 0, 0, 0, hInstance, NULL);

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


extern "C" int WINAPI gir_learn_event(char * old ,char * newevent, int len )
{
  DWORD dwThreadId;
   
  if (hLearnDialog != 0) {
    SetForegroundWindow(hLearnDialog);
    SendMessage(hLearnDialog, WM_SELECT_EVENT, 0, (LPARAM)old);
  }
  else {
    LearnThreadHandle=CreateThread(NULL,0,&LearnThread,old,0,&dwThreadId);
    if (LearnThreadHandle==0)
      MessageBox(0, "Cannot create dialogthread.", "Error", MB_OK);
  }
  return GIR_ASYNC_LEARN;
}

extern "C" int WINAPI gir_start()
{
  if ( Running ) 
    return TRUE;

  HookThreadHandle = CreateThread(NULL, 0, HookThread, NULL, 0, &HookThreadId);
  if (NULL == HookThreadHandle) {
    MessageBox(0, "Cannot create hookthread.", "Error", MB_OK);
    return FALSE;
  }

  DS_StartHook(HookThreadId);

  Running=TRUE;

  return GIR_TRUE;
}

extern "C" int WINAPI gir_stop()
{
  if ( !Running )
    return TRUE;

  DS_EndHook(HookThreadId);

  if (NULL != hMonitorWindow)
    SendMessage(hMonitorWindow, WM_DESTROY, 0, 0);
  else
    while (!PostThreadMessage(HookThreadId, WM_QUIT, 0, 0))
      Sleep(100);
  WaitForSingleObject(HookThreadHandle, INFINITE);
  CloseHandle(HookThreadHandle);

  Running=FALSE;

  return GIR_TRUE;
}



extern "C" void WINAPI gir_config()
{

  DWORD dwThreadId;
   
  if (hConfigDialog != 0) {
    SetForegroundWindow(hConfigDialog);
  }
  else {
    ConfigThreadHandle=CreateThread(NULL,0,&ConfigThread,NULL,0,&dwThreadId);
    if (ConfigThreadHandle==0)
      MessageBox(0, "Cannot create dialogthread.", "Error", MB_OK);
  }

  
}


extern "C" int WINAPI  gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI  gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" void WINAPI  gir_description(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "DVD Display Spy.  Written by Mike McMahon (MMcM).", Length);
}

extern "C" void WINAPI  gir_name(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINNAME, Length);
}

extern "C" void WINAPI  gir_version(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINVERSION, Length);
}

extern "C" int WINAPI gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
{

  p_functions h = (p_functions )	p;
  if ( h->size != sizeof( s_functions ) )
    {
      return GIR_FALSE;
    }
		
  memcpy((void *)&SF, p, sizeof ( s_functions ));

  return GIR_TRUE;

}


/* Called right before Girder unloads the plugin, make 
   sure you cleaned everything up !!

*/

extern "C" int WINAPI gir_close()
{
  /* Make sure the dialogs are closed */
  if (!(hConfigDialog==0))
    {	    
      SendMessage(hConfigDialog, WM_DESTROY,0,0);
      WaitForSingleObject(ConfigThreadHandle, INFINITE);		
      CloseHandle(ConfigThreadHandle);
    }

  if (Running) gir_stop();

  return GIR_TRUE;
}




/* Called by windows */
BOOL WINAPI DllMain( HANDLE hModule, 
                        DWORD fdwreason,  LPVOID lpReserved )
{
	
  switch(fdwreason) {
  case DLL_PROCESS_ATTACH:
    hInstance=HINSTANCE(hModule);
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

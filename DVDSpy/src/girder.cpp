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
#include "girder.h"
#include "ui.h"

#include "DisplaySpyHook.h"

HWND hTargetWindow;
PCHAR pMessageBuffer;
HANDLE hSharedMem;
HANDLE hSMSem;

HANDLE ConfigThreadHandle=NULL;
HANDLE LearnThreadHandle=NULL;
HANDLE HookThreadHandle=NULL;
DWORD HookThreadId=0;
HWND hMonitorWindow=NULL;
bool Running=FALSE;

void GirderEvent(PCHAR event, PCHAR sarg, long iarg)
{
  int nreg = 1;
  if (NULL != sarg) {
    LPCSTR ps = strrchr(event, '.');
    if ((NULL != ps) &&
        ('0' <= *++ps) &&
        ('9' >= *ps))
      nreg = atoi(ps);
  }

#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "Girder event: '%s' reg=%d sarg='%s' iarg=%d.\n", 
            event, nreg, sarg, iarg);
    OutputDebugString(dbuf);
  }
#endif
#if 0
  COPYDATASTRUCT cd;
  cd.lpData=event;
  cd.cbData=strlen(event)+1;
  SendMessage(hTargetWindow, WM_COPYDATA, PLUGINNUM, (LPARAM)&cd);
#else
  if (WaitForSingleObject(hSMSem, 2500)==WAIT_OBJECT_0) {
    if (NULL != sarg) {
      HFunctions.SetGirderStrReg(sarg, nreg);
      if (0 != iarg)
        HFunctions.SetGirderReg(iarg, nreg);
    }
    strncpy(pMessageBuffer, event, 250);
    PostMessage(hTargetWindow, WM_USER+1030, PLUGINNUM, 0);
  }
#ifdef _DEBUG
  else {
    char dbuf[1024];
    sprintf(dbuf, "Could not get Girder semaphore.\n");
    OutputDebugString(dbuf);
  }
#endif
#endif
}

LRESULT CALLBACK MonitorWindow(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_CREATE:  
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window started.");
    OutputDebugString(dbuf);
  }
#endif
    break;	
  case WM_DESTROY: 
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "DVDSpy monitor window ended.");
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
          char buf[256], *pb;
          DWORD volser = 0;
          char volnam[MAX_PATH] = {'\0'};
          switch(wParam) {
          case DBT_DEVICEARRIVAL:
            buf[0] = cd;
            buf[1] = ':';
            buf[2] = '\\';
            buf[3] = '\0';
            GetDVDTitle(buf, volnam, sizeof(volnam), &volser);
            strcpy(buf, "Disc.Insert.");
            pb = buf + strlen(buf);
            *pb++ = cd;
            *pb++ = '.';
            *pb++ = '9';
            *pb++ = '\0';
            GirderEvent(buf, volnam, (long)volser);
            break;
          case DBT_DEVICEREMOVECOMPLETE:
            strcpy(buf, "Disc.Eject.");
            pb = buf + strlen(buf);
            *pb++ = cd;
            *pb++ = '\0';
            GirderEvent(buf, NULL);
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
      size_t elen = strlen(event);
      LPSTR sarg = NULL;
      if (elen + 1 < pcd->cbData)
        sarg = event + elen + 1;
      GirderEvent(event, sarg, pcd->dwData);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
      char szVal[128];
      while (DS_GetNext(&nMatch, &nIndex, szVal, sizeof(szVal))) {
        PCHAR pszVal = szVal;
        char szName[128];
        DS_GetMatchName(nMatch, szName, sizeof(szName));
        int nReg = DS_GetMatchRegister(nMatch);
        char szEvent[256];
        memset(szEvent, 0, sizeof(szEvent));
        strcpy(szEvent, szName);
        LPSTR ep = szEvent + strlen(szEvent);
        if (nReg == 0) {
          pszVal = NULL;
        }
        else {
          sprintf(ep, ".%d", nReg + nIndex);
          ep = ep + strlen(ep);
        }
        *ep++ = '\0';
        GirderEvent(szEvent, pszVal);
      }
    }
    else
      DispatchMessage(&msg);
  }
  return 0;
}

extern "C" int WINAPI h_support_functions(void *sf)
{
  sHFunctions1 *h;

  if ( sf == NULL )
    {
      return 1;                 // we want version 1 hardware support functions
    }
  else
    {
      h = (sHFunctions1 *)sf;
      if ( h->dwSize != sizeof( sHFunctions1 ) )
        {
          return GIR_ERROR;
        }
		
      memcpy((void *)&HFunctions, sf, sizeof ( sHFunctions1));
    }	

  return 0;
}

extern "C" bool WINAPI start_device()
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
  return TRUE;
}

extern "C" bool WINAPI stop_device()
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
  return TRUE;
}


extern "C" bool WINAPI flush_device()
{
  DS_Reset();
  return TRUE;
}


extern "C" bool WINAPI config_device()
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

  return TRUE;
}

extern "C" void WINAPI learn_device(PCHAR old)
{
  DWORD dwThreadId;
   
  if (hLearnDialog != 0) {
    SetForegroundWindow(hLearnDialog);
    SendMessage(hLearnDialog, WM_USER+100, 0, (LPARAM)old);
  }
  else {
    LearnThreadHandle=CreateThread(NULL,0,&LearnThread,old,0,&dwThreadId);
    if (LearnThreadHandle==0)
      MessageBox(0, "Cannot create dialogthread.", "Error", MB_OK);
  }
}



extern "C" void WINAPI identify_device(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "DVD Display Spy", Length);
}

extern "C" void WINAPI support_device(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "DVDSpy", Length);
}

extern "C" void WINAPI version_device(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "1.12", Length);
}

extern "C" bool WINAPI compare_str(PCHAR Orig,PCHAR Comp)
{
  return (strcmp(Orig, Comp)==0);
}

extern "C" void WINAPI init_device(HWND hWindow, TranslateFunct p)
{
  hTargetWindow=hWindow;	
  I18NTranslate=p;
}

void init_dll()
{
  hSharedMem = OpenFileMapping(FILE_MAP_WRITE,FALSE,"GirderFileMap6111976");
  if ( hSharedMem == NULL )
    {
      MessageBox(0, "Cannot open sharedmemory segment", "Error",0);
    }

  hSMSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS	, FALSE, "GirderSemaphore6111976");
  if ( hSMSem == NULL )
    {
      MessageBox(0, "Cannot open Semaphore segment", "Error",0);
    }

  pMessageBuffer = (PCHAR)MapViewOfFile(hSharedMem,FILE_MAP_WRITE,0,0,250);
  if ( pMessageBuffer == NULL )
    {
      MessageBox(0, "Cannot map segment", "Error",0);
    }	

}


/* Return the device number make sure this is a unique one, see file plugins.txt ! */
extern "C" int WINAPI get_device_num()
{
  return PLUGINNUM;
}


/* Girder calls this just before it calls the FreeLibrary */
extern "C" void WINAPI clean_up_device()
{
  /* Make sure the dialogs are closed */
  if (!(hConfigDialog==0))
    {	    
      SendMessage(hConfigDialog, WM_DESTROY,0,0);
      WaitForSingleObject(ConfigThreadHandle, INFINITE);		
      CloseHandle(ConfigThreadHandle);
    }

  if (Running) stop_device();

}



void close_dll()
{
  CloseHandle(hSharedMem);
  CloseHandle(hSMSem);
}



BOOL WINAPI DllMain( HANDLE hModule, DWORD fdwreason,  LPVOID lpReserved )
{
	
  switch(fdwreason) {
  case DLL_PROCESS_ATTACH:
	    
    hInstance=HINSTANCE(hModule);
    init_dll();

    // The DLL is being mapped into process's address space
    //  Do any required initialization on a per application basis, return FALSE if failed
    break;
  case DLL_THREAD_ATTACH:
    // A thread is created. Do any required initialization on a per thread basis
    break;
  case DLL_THREAD_DETACH:
		
    // Thread exits with  cleanup
    break;
  case DLL_PROCESS_DETACH:
    close_dll();



    // The DLL unmapped from process's address space. Do necessary cleanup
    break;
  }
  return TRUE;
}

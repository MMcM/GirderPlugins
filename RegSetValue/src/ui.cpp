/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.0 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "girder.h"

/* Local variables */
HWND hDialog;
HANDLE ConfigThreadHandle;

static
BOOL SaveUISettings(HWND hwnd)
{
  char buf[1024];

  GetWindowText(GetDlgItem(hwnd, IDC_KEY), buf, sizeof(buf));
  ReallocPchar(&(CurCommand->svalue1), buf);

  GetWindowText(GetDlgItem(hwnd, IDC_VALUE), buf, sizeof(buf));
  ReallocPchar(&(CurCommand->svalue2), buf);

  CurCommand->actiontype=PLUGINNUM;
  SetCommand(CurCommand);

  HKEY hkey;
  PCHAR pval = DecodeKeyValue(CurCommand->svalue1, &hkey);
  if (NULL == pval)
    return FALSE;
  LONG rc = RegQueryValueEx(hkey, pval, NULL, NULL, NULL, NULL);
  RegCloseKey(hkey);
  if (ERROR_SUCCESS != rc) {
    sprintf(buf, "Value does not exist: %s", CurCommand->svalue1);
    MessageBox(0, buf, "Error", MB_ICONERROR);
    return FALSE;
  }
  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static
void EmptyUI(HWND hwnd)
{
  SetWindowText(GetDlgItem(hwnd, IDC_KEY), "");
  SetWindowText(GetDlgItem(hwnd, IDC_VALUE), "");
}

static
void LoadUISettings(HWND hwnd)
{
  SetWindowText(GetDlgItem(hwnd, IDC_KEY), CurCommand->svalue1);
  SetWindowText(GetDlgItem(hwnd, IDC_VALUE), CurCommand->svalue2);
}

static
BOOL CALLBACK DialogProc(  HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      PCHAR trans;
      	
      hDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      trans=I18NTranslate("Ok");
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      trans=I18NTranslate("Cancel");
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      trans=I18NTranslate("Apply");
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      trans=I18NTranslate("Key\\Name:");
      SetWindowText(GetDlgItem(hwnd, IDC_KEYL), trans);
	
      trans=I18NTranslate("Value:");
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      LoadUISettings(hwnd);

      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      return 0;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return 0;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (SaveUISettings(hwnd))
        EndDialog(hwnd, TRUE);
      return 1;

    case IDC_APPLY:
      SaveUISettings(hwnd);
      return 1;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return 1;

    case IDC_KEY:
    case IDC_VALUE:
      if ( HIWORD(wParam)==EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;
    }
    break;

  case WM_USER+100:
    LoadUISettings(hwnd);
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
    return 1;

  case WM_USER+101:
    EmptyUI(hwnd);
    return 1;

  case WM_USER+102:
    if (wParam==1) {
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_KEY), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_VALUE), TRUE);
    }
    else {
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_KEY), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_VALUE), FALSE);
    }
    return 1;			
  }
  return 0;
}

static DWORD WINAPI ConfigThread( LPVOID lpParameter )
{
  BOOL fResult;

  fResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, 
                           DialogProc, (LPARAM)lpParameter);

  hDialog=0;
  
  return 0;
}

void Update_Config()
{

  SendMessage(hDialog, WM_USER+100, 0,0);

}

void Empty_Config()
{

  SendMessage(hDialog, WM_USER+101, 0,0);

}

void Enable_Config(BOOL bValue)
{
  if (bValue)
    SendMessage(hDialog, WM_USER+102, 1,0);
  else
    SendMessage(hDialog, WM_USER+102, 0,0);

}

void Show_Config()
{
   DWORD dwThreadId;
   HANDLE ConfigThreadHandle;
   
   if (hDialog != 0)   
   {
      SetForegroundWindow(hDialog);
   }
   else
   {
      ConfigThreadHandle=CreateThread(NULL,0,&ConfigThread,NULL,0,&dwThreadId);
      if (ConfigThreadHandle==0)
         MessageBox(0, "Cannot create dialogthread.", "Error", MB_OK);
    
   }


   return ;
}

void Close_Config()
{
  
  if ( hDialog!=NULL) {

    SendMessage(hDialog, WM_DESTROY,0,0);
    WaitForSingleObject(ConfigThreadHandle,4000);
    CloseHandle(ConfigThreadHandle);

  }
  
}

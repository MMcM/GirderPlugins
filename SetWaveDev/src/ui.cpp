/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.1 Plugin                                                                  */
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

  if (SendMessage(GetDlgItem(hwnd, IDC_RECORD), BM_GETCHECK, 0, 0))
    CurCommand->ivalue1 = 1;
  else
    CurCommand->ivalue1 = 0;
  
  GetWindowText(GetDlgItem(hwnd, IDC_DEVICE), buf, sizeof(buf));
  SF.ReallocPchar(&(CurCommand->svalue1), buf);

  CurCommand->actiontype=PLUGINNUM;
  SF.SetCommand(CurCommand);
  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static
void LoadCombo(HWND hwnd)
{
  HWND combo = GetDlgItem(hwnd, IDC_DEVICE);
  SendMessage(combo, CB_RESETCONTENT, 0, 0);
  for (UINT i = 0; i < waveOutGetNumDevs(); i++) {
    WAVEOUTCAPS caps;
    if (MMSYSERR_NOERROR != waveOutGetDevCaps(i, &caps, sizeof(caps)))
      continue;
    SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)caps.szPname);
  }
}

static
void EmptyUI(HWND hwnd)
{
  SendMessage(GetDlgItem(hwnd, IDC_PLAYBACK), BM_SETCHECK, TRUE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_RECORD), BM_SETCHECK, FALSE, 0);
  LoadCombo(hwnd);
  SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_SETCURSEL, 0, 0);
}

static
void LoadUISettings(HWND hwnd)
{
  SendMessage(GetDlgItem(hwnd, IDC_PLAYBACK), BM_SETCHECK, (CurCommand->ivalue1 == 0), 0);
  SendMessage(GetDlgItem(hwnd, IDC_RECORD), BM_SETCHECK, (CurCommand->ivalue1 == 1), 0);
  LoadCombo(hwnd);
  SetWindowText(GetDlgItem(hwnd, IDC_DEVICE), CurCommand->svalue1);
}

static
BOOL CALLBACK DialogProc(  HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      hDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.I18NTranslateEx("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.I18NTranslateEx("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.I18NTranslateEx("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.I18NTranslateEx("Playback", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_PLAYBACK), trans);

      SF.I18NTranslateEx("Record", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_RECORD), trans);

      SF.I18NTranslateEx("Device:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_DEVICEL), trans);

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

    case IDC_DEVICE:
      if (HIWORD(wParam) == CBN_EDITCHANGE) {
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
      EnableWindow(GetDlgItem(hwnd, IDC_PLAYBACK), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_RECORD), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_DEVICE), TRUE);
    }
    else {
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_PLAYBACK), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_RECORD), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_DEVICE), FALSE);
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

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
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "girder.h"
#include <lcdriver.h>

/* Local variables */
HWND hDialog;
HANDLE ConfigThreadHandle;

DisplayAction DisplayActions[] = {
  { "String", valSTR, DisplayString },
  { "String Register", valINT, DisplayStringRegister },
  { "Current Date/Time", valSTR, DisplayCurrentTime },
  { "Clear Display", valNONE, DisplayClear },
  { "Close Display", valNONE, DisplayClose },
  { "Payload", valINT, DisplayPayload },
  { "Filename Payload", valINT, DisplayFilenamePayload },
};

#define countof(x) (sizeof(x)/sizeof(x[0]))

static
BOOL SaveUISettings(HWND hwnd)
{
  char buf[256];

  CurCommand->ivalue1 = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);

  GetWindowText(GetDlgItem(hwnd, IDC_VALSTR), buf, sizeof(buf));
  SF.ReallocPchar(&(CurCommand->svalue1), buf);

  CurCommand->ivalue2 = SendMessage(GetDlgItem(hwnd, IDC_VAL_SPIN), UDM_GETPOS, 0, 0);

  CurCommand->lvalue1 = SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_GETPOS, 0, 0);
  CurCommand->lvalue2 = SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_GETPOS, 0, 0);
  if (SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_GETCHECK, 0, 0))
    CurCommand->lvalue3 = 0;
  else
    CurCommand->lvalue3 = SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_GETPOS, 0, 0);
  
  CurCommand->actiontype=PLUGINNUM;
  SF.SetCommand(CurCommand);

  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static
void EmptyUI(HWND hwnd)
{
  SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_SETCURSEL, 0, 0);
  SetWindowText(GetDlgItem(hwnd, IDC_VALSTR), "");
  SendMessage(GetDlgItem(hwnd, IDC_VAL_SPIN), UDM_SETPOS, 0, 1);
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), SW_SHOW);
  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), SW_SHOW);
  ShowWindow(GetDlgItem(hwnd, IDC_VALINT), SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), SW_HIDE);
  SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, 0);
  SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, TRUE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, FALSE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETPOS, 0, 0);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), FALSE);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), FALSE);
}

static
void LoadUISettings(HWND hwnd)
{
  SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_SETCURSEL, CurCommand->ivalue1, 0);
  SetWindowText(GetDlgItem(hwnd, IDC_VALSTR), CurCommand->svalue1);
  SendMessage(GetDlgItem(hwnd, IDC_VAL_SPIN), UDM_SETPOS, 0, CurCommand->ivalue2);
  DisplayAction& action = DisplayActions[CurCommand->ivalue1];
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
             (valNONE != action.valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
             (valSTR == action.valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
             (valINT == action.valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
             (valINT == action.valueType) ? SW_SHOW : SW_HIDE);
  SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, CurCommand->lvalue1);
  SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, CurCommand->lvalue2);
  SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, !CurCommand->lvalue3, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, !!CurCommand->lvalue3, 0);
  SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETPOS, 0, CurCommand->lvalue3);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), !!CurCommand->lvalue3);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), !!CurCommand->lvalue3);
}

static
BOOL CALLBACK DialogProc(  HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      hDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.I18NTranslateEx("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.I18NTranslateEx("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.I18NTranslateEx("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.I18NTranslateEx("Type:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_TYPEL), trans);

      SF.I18NTranslateEx("Value:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      SF.I18NTranslateEx("Row:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_ROWL), trans);

      SF.I18NTranslateEx("Column:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_COLL), trans);

      SF.I18NTranslateEx("Rest of line", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_REST), trans);

      SF.I18NTranslateEx("Width:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_WIDTH), trans);

      HWND combo = GetDlgItem(hwnd, IDC_TYPE);
      for (size_t i = 0; i < countof(DisplayActions); i++) {
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)DisplayActions[i].name);
      }

      SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(lcdGetHeight()-1, 0));
      SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(lcdGetWidth()-1, 0));
      SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(lcdGetWidth(), 1));

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

    case IDC_TYPE:
      if ( HIWORD(wParam)==CBN_SELENDOK) {
        int pos = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);
        DisplayAction& action = DisplayActions[pos];
        ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
                   (valNONE != action.valueType) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
                   (valSTR == action.valueType) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
                   (valINT == action.valueType) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
                   (valINT == action.valueType) ? SW_SHOW : SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_VALSTR:
    case IDC_ROW:
    case IDC_COL:
    case IDC_WIDTH:
      if ( HIWORD(wParam)==EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_USE_REST:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), FALSE);
      break;

    case IDC_USE_WIDTH:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), TRUE);
      break;
    }
    break;

  case WM_NOTIFY:
    {
      LPNMHDR phdr = (LPNMHDR)lParam;
      switch (phdr->code) {
      case UDN_DELTAPOS:
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
        break;
      }
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
    }
    else {
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
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

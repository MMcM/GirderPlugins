/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.2 Plugin                                                                  */
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
#include "plugin.h"
#include <lcdriver.h>

/* Local variables */
HWND hDialog;
HANDLE ConfigThreadHandle;

DisplayAction DisplayActions[] = {
  { "s", "String", valSTR, DisplayString },
  { "v", "Variable", valVAR, DisplayVariable },
  { "t", "Current Date/Time", valSTR, DisplayCurrentTime },
  { "f", "Filename Variable", valVAR, DisplayFilename },
  { "x", "Close Display", valNONE, DisplayClose },
  { "c", "Clear Display", valNONE, DisplayClear },
};

#define countof(x) (sizeof(x)/sizeof(x[0]))

DisplayAction *FindDisplayAction(p_command command)
{
  PCHAR key = command->svalue2;
  if ((NULL == key) || ('\0' == *key)) {
    // An older GML file.  Convert the command to the new format.
    PCHAR val = NULL;
    char buf[128];

    switch (command->ivalue1) {
    case 0:                       // String
      key = "s";
      break;
    case 1:                       // String Register
      key = "v";
      sprintf(buf, "treg%d", command->ivalue2);
      val = buf;
      break;
    case 2:                       // Current Date/Time
      key = "t";
      break;
    case 3:                       // Clear Display
      key = "c";
      break;
    case 4:                       // Close Display
      key = "x";
      break;
    case 5:                       // Payload
      key = "v";
      sprintf(buf, "pld%d", command->ivalue2);
      val = buf;
      break;
    case 6:                       // Filename Payload
      key = "f";
      sprintf(buf, "pld%d", command->ivalue2);
      val = buf;
      break;
    default:
      return NULL;
    }
    // Position information.
    command->ivalue1 = command->lvalue1; // Row
    command->ivalue2 = command->lvalue2; // Column
    command->ivalue3 = command->lvalue3; // Width
    if (command->ivalue3 <= 0)
      command->ivalue3 = -1;    // Standardize rest value.
    command->lvalue1 = 0;       // No links.
    command->lvalue2 = 0;
    command->lvalue3 = 0;
    if (NULL != val)
      SF.realloc_pchar(&(command->svalue1), val);
    SF.realloc_pchar(&(command->svalue2), key);
  }

  for (size_t i = 0; i < countof(DisplayActions); i++) {
    if (!strcmp(DisplayActions[i].key, key))
      return DisplayActions + i;
  }
  return NULL;
}

static BOOL SaveUISettings(HWND hwnd)
{
  char buf[256];

  if (CurCommand == NULL)
     return FALSE;

  EnterCriticalSection(&CurCommand->critical_section);

  int idx = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);
  DisplayAction *action = (DisplayAction *)
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
  if (NULL != action)
    SF.realloc_pchar(&(CurCommand->svalue2), (PCHAR)action->key);
  else
    SF.realloc_pchar(&(CurCommand->svalue2), "?");

  if ((NULL != action) && (valINT == action->valueType))
    GetWindowText(GetDlgItem(hwnd, IDC_VALINT), buf, sizeof(buf));
  else
    GetWindowText(GetDlgItem(hwnd, IDC_VALSTR), buf, sizeof(buf));
  SF.realloc_pchar(&(CurCommand->svalue1), buf);

  CurCommand->ivalue1 = SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_GETPOS, 0, 0);
  if (SendMessage(GetDlgItem(hwnd, IDC_USE_WRAP), BM_GETCHECK, 0, 0))
    CurCommand->ivalue2 = -1;
  else
    CurCommand->ivalue2 = SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_GETPOS, 0, 0);
  if (SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_GETCHECK, 0, 0))
    CurCommand->ivalue3 = -1;
  else
    CurCommand->ivalue3 = SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_GETPOS, 0, 0);
  
  CurCommand->actiontype = PLUGINNUM;
  SF.set_command(CurCommand);

  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  LeaveCriticalSection(&CurCommand->critical_section);

  return TRUE;
}
     
static void EmptyUI(HWND hwnd)
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
  SendMessage(GetDlgItem(hwnd, IDC_USE_WRAP), BM_SETCHECK, FALSE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_COL), BM_SETCHECK, TRUE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, TRUE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, FALSE, 0);
  SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETPOS, 0, 0);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), FALSE);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), FALSE);
}

static void LoadUISettings(HWND hwnd)
{
  if (CurCommand == NULL)
     return;

  EnterCriticalSection(&CurCommand->critical_section);

  DisplayAction *action = FindDisplayAction(CurCommand);
  if (NULL == action)
    action = DisplayActions;    // String

  for (int idx = 0; idx < countof(DisplayActions); idx++) {
    DisplayAction *itemAction = (DisplayAction *)
      SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
    if (itemAction == action) {
      SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_SETCURSEL, idx, 0);      
      break;
    }
  }
  
  if (valINT == action->valueType)
    SetWindowText(GetDlgItem(hwnd, IDC_VALINT), CurCommand->svalue1);
  else
    SetWindowText(GetDlgItem(hwnd, IDC_VALSTR), CurCommand->svalue1);

  char trans[256];
  SF.i18n_translate((valVAR == action->valueType) ? "Variable:" : "Value:",
                    trans, sizeof(trans));
  SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
             (valNONE != action->valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
             ((valSTR == action->valueType) || (valVAR == action->valueType)) ?
              SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
             (valINT == action->valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
             (valINT == action->valueType) ? SW_SHOW : SW_HIDE);

  SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, CurCommand->ivalue1);

  BOOL wrap = (CurCommand->ivalue2 < 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_WRAP), BM_SETCHECK, wrap, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_COL), BM_SETCHECK, !wrap, 0);
  SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, CurCommand->ivalue2);
  EnableWindow(GetDlgItem(hwnd, IDC_COL), !wrap);
  EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), !wrap);

  BOOL rest = (wrap || (CurCommand->ivalue3 <= 0));
  EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), !wrap);
  EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), !wrap);
  SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, rest, 0);
  SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, !rest, 0);
  SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETPOS, 0, CurCommand->ivalue3);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), !rest);
  EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), !rest);

  LeaveCriticalSection(&CurCommand->critical_section);
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
			
      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.i18n_translate("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.i18n_translate("Type:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_TYPEL), trans);

      SF.i18n_translate("Value:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      SF.i18n_translate("Row:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_ROWL), trans);

      SF.i18n_translate("Marquee", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_WRAP), trans);

      SF.i18n_translate("Column:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_COL), trans);

      SF.i18n_translate("Rest of line", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_REST), trans);

      SF.i18n_translate("Width:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_WIDTH), trans);

      HWND combo = GetDlgItem(hwnd, IDC_TYPE);
      for (size_t i = 0; i < countof(DisplayActions); i++) {
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)DisplayActions[i].name);
        SendMessage(combo, CB_SETITEMDATA, idx, (LPARAM)(DisplayActions+i));
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
      if (HIWORD(wParam) == CBN_SELENDOK) {
        int idx = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);
        DisplayAction *action = (DisplayAction *)
          SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
        if (NULL == action) break;
        char trans[256];
        SF.i18n_translate((valVAR == action->valueType) ? "Variable:" : "Value:",
                          trans, sizeof(trans));
        SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);
        ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
                   (valNONE != action->valueType) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
                   ((valSTR == action->valueType) || (valVAR == action->valueType)) ?
                   SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
                   (valINT == action->valueType) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
                   (valINT == action->valueType) ? SW_SHOW : SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_VALSTR:
    case IDC_ROW:
    case IDC_COL:
    case IDC_WIDTH:
      if (HIWORD(wParam) == EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_USE_WRAP:
      EnableWindow(GetDlgItem(hwnd, IDC_COL), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), FALSE);
      SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, TRUE, 0);
      SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, FALSE, 0);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), FALSE);
      /* falls through */
    case IDC_USE_REST:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), FALSE);
      break;

    case IDC_USE_COL:
      EnableWindow(GetDlgItem(hwnd, IDC_COL), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), TRUE);
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
    SendMessage(hDialog, WM_USER+102, 1, 0);
  else
    SendMessage(hDialog, WM_USER+102, 0, 0);
}

void Show_Config()
{
  if (hDialog != NULL) {
    SetForegroundWindow(hDialog);
  }
  else {
    DWORD dwThreadId;
    ConfigThreadHandle = CreateThread(NULL,0,&ConfigThread,NULL,0,&dwThreadId);
    if (ConfigThreadHandle == NULL)
      MessageBox(0, "Cannot create dialogthread.", "Error", MB_OK);
  }
}

void Close_Config()
{
  if (hDialog != NULL) {
    SendMessage(hDialog, WM_DESTROY,0,0);
    WaitForSingleObject(ConfigThreadHandle,4000);
    CloseHandle(ConfigThreadHandle);
  }
}

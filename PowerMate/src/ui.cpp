/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

struct ActionEntry {
  const char *desc;
  ActionType_t subtype;
  RequestType_t reqtype;
} ActionEntries[] = {
  { "Repeat [pld1] times", ACTION_REPEAT },
  { "Set LED Brightness (0-255)", ACTION_REQUEST, LED_BRIGHTNESS },
  { "Pulse during sleep (0 / 1)", ACTION_REQUEST, PULSE_SLEEP },
  { "Pulse always (0 / 1)", ACTION_REQUEST, PULSE_ALWAYS },
  { "Pulse speed (1-24)", ACTION_REQUEST, PULSE_SPEED },
};

#define countof(x) sizeof(x)/sizeof(x[0])

/* Local variables */
p_command g_pCommand = NULL;
HWND g_hCommandDialog = NULL;
HANDLE g_hCommandThread = NULL;
int g_nLink = 0;

static void AdjustVisibility(HWND hwnd)
{
  BOOL bHasLink = FALSE;
  
  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if (NULL != ae)
      bHasLink = (ACTION_REPEAT == ae->subtype);
  }
  
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), (!bHasLink) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALUE), (!bHasLink) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_LINKL), (bHasLink) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_LINK), (bHasLink) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_LINKB), (bHasLink) ? SW_SHOW : SW_HIDE);
}

static void LoadUISettings(HWND hwnd)
{
  if (NULL == g_pCommand) {
    EnableWindow(GetDlgItem(hwnd, IDC_ACTION), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_VALUE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_LINKB), FALSE);
    return;
  }

  EnterCriticalSection(&g_pCommand->critical_section);

  SetWindowText(GetDlgItem(hwnd, IDC_VALUE), g_pCommand->svalue1);

  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int nf = SendMessage(hwndList, CB_GETCOUNT, 0, 0);
  int selidx = 0;
  for (int idx = 0; idx < nf; idx++) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if ((ae->subtype == g_pCommand->actionsubtype) &&
        (ae->reqtype == g_pCommand->ivalue1)) {
      selidx = idx;
      break;
    }
  }
  SendMessage(hwndList, CB_SETCURSEL, selidx, 0);
             
  EnableWindow(GetDlgItem(hwnd, IDC_ACTION), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_VALUE), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_LINKB), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  
  LeaveCriticalSection(&g_pCommand->critical_section);

  AdjustVisibility(hwnd);
}

static BOOL SaveUISettings(HWND hwnd)
{
  char buf[1024];
  
  EnterCriticalSection(&g_pCommand->critical_section);

  GetWindowText(GetDlgItem(hwnd, IDC_VALUE), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue1, buf);

  g_pCommand->lvalue1 = g_nLink;

  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    g_pCommand->actionsubtype = ae->subtype;
    g_pCommand->ivalue1 = ae->reqtype;
  }

  g_pCommand->actiontype = PLUGINNUM;
  SF.set_command(g_pCommand);
  
  LeaveCriticalSection(&g_pCommand->critical_section);  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static BOOL CALLBACK CommandDialogProc(HWND hwnd,  UINT uMsg, 
                                       WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_hCommandDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.i18n_translate("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.i18n_translate("Action:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_ACTIONL), trans);

      SF.i18n_translate("Value:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      SF.i18n_translate("Command:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_LINKL), trans);

      SF.i18n_translate("Browse", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_LINKB), trans);

      HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
      for (size_t i = 0; i < countof(ActionEntries); i++) {
        int idx = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)ActionEntries[i].desc);
        SendMessage(hwndList, CB_SETITEMDATA, idx, (LPARAM)(ActionEntries + i));
      }

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

    case IDC_VALUE:
      if (EN_CHANGE == HIWORD(wParam)) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_ACTION:
      if (CBN_SELENDOK == HIWORD(wParam)) {
        AdjustVisibility(hwnd);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_LINKB:
      {
        int nid = SF.treepicker_show(hwnd, g_nLink);
        if (nid > 0) {
          char buf[256];
          SF.get_link_name(nid, buf, sizeof(buf));
          SetWindowText(GetDlgItem(hwnd, IDC_LINK), buf);
          g_nLink = nid;
        }
      }
      return 1;
    }
    break;

  case WM_USER+100:
    LoadUISettings(hwnd);
    return 1;
  }

  return 0;
}

static DWORD WINAPI CommandThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, 
                               CommandDialogProc, (LPARAM)lpParam);
  g_hCommandDialog = NULL;
  return 0;
}

void OpenCommandUI()
{
   DWORD dwThreadId;
   if (NULL != g_hCommandDialog) {
     SetForegroundWindow(g_hCommandDialog);
   }
   else {
     g_hCommandThread = CreateThread(NULL, 0, &CommandThread, NULL, 0, &dwThreadId);
     if (NULL == g_hCommandThread)
       MessageBox(NULL, "Cannot create dialog thread.", "Error", MB_OK);
   }
}

void UpdateCommandUI(p_command command)
{
  g_pCommand = command;
  if (NULL != g_hCommandDialog)
    SendMessage(g_hCommandDialog, WM_USER+100, 0, 0);
}

void CloseCommandUI()
{
  if (NULL != g_hCommandDialog) {
    SendMessage(g_hCommandDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_hCommandThread, 4000);
    CloseHandle(g_hCommandThread);
  }
}

/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "ipc.h"

const UINT WM_SELECT_EVENT = WM_USER+100;

// See AttribKeys
enum ActionKey { 
  ATTRIB_SHUFFLE = 1, ATTRIB_REPEAT, ATTRIB_CROSSFADE
};

struct ActionEntry {
  const char *desc;
  IPC_OPCODE op;
  BOOL value;                   // Takes a value.
  BOOL reply;                   // Returns a result.
  int subindex;                 // Which of the returned values.
  int key;                      // For attributes.
} ActionEntries[] = {
  { "Play", IPC_C_START },
  { "Stop", IPC_C_STOP },
  { "Pause", ICP_C_PAUSE },
  { "Resume", IPC_C_RESUME },
  { "Pause / Resume", IPC_C_PAUSE_TOGGLE },
  { "Set position absolute (ms)", IPC_C_SEEK_ABSOLUTE, TRUE },
  { "Set position relative (ms)", IPC_C_SEEK_RELATIVE, TRUE },
  { "Next", IPC_C_NEXT },
  { "Previous", IPC_C_PREV },
  { "Set next playstring", IPC_C_SET_NEXT, TRUE },
  { "Set playlist position", IPC_C_TRACK, TRUE },
  { "Next playlst", IPC_C_NEXT_PLAYLIST },
  { "Previous playlist", IPC_C_PREV_PLAYLIST },
  { "Start playlist", IPC_C_START_PLAYLIST },
  { "Set volume", IPC_C_SET_VOLUME, TRUE },
  { "Set shuffle", IPC_C_SET_ATTRIB, TRUE, FALSE, 0, ATTRIB_SHUFFLE },
  { "Toggle shuffle", IPC_C_TOGGLE_ATTRIB, FALSE, FALSE, 0, ATTRIB_SHUFFLE },
  { "Set repeat", IPC_C_SET_ATTRIB, TRUE, FALSE, 0, ATTRIB_REPEAT },
  { "Toggle repeat", IPC_C_TOGGLE_ATTRIB, FALSE, FALSE, 0, ATTRIB_REPEAT },
  { "Set crossfade", IPC_C_SET_ATTRIB, TRUE, FALSE, 0, ATTRIB_CROSSFADE },
  { "Toggle crossfade", IPC_C_TOGGLE_ATTRIB, FALSE, FALSE, 0, ATTRIB_CROSSFADE },

  { "Get state", IPC_R_STATE, FALSE, TRUE },
  { "Get playstring", IPC_R_PLAYSTRING, FALSE, TRUE },
  { "Get meta-data", IPC_R_METADATA, TRUE, TRUE },
  { "Get position", IPC_R_TRACK, FALSE, TRUE, 0 },
  { "Get length", IPC_R_TRACK, FALSE, TRUE, 1 },
  { "Get elapsed", IPC_R_POSITION, FALSE, TRUE, 0 },
  { "Get duration", IPC_R_POSITION, FALSE, TRUE, 1 },
  { "Get playlist item", IPC_R_ITEM, TRUE, TRUE },
  { "Get playlist name", IPC_R_PLAYLIST, FALSE, TRUE },
  { "Get volume", IPC_R_VOLUME, FALSE, TRUE },
  { "Get shuffle", IPC_R_ATTRIB, FALSE, TRUE, 0, ATTRIB_SHUFFLE },
  { "Get repeat", IPC_R_ATTRIB, FALSE, TRUE, 0, ATTRIB_REPEAT },
  { "Get crossfade", IPC_R_ATTRIB, FALSE, TRUE, 0, ATTRIB_CROSSFADE },
};

#define countof(x) sizeof(x)/sizeof(x[0])

/* Local variables */
p_command g_pCommand = NULL;
HWND g_hCommandDialog = NULL;
HANDLE g_hCommandThread = NULL;
HANDLE g_hLearnThread = NULL;
HWND g_hLearnDialog = NULL;

static void AdjustVisibility(HWND hwnd)
{
  BOOL bHasValue = FALSE, bHasResult = FALSE;
  
  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if (NULL != ae) {
      bHasValue = ae->value;
      bHasResult = ae->reply;
    }
  }
  
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), (bHasValue) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALUE), (bHasValue) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_RESULTL), (bHasResult) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_RESULT), (bHasResult) ? SW_SHOW : SW_HIDE);
}

static void LoadUISettings(HWND hwnd)
{
  if (NULL == g_pCommand) {
    EnableWindow(GetDlgItem(hwnd, IDC_ACTION), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_VALUE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_RESULT), FALSE);
    return;
  }

  EnterCriticalSection(&g_pCommand->critical_section);

  SetWindowText(GetDlgItem(hwnd, IDC_VALUE), g_pCommand->svalue1);
  SetWindowText(GetDlgItem(hwnd, IDC_RESULT), g_pCommand->svalue2);

  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int nf = SendMessage(hwndList, CB_GETCOUNT, 0, 0);
  int selidx = 0;
  for (int idx = 0; idx < nf; idx++) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if ((ae->op == g_pCommand->ivalue1) &&
        (ae->subindex == g_pCommand->ivalue2) &&
        (ae->key == g_pCommand->ivalue3)) {
      selidx = idx;
      break;
    }
  }
  SendMessage(hwndList, CB_SETCURSEL, selidx, 0);
             
  EnableWindow(GetDlgItem(hwnd, IDC_ACTION), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_VALUE), TRUE);
  
  LeaveCriticalSection(&g_pCommand->critical_section);

  AdjustVisibility(hwnd);
}

static BOOL SaveUISettings(HWND hwnd)
{
  char buf[1024];
  
  EnterCriticalSection(&g_pCommand->critical_section);

  GetWindowText(GetDlgItem(hwnd, IDC_VALUE), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue1, buf);

  GetWindowText(GetDlgItem(hwnd, IDC_RESULT), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue2, buf);

  HWND hwndList = GetDlgItem(hwnd, IDC_ACTION);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    ActionEntry *ae = (ActionEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    g_pCommand->ivalue1 = ae->op;
    g_pCommand->ivalue2 = ae->subindex;
    g_pCommand->ivalue3 = ae->key;
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

      SF.i18n_translate("Result var:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_RESULTL), trans);

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
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_COMMAND_DIALOG), NULL, 
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

// Has to be kept in sync both with ProcessMessage and MetaInfos.
const char *Events[] = {
  "Close",
  "Start",
  "Stop",
  "Pause",
  "Resume",
  "Length",
  "Position",
  "Duration",
  "Elapsed",
  "Playstring",
  "Title",
  "Album",
  "Artist",
  "Genre",
  "Year",
  "Comment",
  "Info",
#if 0
  "Bitrate",
  "Bps",
  "Nch",
  "Srate",
#endif
  "Track",
  "Name",
#if 0
  "AddedOn",
  "Disabled",
  "Last played",
  "Length",
  "Modified",
  "Name2",
  "Playstring",
  "Size",
#endif
};

static BOOL CALLBACK LearnDialogProc(HWND hwnd, UINT uMsg, 
                                     WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_hLearnDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Learn", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_LEARN), trans);

      SF.i18n_translate("Close", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCLOSE), trans);

      HWND events = GetDlgItem(hwnd, IDC_EVENTS);
      for (int i = 0; i < countof(Events); i++) {
        char buf[128];
        strcpy(buf, "Winamp3.");
        strcat(buf, Events[i]);
        SendMessage(events, LB_ADDSTRING, 0, (LPARAM)buf);
      }

      SendMessage(hwnd, WM_SELECT_EVENT, 0, lParam);

      return FALSE;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return FALSE;

  case WM_CLOSE: 
    EndDialog(hwnd, FALSE);
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDC_LEARN:
      {
        HWND events = GetDlgItem(hwnd, IDC_EVENTS);
        LRESULT pos = SendMessage(events, LB_GETCURSEL, 0, lParam);
        if (LB_ERR != pos) {
          char buf[256];
          SendMessage(events, LB_GETTEXT, (WPARAM)pos, (LPARAM)buf);
          SF.send_event(buf, NULL, 0, PLUGINNUM);
        }
      }
      return TRUE;

    case IDCLOSE:
      EndDialog(hwnd, TRUE);
      return TRUE;
    }
    break;

  case WM_SELECT_EVENT:
    {
      HWND events = GetDlgItem(hwnd, IDC_EVENTS);
      LRESULT pos = SendMessage(events, LB_FINDSTRINGEXACT, 0, lParam);
      SendMessage(events, LB_SETCURSEL, (WPARAM)pos, 0);
      return TRUE;
    }

  }
  return FALSE;
}

static DWORD WINAPI LearnThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_LEARN_DIALOG), NULL,
                               LearnDialogProc, (LPARAM)lpParam);
  g_hLearnDialog = NULL;
  return 0;
}

void OpenLearnUI(PCHAR old)
{
  DWORD dwThreadId;
  if (g_hLearnDialog != 0) {
    SetForegroundWindow(g_hLearnDialog);
    SendMessage(g_hLearnDialog, WM_SELECT_EVENT, 0, (LPARAM)old);
  }
  else {
    g_hLearnThread = CreateThread(NULL, 0, LearnThread, old, 0, &dwThreadId);
    if (NULL == g_hLearnThread)
      MessageBox(0, "Cannot create dialog thread.", "Error", MB_OK);
  }
}

void CloseLearnUI()
{
  if (NULL != g_hLearnDialog) {
    SendMessage(g_hLearnDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_hLearnThread, 5000);
    CloseHandle(g_hLearnThread);
  }
}

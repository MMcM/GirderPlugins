/* User interface. */

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

/* Local variables */
p_command g_pCommand = NULL;
HWND g_hCommandDialog = NULL;
HANDLE g_hCommandThread = NULL;

static BOOL SaveUISettings(HWND hwnd)
{
  char buf[1024];

  EnterCriticalSection(&g_pCommand->critical_section);

  if (SendMessage(GetDlgItem(hwnd, IDC_RECORD), BM_GETCHECK, 0, 0))
    g_pCommand->ivalue1 = 1;
  else
    g_pCommand->ivalue1 = 0;
  
  GetWindowText(GetDlgItem(hwnd, IDC_DEVICE), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue1, buf);

  g_pCommand->actiontype = PLUGINNUM;
  SF.set_command(g_pCommand);
  
  LeaveCriticalSection(&g_pCommand->critical_section);
  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static void LoadCombo(HWND hwnd)
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

static void LoadUISettings(HWND hwnd)
{
  if (NULL == g_pCommand) {
    EnableWindow(GetDlgItem(hwnd, IDC_PLAYBACK), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_RECORD), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_DEVICE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
    return;
  }

  EnterCriticalSection(&g_pCommand->critical_section);

  SendMessage(GetDlgItem(hwnd, IDC_PLAYBACK), BM_SETCHECK, (g_pCommand->ivalue1 == 0), 0);
  SendMessage(GetDlgItem(hwnd, IDC_RECORD), BM_SETCHECK, (g_pCommand->ivalue1 == 1), 0);
  LoadCombo(hwnd);
  SetWindowText(GetDlgItem(hwnd, IDC_DEVICE), g_pCommand->svalue1);

  EnableWindow(GetDlgItem(hwnd, IDC_PLAYBACK), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_RECORD), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_DEVICE), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  LeaveCriticalSection(&g_pCommand->critical_section);
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

      SF.i18n_translate("Playback", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_PLAYBACK), trans);

      SF.i18n_translate("Record", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_RECORD), trans);

      SF.i18n_translate("Device:", trans, sizeof(trans));
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

/* User interface. 
$Header$
*/

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
  
  GetWindowText(GetDlgItem(hwnd, IDC_KEY), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue1, buf);

  GetWindowText(GetDlgItem(hwnd, IDC_VALUE), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue2, buf);
  
  if (SendMessage(GetDlgItem(hwnd, IDC_DWORD), BM_GETCHECK, 0, 0))
    g_pCommand->ivalue1 = REG_DWORD;
  else
    g_pCommand->ivalue1 = REG_SZ;

  g_pCommand->actiontype=PLUGINNUM;
  SF.set_command(g_pCommand);

  HKEY hkey;
  PCHAR pval = DecodeKeyValue(g_pCommand->svalue1, &hkey);
  if (NULL == pval)
  {
    LeaveCriticalSection(&g_pCommand->critical_section);
    return FALSE;
  }
  LONG rc = RegQueryValueEx(hkey, pval, NULL, NULL, NULL, NULL);
  RegCloseKey(hkey);
  if (ERROR_SUCCESS != rc) {
    LeaveCriticalSection(&g_pCommand->critical_section);
    sprintf(buf, "Value does not exist: %s", g_pCommand->svalue1);
    MessageBox(0, buf, "Error", MB_ICONERROR);
    return FALSE;
  }
  
  LeaveCriticalSection(&g_pCommand->critical_section);  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static void LoadUISettings(HWND hwnd)
{
  if (NULL == g_pCommand) {
    EnableWindow(GetDlgItem(hwnd, IDC_KEY), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_VALUE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
    return;
  }

  EnterCriticalSection(&g_pCommand->critical_section);

  if (!g_pCommand->ivalue1) {
    // Compatibility.
    if (g_pCommand->bvalue1) {
      g_pCommand->bvalue1 = FALSE;
      char buf[32];
      sprintf(buf, "%d", g_pCommand->lvalue1);
      g_pCommand->lvalue1 = 0;
      SF.realloc_pchar(&(g_pCommand->svalue2), buf);
      g_pCommand->ivalue1 = REG_DWORD;
    }
    else
      g_pCommand->ivalue1 = REG_SZ;
  }

  SetWindowText(GetDlgItem(hwnd, IDC_KEY), g_pCommand->svalue1);
  SetWindowText(GetDlgItem(hwnd, IDC_VALUE), g_pCommand->svalue2);
  SendMessage(GetDlgItem(hwnd, IDC_SZ), BM_SETCHECK, 
              (g_pCommand->ivalue1 == REG_SZ), 0);
  SendMessage(GetDlgItem(hwnd, IDC_DWORD), BM_SETCHECK, 
              (g_pCommand->ivalue1 == REG_DWORD), 0);
  
  EnableWindow(GetDlgItem(hwnd, IDC_KEY), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_VALUE), TRUE);
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

      SF.i18n_translate("Key\\Name:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_KEYL), trans);
	
      SF.i18n_translate("Value:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      SF.i18n_translate("Type:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_TYPEL), trans);

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
      if (HIWORD(wParam) == EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_SZ:
      SendMessage(GetDlgItem(hwnd, IDC_DWORD), BM_SETCHECK, FALSE, 0);
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_DWORD:
      SendMessage(GetDlgItem(hwnd, IDC_SZ), BM_SETCHECK, FALSE, 0);
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
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

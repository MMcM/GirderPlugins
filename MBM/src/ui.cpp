/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

const UINT WM_SELECT_EVENT = WM_USER+100;

HANDLE g_hLearnThread = NULL;
HWND g_hLearnDialog = NULL;

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
      for (size_t i = 0; i < NSENSORS; i++) {
        LPCSTR event = GetEventName(i);
        if (NULL != event)
          SendMessage(events, LB_ADDSTRING, 0, (LPARAM)event);
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
        char buf[256];
        memset(buf, 0, sizeof(buf));
        if (LB_ERR != pos) {
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

/* User interface. */

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

const UINT WM_SELECT_EVENT = WM_USER+100;

HANDLE g_hConfigThread = NULL;
HWND g_hConfigDialog = NULL;
HANDLE g_hLearnThread = NULL;
HWND g_hLearnDialog = NULL;

static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_hConfigDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DVDSPY)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

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
    case IDOK:
      EndDialog(hwnd, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return TRUE;
    }
    break;

  }
  return FALSE;
}

static DWORD WINAPI ConfigThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CONFIG_DIALOG), NULL,
                               ConfigDialogProc, (LPARAM)lpParam);
  g_hConfigDialog = NULL;
  return 0;
}

void OpenConfigUI()
{
  DWORD dwThreadId;
  if (NULL != g_hConfigDialog) {
    SetForegroundWindow(g_hConfigDialog);
  }
  else {
    g_hConfigThread = CreateThread(NULL, 0, ConfigThread, NULL, 0, &dwThreadId);
    if (NULL == g_hConfigThread)
      MessageBox(0, "Cannot create dialog thread.", "Error", MB_OK);
  }
}

void CloseConfigUI()
{
  if (NULL != g_hConfigDialog) {
    SendMessage(g_hConfigDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_hConfigThread, 5000);
    CloseHandle(g_hConfigThread);
  }
}

static BOOL CALLBACK LearnDialogProc(HWND hwnd, UINT uMsg, 
                                     WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_hLearnDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DVDSPY)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Learn", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_LEARN), trans);

      SF.i18n_translate("Close", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCLOSE), trans);

      HWND events = GetDlgItem(hwnd, IDC_EVENTS);
      {
        char buf[128];
        GetLogicalDriveStrings(sizeof(buf), buf);
        LPSTR pb = buf;
        while ('\0' != *pb) {
          if (DRIVE_CDROM == GetDriveType(pb)) {
            char cd = *pb;
            if ((cd >= 'a') && (cd <= 'z'))
              cd -= 'a' - 'A';  // Happens sometimes, maybe on XP?
            for (int i = 0; i < 2; i++) {
              char ebuf[256];
              strcpy(ebuf, "Disc.");
              if (0 == i)
                strcat(ebuf, "Insert");
              else
                strcat(ebuf, "Eject");
              LPSTR pe = ebuf + strlen(ebuf);
              *pe++ = '.';
              *pe++ = cd;
              *pe++ = '\0';
              SendMessage(events, LB_ADDSTRING, 0, (LPARAM)ebuf);
            }
          }
          pb += strlen(pb) + 1;
        }
      }

      // Registry can specify events not known to the hook itself, such as WMP.
      HKEY hKey = NULL;
      if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, 
                                      "Software\\Girder3\\HardPlugins\\DVDSpy\\Events", 
                                      &hKey)) {
        DWORD dwIndex = 0;
        while (TRUE) {
          char szName[128], szValue[128];
          DWORD dwType, dwNLen = sizeof(szName), dwLen = sizeof(szValue);
          if (ERROR_SUCCESS != RegEnumValue(hKey, dwIndex++, szName, &dwNLen,
                                            NULL, &dwType, (LPBYTE)szValue, &dwLen))
            break;
          if (REG_SZ != dwType) continue;
          if (dwLen > 1) {
            strcat(szName, " (");
            strcat(szName, szValue);
            strcat(szName, ")");
          }
          SendMessage(events, LB_ADDSTRING, 0, (LPARAM)szName);
        }
      }

      // Do ones not mentioned in the registry.
      size_t nmatches = DS_GetMatchCount();
      for (size_t i = 0; i < nmatches; i++) {
        size_t nindex = DS_GetMatchIndexCount(i);
        for (size_t j = 0; j < nindex; j++) {
          char szName[128];
          DS_GetName(i, j, szName, sizeof(szName));
          if ((NULL == hKey) ||
              (ERROR_SUCCESS != RegQueryValueEx(hKey, szName, 
                                                NULL, NULL, NULL, NULL))) {
            SendMessage(events, LB_ADDSTRING, 0, (LPARAM)szName);
          }
        }
      }
      SendMessage(hwnd, WM_SELECT_EVENT, 0, lParam);

      if (NULL != hKey)
        RegCloseKey(hKey);

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
        if (LB_ERR != pos)
          SendMessage(events, LB_GETTEXT, (WPARAM)pos, (LPARAM)buf);
        char *sp = strstr(buf, " ("); // Trim off any description.
        if (NULL != sp)
          *sp = '\0';
        GirderEvent(buf);
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

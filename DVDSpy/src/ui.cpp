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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define UI_CPP
#include "girder.h"
#include "ui.h"
#include "resource.h"

#include "DisplaySpyHook.h"

static
BOOL CALLBACK ConfigDialogProc(  HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      PCHAR trans;
      	
      hConfigDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      trans=I18NTranslate("Ok");
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      trans=I18NTranslate("Cancel");
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);
      return 0;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return 0;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      EndDialog(hwnd, TRUE);
      return 1;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return 1;
    }
    break;

  }
  return 0;
}

DWORD WINAPI ConfigThread( LPVOID lpParameter )
{
  BOOL fResult;

  fResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CONFIG_DIALOG), NULL,
                           ConfigDialogProc, (LPARAM)lpParameter);

  hConfigDialog=0;
  
  return 0;
}

static
BOOL CALLBACK LearnDialogProc(  HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      PCHAR trans;
      	
      hLearnDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      trans=I18NTranslate("Learn");
      SetWindowText(GetDlgItem(hwnd, IDC_LEARN), trans);

      trans=I18NTranslate("Close");
      SetWindowText(GetDlgItem(hwnd, IDCLOSE), trans);

      HWND events = GetDlgItem(hwnd, IDC_EVENTS);
      {
        char buf[128];
        GetLogicalDriveStrings(sizeof(buf), buf);
        LPSTR pb = buf;
        while ('\0' != *pb) {
          if (DRIVE_CDROM == GetDriveType(pb)) {
            for (int i = 0; i < 2; i++) {
              char ebuf[256];
              strcpy(ebuf, "Disc.");
              if (0 == i)
                strcat(ebuf, "Insert");
              else
                strcat(ebuf, "Eject");
              LPSTR pe = ebuf + strlen(ebuf);
              *pe++ = '.';
              *pe++ = *pb;
              *pe++ = '\0';
              SendMessage(events, LB_ADDSTRING, 0, (LPARAM)ebuf);
            }
          }
          pb += strlen(pb) + 1;
        }
      }
      size_t nmatches = DS_GetMatchCount();
      for (size_t i = 0; i < nmatches; i++) {
        char szName[128];
        DS_GetMatchName(i, szName, sizeof(szName));
        size_t nindex = DS_GetMatchIndexCount(i);
        if (nindex <= 1)
          SendMessage(events, LB_ADDSTRING, 0, (LPARAM)szName);
        else {
          LPSTR end = szName + strlen(szName);
          for (size_t j = 0; j < nindex; j++) {
            sprintf(end, ".%d", j);
            SendMessage(events, LB_ADDSTRING, 0, (LPARAM)szName);
          }
        }
      }
      SendMessage(hwnd, WM_USER+100, 0, lParam);

      return 0;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return 0;

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
        GirderEvent(buf);
      }
      return 1;

    case IDCLOSE:
      EndDialog(hwnd, TRUE);
      return 1;
    }
    break;

  case WM_USER+100:
    {
      HWND events = GetDlgItem(hwnd, IDC_EVENTS);
      LRESULT pos = SendMessage(events, LB_FINDSTRINGEXACT, 0, lParam);
      SendMessage(events, LB_SETCURSEL, (WPARAM)pos, 0);
    }
    break;

  }
  return 0;
}

DWORD WINAPI LearnThread( LPVOID lpParameter )
{
  BOOL fResult;

  fResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_LEARN_DIALOG), NULL,
                           LearnDialogProc, (LPARAM)lpParameter);

  hLearnDialog=0;
  
  return 0;
}

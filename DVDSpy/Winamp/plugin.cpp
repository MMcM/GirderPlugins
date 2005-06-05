/** Plug-in for communicating from Winamp to Girder / DVDSpy 
$Header$
**/

#include <windows.h>
#include <stdio.h>

#include "resource.h"

extern "C" {
#include "GEN.H"
#include "FRONTEND.H"
}

void Config();
void Quit();
int Init();

static winampGeneralPurposePlugin g_plugin = {
  GPPHDR_VER,
  "Girder / DVDSpy plug-in v1.67",
  Init,
  Config,
  Quit,
};

static HWND g_hwndDVDSpy = NULL;
static int g_nRefreshInterval = 1000;   // One second.
static char g_szEventPrefix[128] = "Winamp";
static UINT g_idTimer = 0;
static int g_nTimerCount = 0;
static int g_nPosition = 0;

enum ValType { TYPE_INT = 1, TYPE_STRING, TYPE_SECS, TYPE_MILLIS, 
               TYPE_PLAYSTATE, TYPE_INT_ZERO };

const WPARAM ARG_SETPOS = 0xFEEFABBA;
const WPARAM ARG_GETPOS = 0xFEEFBAAB;

struct {
  const char *szName;
  LPARAM lIPC;
  ValType type;
  WPARAM nArg;
  LRESULT lOld;
} g_extracts[] = {
  { "Status", IPC_ISPLAYING, TYPE_PLAYSTATE },
  { "Length", IPC_GETLISTLENGTH, TYPE_INT },
  { "Position", IPC_GETLISTPOS, TYPE_INT_ZERO, ARG_SETPOS },
  { "Duration", IPC_GETOUTPUTTIME, TYPE_SECS, 1 },
  { "Elapsed", IPC_GETOUTPUTTIME, TYPE_MILLIS, 0 },
  { "File", IPC_GETPLAYLISTFILE, TYPE_STRING, ARG_GETPOS },
  { "Title", IPC_GETPLAYLISTTITLE, TYPE_STRING, ARG_GETPOS },
#if 0
  { "SampleRate", IPC_GETINFO, TYPE_INT, 0 },
  { "Bitrate", IPC_GETINFO, TYPE_INT, 1 },
  { "Channels", IPC_GETINFO, TYPE_INT, 2 },
#endif
};

#define countof(x) (sizeof(x)/sizeof(x[0]))

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

void formatTime(LPSTR buf, int secs)
{
  if (secs < 0) {
    *buf = '\0';
    return;
  }
  int mins = secs / 60;
  secs -= mins * 60;
  int hrs = mins / 60;
  mins -= hrs * 60;
  if (hrs > 0)
    sprintf(buf, "%d:%02d:%02d", hrs, mins, secs);
  else
    sprintf(buf, "%d:%02d", mins, secs);
}

void CALLBACK Timer(HWND, UINT, UINT, DWORD)
{
  // TODO: Consider IsWindow check on g_hwndDVDSpy.
  BOOL bFirst = (g_nTimerCount++ == 0);
  HWND hwndTarget = g_plugin.hwndParent; // No need for FindWindow.
  for (int i = 0; i < countof(g_extracts); i++) {
    WPARAM wParam = g_extracts[i].nArg;
    LPARAM lParam = g_extracts[i].lIPC;
    if (ARG_GETPOS == wParam)
      wParam = g_nPosition;
    LRESULT lResult = SendMessage(hwndTarget, WM_WA_IPC, wParam, lParam);
    if (ARG_SETPOS == wParam)
      g_nPosition = lResult;
    ValType type = g_extracts[i].type;
    if ((TYPE_STRING == type) && (NULL == lResult))
      lResult = (LPARAM)"";
    if (bFirst || 
        ((TYPE_STRING == type) ? 
         (0 != strcmp((LPCSTR)lResult, (LPCSTR)g_extracts[i].lOld)) :
         (lResult != g_extracts[i].lOld))) {
      if (TYPE_STRING == type) {
        if (!bFirst)
          free((PVOID)g_extracts[i].lOld);
        lResult = (LPARAM)_strdup((LPCSTR)lResult);
      }
      g_extracts[i].lOld = lResult;
      
      char buf[1024], *pbuf;
      strcpy(buf, g_szEventPrefix);
      pbuf = buf + strlen(buf);
      *pbuf++ = '.';
      strcpy(pbuf, g_extracts[i].szName);
      pbuf += strlen(pbuf) + 1;
      *pbuf++ = 1;
      switch (type) {
      case TYPE_INT:
        sprintf(pbuf, "%d", (int)lResult);
        break;
      case TYPE_STRING:
        strncpy(pbuf, (LPCSTR)lResult, sizeof(buf) - (pbuf - buf));
        break;
      case TYPE_SECS:
        formatTime(pbuf, (int)lResult);
        break;
      case TYPE_MILLIS:
        formatTime(pbuf, (int)lResult / 1000);
        break;
      case TYPE_PLAYSTATE:
        switch (lResult) {
        case 1:
          strncpy(pbuf, "PLAY", sizeof(buf) - (pbuf - buf));
          break;
        case 3:
          strncpy(pbuf, "PAUSE", sizeof(buf) - (pbuf - buf));
          break;
        default:
          strncpy(pbuf, "STOP", sizeof(buf) - (pbuf - buf));
          break;
        }
        break;
      case TYPE_INT_ZERO:
        sprintf(pbuf, "%d", (int)lResult + 1);
        break;
      }
      pbuf += strlen(pbuf) + 1;

      COPYDATASTRUCT cd;
      cd.dwData = 0;
      cd.lpData = buf;
      cd.cbData = pbuf - buf;
      SendMessage(g_hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
    }
  }
}

void StartTimer()
{
  g_idTimer = SetTimer(NULL, 0, g_nRefreshInterval, Timer);
  g_nTimerCount = 0;
}

void StopTimer()
{
  if (0 != g_idTimer) {
    KillTimer(NULL, g_idTimer);
    g_idTimer = 0;
  }
}

void GetIniFile(char *buf)
{
  GetModuleFileName(g_plugin.hDllInstance, buf, MAX_PATH);
  char *ep = strrchr(buf, '\\');
  if (NULL != ep)
    ep++;
  else
    ep = buf;
  strcpy(ep, "plugin.ini");
}

void ReadConfig()
{
  char buf[MAX_PATH];
  GetIniFile(buf);
  g_nRefreshInterval = GetPrivateProfileInt("gen_DVDSpy", "RefreshInterval", 
                                            g_nRefreshInterval, 
                                            buf);
  GetPrivateProfileString("gen_DVDSpy", "EventPrefix",
                          g_szEventPrefix, g_szEventPrefix, sizeof(g_szEventPrefix), 
                          buf);
}

void WriteConfig()
{
  char buf1[MAX_PATH], buf2[128];
  GetIniFile(buf1);
  sprintf(buf2, "%d", g_nRefreshInterval);
  WritePrivateProfileString("gen_DVDSpy", "RefreshInterval", buf2, buf1);
  WritePrivateProfileString("gen_DVDSpy", "EventPrefix", g_szEventPrefix, buf1);
}

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char buf[128];
      sprintf(buf, "%d", g_nRefreshInterval);
      SetWindowText(GetDlgItem(hwndDlg, IDC_REFRESH), buf);
    }
    break;
  case WM_COMMAND:
    {
      WORD ctrl = LOWORD(wParam);
      if ((ctrl == IDC_APPLY) || (ctrl == IDOK)) {
        char buf[128];
        GetWindowText(GetDlgItem(hwndDlg, IDC_REFRESH), buf, sizeof(buf));
        g_nRefreshInterval = atoi(buf);
        WriteConfig();
        StopTimer();
        StartTimer();
      }
      if ((ctrl == IDOK) || (ctrl == IDCANCEL))
        EndDialog(hwndDlg, ctrl);
    }
    break;
  }
  return FALSE;
}

void Config()
{
  DialogBox(g_plugin.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG), 
            g_plugin.hwndParent, ConfigProc);
}

void Quit()
{
  StopTimer();
  if (NULL != g_hwndDVDSpy) {
    char cCloseEvent[1024];
    strcpy(cCloseEvent, g_szEventPrefix);
    strcat(cCloseEvent, ".Close");
    char *ebuf = cCloseEvent + strlen(cCloseEvent) + 1;
    *ebuf++ = '\0';             // Two nuls.
    COPYDATASTRUCT cd;
    cd.dwData = 0;
    cd.lpData = cCloseEvent;
    cd.cbData = ebuf - cCloseEvent;
    SendMessage(g_hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
  }
}

int Init()
{
  EnumWindows(FindMonitorWindow, (LPARAM)&g_hwndDVDSpy);
  // TODO: Consider checking more often.
  if (NULL == g_hwndDVDSpy)
    return 0;
  ReadConfig();
  StartTimer();
  return 0;
}

extern "C" __declspec(dllexport)
winampGeneralPurposePlugin* winampGetGeneralPurposePlugin()
{
  return &g_plugin;
}

/** foo_girder_dvdspy -- Foobar2000 component for DVDSpy 
$Header$
**/

#include "stdafx.h"
#include "resource.h"

DECLARE_COMPONENT_VERSION(
  "Foobar2000 / Girder DVDSpy component",
  "1.63",
  "Spy on Foobar state/track info to pass to Girder DVDSpy")

// Title Format string for Title event
static cfg_string cfg_title_format("title_format", "$if2(%album artist%,%artist%): %title%");

static HWND g_hwndDVDSpy = NULL;
static char g_szEventPrefix[128] = "Foobar";

LRESULT uSendMessageDVDSpy(const char * event, int num_args, ...) {
  char buf[1024], *pbuf;
  strcpy(buf, g_szEventPrefix);
  strcat(buf, event);
  pbuf = buf + strlen(buf) + 1;

  va_list args;

  va_start(args, num_args);
  int n;
  for (n = 0; n < num_args; n++) {
    const char * arg = va_arg(args, const char *);
    *pbuf++ = 1;
    strncpy(pbuf, arg, sizeof(buf) - (pbuf - buf));
  }
  va_end(args);

  pbuf += strlen(pbuf) + 1;

  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = buf;
  cd.cbData = pbuf - buf;
  return uSendMessage(g_hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
}

inline LRESULT uSendMessageDVDSpy(const char * event) {
  return uSendMessageDVDSpy(event, 0);
}

inline LRESULT uSendMessageDVDSpy(const char * event, const char * arg) {
  return uSendMessageDVDSpy(event, 1, arg);
}

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam) {
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;
}

class initquit_spy : public initquit {
  virtual void on_init() {
    // Find DVDSpy window
    EnumWindows(FindMonitorWindow, (LPARAM)&g_hwndDVDSpy);
  }
  virtual void on_quit() {
    if (NULL != g_hwndDVDSpy) {
      uSendMessageDVDSpy(".Close");
    }
  }
  virtual void on_system_shutdown() {
    on_quit();
  }
};

static initquit_factory< initquit_spy > foo_initquit;

class play_callback_spy : public play_callback {
public:
  virtual unsigned get_callback_mask() {
    return MASK_on_playback_starting | MASK_on_playback_new_track | MASK_on_playback_stop |
         MASK_on_playback_pause | MASK_on_playback_time;
  }
  virtual void on_playback_starting() {
    if (NULL != g_hwndDVDSpy) {
      uSendMessageDVDSpy(".Status", "Play");
    }
  }
  virtual void on_playback_new_track(metadb_handle * track) {
    if (NULL != g_hwndDVDSpy) {
      // If DVDSpy wasn't detected at startup, check if it's been
      // started whenever a new track begins.
      EnumWindows(FindMonitorWindow, (LPARAM)&g_hwndDVDSpy);
    }
    if (NULL != g_hwndDVDSpy) {
      // Send two DVDSpy messages, Title and Duration:
      // 1. Title
      string8 title;
      track->handle_format_title(title, cfg_title_format, NULL);

      uSendMessageDVDSpy(".Title", title);

      // 2. Duration
      static const char * LENGTH_FORMAT = "%_length%";
      string8 length;
      track->handle_format_title(length, LENGTH_FORMAT, NULL);

      uSendMessageDVDSpy(".Duration", length);
      uSendMessageDVDSpy(".Elapsed", string_print_time(0));
    }
  }
  virtual void on_playback_stop(play_control::stop_reason reason) {
    if ((NULL != g_hwndDVDSpy) && (reason != play_control::STOP_REASON_STARTING_ANOTHER)) {
      uSendMessageDVDSpy(".Status", "Stop");
    }
  }
  virtual void on_playback_seek(double time) {
  }
  virtual void on_playback_pause(int state) {
    if (NULL != g_hwndDVDSpy) {
      uSendMessageDVDSpy(".Status", state ? "Pause" : "Play");
    }
  }
  virtual void on_playback_edited(metadb_handle * track) {
  }
  virtual void on_playback_dynamic_info(const file_info * info,bool b_track_change) {
  }
  virtual void on_playback_time(metadb_handle * track,double val) {
    if (NULL != g_hwndDVDSpy) {
      uSendMessageDVDSpy(".Elapsed", string_print_time((int)val));
    }
  }
  virtual void on_volume_change(int new_val) {
  }
};

static play_callback_factory< play_callback_spy > foo_play_callback;

class config_spy : public config
{
  static BOOL CALLBACK DialogProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
  {
    switch(msg)
    {
    case WM_INITDIALOG:
      uSetDlgItemText(wnd,IDC_TITLE_FORMAT,cfg_title_format);      
      return 1;
    case WM_COMMAND:
      switch(wp)
      {
      case IDC_TITLE_FORMAT | (EN_CHANGE<<16):
        cfg_title_format = string_utf8_from_window((HWND)lp);
        break;
      }
      break;
    }
    return 0;
  }
public:
  HWND create(HWND parent)
  {
    return uCreateDialog(IDD_CONFIG,parent,DialogProc);
  }
  const char * get_name() {return "Girder DVDSpy";}
  const char * get_parent_name() {return "Components";}

};

static config_factory< config_spy > foo_config;

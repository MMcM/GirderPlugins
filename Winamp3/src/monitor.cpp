/* Monitor thread */

#include "stdafx.h"
#include "plugin.h"
#include "ipc.h"

HANDLE g_hMonitorThread = NULL;
DWORD g_dwMonitorThreadId = 0;
HWND g_hMonitorWindow = NULL;
BOOL g_bRunning = FALSE, g_bEvents = FALSE;

// Keep track of each running instance of Winamp that connects to Girder.
// This says which prefix to use for its incoming events and what window to
// send outgoing commands to.
class WinampInstance
{
public:
  static WinampInstance *Find(const char *prefix);
  static WinampInstance *Find(IPC_HANDLE handle);
  static WinampInstance *For(IPCMessage& msg) {
    return Find(msg.getSource());
  }

  WinampInstance(IPC_HANDLE handle, const char *prefix);
  ~WinampInstance();
  
  static WinampInstance *g_extent;
  IPC_HANDLE m_handle;
  char *m_prefix;
  WinampInstance *m_next;

  int m_position, m_length, m_elapsed, m_duration;
};

WinampInstance *WinampInstance::g_extent = NULL;

WinampInstance *WinampInstance::Find(const char *prefix)
{
  for (WinampInstance *inst = g_extent; NULL != inst; inst = inst->m_next) {
    if (!strcmp(inst->m_prefix, prefix))
      return inst;
  }
  return NULL;
}

WinampInstance *WinampInstance::Find(IPC_HANDLE handle) 
{
  for (WinampInstance *inst = g_extent; NULL != inst; inst = inst->m_next) {
    if (inst->m_handle == handle)
      return inst;
  }
  return NULL;
}

 WinampInstance::WinampInstance(IPC_HANDLE handle, const char *prefix) 
{
  m_handle = handle;
  m_prefix = strdup(prefix);
  m_next = g_extent;
  g_extent = this;

  m_position, m_length, m_elapsed, m_duration = -1;
}

 WinampInstance::~WinampInstance() 
{
  free(m_prefix);
  WinampInstance **prev = &g_extent;
  while (NULL != *prev) {
    if (this == *prev) {
      *prev = m_next;
      break;
    }
    prev = &(*prev)->m_next;
  }
}

void StringEvent(WinampInstance *inst, const char *event, const char *payload)
{
  char evbuf[128];
  char *ep = evbuf;
  size_t prlen = strlen(inst->m_prefix);
  if (prlen > 0) {
    memcpy(ep, inst->m_prefix, prlen);
    ep += prlen;
    *ep++ = '.';
  }
  strcpy(ep, event);

  if (NULL == payload) {
    SF.send_event(evbuf, NULL, 0, PLUGINNUM);
    return;
  }

  char plbuf[1024];
  plbuf[0] = 1;
  strncpy(plbuf + 1, payload, sizeof(plbuf) - 1);
  SF.send_event(evbuf, plbuf, strlen(plbuf) + 1, PLUGINNUM);
}

// Format numerators according to the length of denominators so that
// fixed length displays work nicely.

size_t SizeInt(int i)
{
  if (i < 0) 
    return (SizeInt(-i) + 1);

  int n = 1;
  unsigned int p = 1;
  while (TRUE) {
    p *= 10;
    if (p > (unsigned int)i) break;
    n++;
  }
  return n;
}

char *FormatInt(char *buf, int i, size_t size)
{
  if (i < 0) {
    *buf++ = '-';
    i = -i;
  }
  for (size_t n = 0; n < size; n++) {
    buf[size - n - 1] = '0' + (i % 10);
    i /= 10;
  }
  buf += size;
  *buf = '\0';
  return buf;
}

void DecodeTime(int time, int& thours, int& tmins, int& tsecs)
{
  // Time is in milliseconds; do desk calculator style rounding.
  tsecs = (time + 500) / 1000;
  tmins = tsecs / 60;
  tsecs -= tmins * 60;
  thours = tmins / 60;
  tmins -= thours * 60;
}

void FormatTime(char *buf, int time, int sizer)
{
  int thours, tmins, tsecs;
  int shours, smins, ssecs;
  DecodeTime(time, thours, tmins, tsecs);
  DecodeTime(sizer, shours, smins, ssecs);
  if (shours > 0) {
    buf = FormatInt(buf, thours, SizeInt(shours));
    *buf++ = ':';
    buf = FormatInt(buf, tmins, 2);
  }
  else {
    buf = FormatInt(buf, tmins, SizeInt(smins));
  }
  *buf++ = ':';
  buf = FormatInt(buf, tsecs, 2);
}

void IntEvent(WinampInstance *inst, const char *event, int payload, int sizer)
{
  char buf[32];
  FormatInt(buf, payload, SizeInt(sizer));
  StringEvent(inst, event, buf);
}

void IntEvent(WinampInstance *inst, const char *event, int payload, int sizer,
              int& old)
{
  if (old == payload) return;
  old = payload;
  IntEvent(inst, event, payload, sizer);
}

void TimeEvent(WinampInstance *inst, const char *event, int payload, int sizer)
{
  char buf[128];
  FormatTime(buf, payload, sizer);
  StringEvent(inst, event, buf);
}

void TimeEvent(WinampInstance *inst, const char *event, int payload, int sizer,
               int& old)
{
  if (old == payload) return;
  old = payload;
  TimeEvent(inst, event, payload, sizer);
}

void ProcessMessage(IPC_OPCODE op, IPCMessage& msg)
{
  if (!g_bEvents) {
    if ((IPC_E_HELLO != op) && (IPC_E_GOODBYE != op))
      return;                   // Not processing normal events.
  }

  switch (op) {
  case IPC_E_HELLO:
    new WinampInstance(msg.getSource(), msg.nextString());
    return;
  case IPC_E_GOODBYE:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      StringEvent(inst, "Close", NULL);
      delete inst;
    }
    return;
  case IPC_E_START:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      int pos, len;
      msg >> pos >> len;
      IntEvent(inst, "Length", len, len, inst->m_length);
      IntEvent(inst, "Position", pos+1, len, inst->m_position);
      StringEvent(inst, "Start", "Playing");
    }
    return;
  case IPC_E_STOP:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      StringEvent(inst, "Stop", "Stopped");
    }
    return;
  case IPC_E_PAUSE:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      int elapsed, duration;
      msg >> elapsed >> duration;
      TimeEvent(inst, "Duration", duration, duration, inst->m_duration);
      TimeEvent(inst, "Elapsed", elapsed, duration, inst->m_elapsed);
      StringEvent(inst, "Pause", "Paused");
    }
    return;
  case IPC_E_RESUME:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      int elapsed, duration;
      msg >> elapsed >> duration;
      TimeEvent(inst, "Duration", duration, duration, inst->m_duration);
      TimeEvent(inst, "Elapsed", elapsed, duration, inst->m_elapsed);
      StringEvent(inst, "Resume", "Playing");
    }
    return;
  case IPC_E_SEEK:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      int elapsed, duration;
      msg >> elapsed >> duration;
      TimeEvent(inst, "Duration", duration, duration, inst->m_duration);
      TimeEvent(inst, "Elapsed", elapsed, duration, inst->m_elapsed);
    }
    return;
  case IPC_E_PLAYING:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      int elapsed, duration;
      msg >> elapsed >> duration;
      TimeEvent(inst, "Duration", duration, duration, inst->m_duration);
      TimeEvent(inst, "Elapsed", elapsed, duration, inst->m_elapsed);
    }
    return;
  case IPC_E_PLAYSTRING:
    {
      WinampInstance *inst = WinampInstance::For(msg);
      if (NULL == inst) return;
      StringEvent(inst, "Playstring", msg.nextString());
      while (TRUE) {
        const char *tag = msg.nextString();
        if (NULL == tag) break;
        StringEvent(inst, tag, msg.nextString());
      }
    }
    return;
  }
}

LRESULT CALLBACK MonitorWindow(HWND hwnd, UINT uMsg, 
                               WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_CREATE:  
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "Winamp monitor window started.\n");
    OutputDebugString(dbuf);
  }
#endif
    break;	
  case WM_DESTROY: 
#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "Winamp monitor window ended.\n");
    OutputDebugString(dbuf);
  }
#endif
    PostQuitMessage(0); 
    break;
  case WM_IPC:
    {
      IPCMessage msg(lParam);
      ProcessMessage((IPC_OPCODE)wParam, msg);
      return 0;                 // No messages with replies.
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#define WCNAME "Girder Winamp Monitor Window"

DWORD WINAPI MonitorThread(LPVOID param)
{
  WNDCLASS wclass;
  memset(&wclass, 0, sizeof(wclass));
  wclass.lpfnWndProc = MonitorWindow;
  wclass.hInstance = g_hInstance;
  wclass.lpszClassName = WCNAME;
  RegisterClass(&wclass);

  g_hMonitorWindow = CreateWindow(WCNAME, WCNAME, 0, 0, 0, 0, 0, 0, 0,
                                  g_hInstance, NULL);

  IPCMessage::Init(g_hMonitorWindow, TRUE);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    DispatchMessage(&msg);
  }

  IPCMessage::Term();

  return 0;
}

BOOL StartMonitor(BOOL events)
{
  // The monitor thread and window are always running in order to
  // rendezvous with Winamp and so be able to send it requests even
  // when events are disabled.  Is that right?
  if (events)
    g_bEvents = TRUE;

  if (g_bRunning)
    return TRUE;

  g_hMonitorThread = CreateThread(NULL, 0, MonitorThread, NULL, 0, &g_dwMonitorThreadId);
  if (NULL == g_hMonitorThread) {
    MessageBox(0, "Cannot create monitor thread.", "Error", MB_OK);
    return FALSE;
  }

  g_bRunning = TRUE;

  return TRUE;
}

void StopMonitor(BOOL events)
{
  if (events) {
    g_bEvents = FALSE;
    return;
  }

  if (!g_bRunning)
    return;

  if (NULL != g_hMonitorWindow)
    SendMessage(g_hMonitorWindow, WM_DESTROY, 0, 0);
  else
    while (!PostThreadMessage(g_dwMonitorThreadId, WM_QUIT, 0, 0))
      Sleep(100);
  WaitForSingleObject(g_hMonitorThread, INFINITE);
  CloseHandle(g_hMonitorThread);

  g_bRunning = FALSE;
}

// ivalue1 is opcode (IPC_OPCODE)
// ivalue2 is returned value subindex
// svalue1 is value (variables are expanded)
// svalue2 is variable to set
// svalue3 is prefix (default 'Winamp3')
void SendCommand(p_command command, char *status, int statuslen)
{
  const char *prefix = command->svalue3;
  if ('\0' == *prefix)
    prefix = "Winamp3";
  WinampInstance *inst = WinampInstance::Find(prefix);
  if (NULL == inst) {
    char buf[1024];
    sprintf(buf, "%s not found", prefix);
    strncpy(status, buf, statuslen);
    return;
  }
  
  IPC_OPCODE op = (IPC_OPCODE)command->ivalue1;
  IPCMessage msg(op, inst->m_handle);

  char buf[1024];
  if ('\0' == command->svalue1[0])
    buf[0] = '\0';
  else
    SF.parse_reg_string(command->svalue1, buf, sizeof(buf));

  switch (op) {
  case IPC_C_SEEK_ABSOLUTE:
  case IPC_C_SEEK_RELATIVE:
  case IPC_C_TRACK:
  case IPC_R_ITEM:
    msg.addInt(atoi(buf));
    break;
  case IPC_C_SET_NEXT:
    msg.addString(buf);
    break;
  case IPC_R_METADATA:
    {
      char *fp = strchr(buf, ',');
      if (NULL == fp) {
        msg.addString(buf);
      }
      else {
        *fp++ = '\0';
        msg.addString(buf);
        msg.addString(fp);
      }
    }
    break;
  }

  BOOL reply = FALSE;
  switch (op) {
  case IPC_R_STATE:
  case IPC_R_PLAYSTRING:
  case IPC_R_METADATA:
  case IPC_R_TRACK:
  case IPC_R_POSITION:
  case IPC_R_ITEM:
    reply = TRUE;
    break;
  }

  if (!msg.send(reply)) {
    strncpy(status, "Request failed", statuslen);
    return;
  }

  if (!reply) {
    strncpy(status, "Command sent", statuslen);
    return;
  }

  PCHAR varname = command->svalue2;

  switch (op) {
  case IPC_R_STATE:
    {
      PCHAR value;
      int state = msg.nextInt();
      if (state > 0)
        value = "Playing";
      else if (state == 0)
        value = "Stopped";
      else
        value = "Paused";
      SF.set_string_var(varname, value);
    }
    break;
  case IPC_R_PLAYSTRING:
  case IPC_R_METADATA:
  case IPC_R_ITEM:
    {
      const char *value = msg.nextString();
      if (NULL == value)
        SF.delete_var(varname);
      else
        SF.set_string_var(varname, (PCHAR)value);
    }
    break;
  case IPC_R_TRACK:
  case IPC_R_POSITION:
    {
      int n = command->ivalue2;
      int value;
      do {
        value = msg.nextInt();
      } while (n-- > 0);
      SF.set_int_var(varname, value);
    }
    break;
  }
  
  strncpy(status, "Request completed", statuslen);
}

/* Display commands 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "display.h"

class DisplayCommandState
{
public:
  p_command m_command;
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(p_command command,
                      PCHAR status, int statuslen)
    : m_command(command),
      m_status(status), m_statuslen(statuslen) 
  {
    EnterCriticalSection(&m_command->critical_section);    
  }

  ~DisplayCommandState() 
  {
    LeaveCriticalSection(&m_command->critical_section);
  }
  
  void SetStatus(LPCSTR status)
  {
    strncpy(m_status, status, m_statuslen);
  }
};

DisplayDevice *g_device = NULL;

BOOL DisplayOpen(DisplayCommandState& state)
{
  if (NULL == g_device) {
    g_device = DisplayDevice::Create(SF.parent_hwnd);
    if (NULL == g_device) {
      state.SetStatus("Could not create device.");
      return FALSE;
    }
  }

  if (!g_device->Open()) {
    state.SetStatus("Could not open device.");
    return FALSE;
  }

  return TRUE;
}

int DisplayWidth()
{
  if (NULL == g_device) {
    g_device = DisplayDevice::Create(SF.parent_hwnd);
    if (NULL == g_device)
      return 20;
  }
  return g_device->GetWidth();
}

int DisplayHeight()
{
  if (NULL == g_device) {
    g_device = DisplayDevice::Create(SF.parent_hwnd);
    if (NULL == g_device)
      return 4;
  }
  return g_device->GetHeight();
}

int DisplayGPOs()
{
  if (NULL == g_device) {
    g_device = DisplayDevice::Create(SF.parent_hwnd);
    if (NULL == g_device)
      return 0;
  }
  return g_device->GetGPOs();
}

void DisplayClose()
{
  if (NULL != g_device) {
    g_device->Close();
    delete g_device;
    g_device = NULL;
  }
}

void DisplayReopen(DisplayDevice *device)
{
  if (NULL != g_device) {
    g_device->Close();
    delete g_device;
  }
  g_device = device;
}

BOOL DisplayEnableInput()
{
  if (NULL == g_device) {
    g_device = DisplayDevice::Create(SF.parent_hwnd);
    if (NULL == g_device)
      return FALSE;
  }

  if (!g_device->GetEnableInput())
    return TRUE;                // Not enabling, so okay.

  return g_device->EnableInput();
}

void DisplayDisableInput()
{
  if (NULL != g_device)
    g_device->DisableInput();
}

void DisplaySendEvent(LPCSTR event)
{
  SF.send_event((PCHAR)event, NULL, 0, PLUGINNUM);
}

void DisplayClose(DisplayCommandState& state)
{
  if (NULL != g_device) {
    g_device->Close();
    state.SetStatus("Display closed.");
  }
}

void DisplayClear(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;
  g_device->Clear();
  state.SetStatus("Display cleared.");
}

void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) 
    return;
  g_device->Display(state.m_command->ivalue1, state.m_command->ivalue2, 
                    state.m_command->ivalue3, str);
  state.SetStatus(str);
}

void DisplayString(DisplayCommandState& state)
{
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  DisplayCommon(state, buf);
}

void DisplayVariable(DisplayCommandState& state)
{
  char buf[1024];
  int len = SF.get_string_var(state.m_command->svalue1, buf, sizeof(buf));
  if (len < 0)                  // Does not exist, expand like empty string.
    buf[0] = '\0';
  DisplayCommon(state, buf);
}

void DisplayFilename(DisplayCommandState& state)
{
  char buf[1024];
  int len = SF.get_string_var(state.m_command->svalue1, buf, sizeof(buf));
  if (len < 0)                  // Does not exist, expand like empty string.
    buf[0] = '\0';
  PCHAR sp = strrchr(buf, '\\');
  if (NULL != sp)
    sp++;
  else if (NULL != strstr(buf, "://")) // A URL
    sp = strrchr(buf, '/') + 1;
  else
    sp = buf;
  PCHAR ep = strrchr(sp, '.');
  if (NULL != ep)
    *ep = '\0';
  DisplayCommon(state, sp);
}

void DisplayCurrentTime(DisplayCommandState& state)
{
  const char *fmt = state.m_command->svalue1;
  if ((NULL == fmt) || ('\0' == *fmt))
    fmt = "%H:%M:%S";

  time_t ltime;
  struct tm *now;
  char buf[128];
  
  time(&ltime);
  now = localtime(&ltime);
  strftime(buf, sizeof(buf), fmt, now);
  DisplayCommon(state, buf);
}

void DisplayScreen(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;

  int nrows = g_device->GetHeight();

  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  for (int pass = 0; pass <= 1; pass++) { // Do marquee after others.
    PCHAR pval = buf;
    for (int i = 0; i < nrows; i++) {
      if (!(state.m_command->ivalue1 & (1 << i))) { // Enabled
        if (pass == !!(state.m_command->ivalue2 & (1 << i))) { // Marquee
          g_device->Display(i, (pass) ? -1 : 0, -1, pval);
        }
      }
      PCHAR next = strchr(pval, '\n');
      if (NULL != next)
        pval = next + 1;
      else
        pval += strlen(pval);
    }
  }
  state.SetStatus("LCD screen");
}

void DisplayGPO(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;
  g_device->SetGPO(state.m_command->ivalue1, state.m_command->bvalue1);
  state.SetStatus("GPO set");
}

void DisplayCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  char ch = (char)strtoul(buf, NULL, 0);
  g_device->DisplayCharacter(state.m_command->ivalue1, state.m_command->ivalue2, ch);
  sprintf(buf, "%c", ch);
  state.SetStatus(buf);
}

void DisplayCustomCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  CustomCharacter cust(buf);
  g_device->DisplayCustomCharacter(state.m_command->ivalue1, state.m_command->ivalue2, 
                                   cust);
  state.SetStatus("Custom character displayed");
}

void DisplayCommand(p_command command,
                    PCHAR status, int statuslen)
{
  DisplayCommandState state(command,
                            status, statuslen);
  DisplayAction *action = FindDisplayAction(command);
  if (NULL != action)
    (*action->function)(state);
}

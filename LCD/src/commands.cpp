/* Display commands 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "display.h"

LCD_API HWND LCD_DECL DisplayWindowParent()
{
  return SF.parent_hwnd;
}

LCD_API void LCD_DECL DisplayWin32Error(DWORD dwErr, HWND parent)
{
  HLOCAL pMsgBuf = NULL;
  char buf[128];
  LPSTR pMsg;
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		    NULL, dwErr, 0, (LPTSTR)&pMsgBuf, 0, NULL)) {
    pMsg = (LPSTR)pMsgBuf;
  }
  else {
    sprintf(buf, "Err: %lX", dwErr);
    pMsg = buf;
  }
  MessageBox(parent, pMsg, "Error", MB_OK | MB_ICONERROR);
  if (NULL != pMsgBuf)
    LocalFree(pMsgBuf);
}

void LCD_DECL DisplaySendEvent(LPCSTR event, LPCSTR payload)
{
  if (NULL == payload)
    SF.send_event((PCHAR)event, NULL, 0, PLUGINNUM);
  else {
    char buf[128];
    buf[0] = 1;
    strcpy(buf+1, payload);
    SF.send_event((PCHAR)event, buf, strlen(buf+1)+2, PLUGINNUM);
  }
}

/* Local variables */
static CRITICAL_SECTION g_CS;   // Ensure events see consistent device Settings.
static DisplayDeviceList g_devices;

/*** Display oriented routines ***/

void DisplayInitCS()
{
  DisplayDeviceFactory::InitCS();
  InitializeCriticalSection(&g_CS);
}

void DisplayDeleteCS()
{
  DeleteCriticalSection(&g_CS);
  DisplayDeviceFactory::DeleteCS();
}

void DisplayEnterCS()
{
  EnterCriticalSection(&g_CS);
}

void DisplayLeaveCS()
{
  LeaveCriticalSection(&g_CS);
}

void DisplayClose()
{
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext())
    device->Close();
  g_devices.Clear();
}

void DisplayUnload()
{
  DisplayDeviceFactory::CloseAll();
}

PVOID DisplaySave()
{
  return g_devices.Save();
}

void DisplayRestore(PVOID state)
{
  if (NULL == state)
    return;
  g_devices.Restore(state);
}

BOOL DisplayEnableInput()
{
  g_devices.LoadFromRegistry();
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext()) {
    if (device->GetEnableInput()) {
      if (!device->EnableInput()) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

void DisplayDisableInput()
{
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext()) {
    device->DisableInput();
  }
}

int DisplayWidth(LPCSTR devname)
{
  DisplayDevice *device = g_devices.Get(devname);
  if (NULL == device)
    return 20;
  return device->GetWidth();
}

int DisplayHeight(LPCSTR devname)
{
  DisplayDevice *device = g_devices.Get(devname);
  if (NULL == device)
    return 4;
  return device->GetHeight();
}

void DisplayClose(LPCSTR devname)
{
  DisplayDevice *device = g_devices.Get(devname);
  if (NULL != device)
    device->Close();
}

void DisplayString(int row, int col, int width, LPCSTR str, LPCSTR devname)
{
  DisplayDevice *device = g_devices.Get(devname);
  if ((NULL != device) && device->Open())
    device->Display(row, col, width, str);
}

void DisplayCustomCharacter(int row, int col, LPCSTR bits, LPCSTR devname)
{
  DisplayDevice *device = g_devices.Get(devname);
  if ((NULL != device) && device->Open())
    device->DisplayCustomCharacter(row, col, CustomCharacter(bits));
}

/*** Actual command routines ***/

class DisplayCommandState
{
public:
  p_command m_command;
  DisplayDevice *m_device;
  DisplayAction *m_action;
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(p_command command,
                      PCHAR status, int statuslen)
    : m_command(command),
      m_status(status), m_statuslen(statuslen) 
  {
    DisplayEnterCS();
    EnterCriticalSection(&m_command->critical_section);    
  }

  ~DisplayCommandState() 
  {
    LeaveCriticalSection(&m_command->critical_section);
    DisplayLeaveCS();
  }
  
  void SetStatus(LPCSTR status)
  {
    strncpy(m_status, status, m_statuslen);
  }
};

BOOL DisplayOpen(DisplayCommandState& state)
{
  if (NULL == state.m_device) {
    state.SetStatus("Could not find device.");
    return FALSE;
  }
  if (!state.m_device->Open()) {
    state.SetStatus("Could not open device.");
    return FALSE;
  }
  return TRUE;
}

void DisplayClose(DisplayCommandState& state)
{
  if (NULL == state.m_device) {
    DisplayClose();
    state.SetStatus("All displays closed.");
  }
  else {
    state.m_device->Close();
    state.SetStatus("Display closed.");
  }
}

void DisplayClear(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  state.m_device->Clear();
  state.SetStatus("Display cleared.");
}

void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) return; 
  state.m_device->Display(state.m_command->ivalue1, state.m_command->ivalue2, 
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
  if (!DisplayOpen(state)) return; 

  int nrows = state.m_device->GetHeight();

  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  for (int pass = 0; pass <= 1; pass++) { // Do marquee after others.
    PCHAR pval = buf;
    for (int i = 0; i < nrows; i++) {
      if (!(state.m_command->ivalue1 & (1 << i))) { // Enabled
        if (pass == !!(state.m_command->ivalue2 & (1 << i))) { // Marquee
          state.m_device->Display(i, (pass) ? -1 : 0, -1, pval);
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
  if (!DisplayOpen(state)) return; 
  state.m_device->SetGPO(state.m_command->ivalue1, state.m_command->bvalue1);
  state.SetStatus("GPO set");
}

void DisplayCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  char ch = (char)strtoul(buf, NULL, 0);
  state.m_device->DisplayCharacter(state.m_command->ivalue1, state.m_command->ivalue2, 
                                   ch);
  sprintf(buf, "%c", ch);
  state.SetStatus(buf);
}

void DisplayCustomCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  CustomCharacter cust(buf);
  state.m_device->DisplayCustomCharacter(state.m_command->ivalue1, state.m_command->ivalue2,
                                         cust);
  state.SetStatus("Custom character displayed");
}

void DisplayCommand(p_command command, PCHAR status, int statuslen)
{
  DisplayCommandState state(command, status, statuslen);
  if (FindDisplayAction(g_devices, command, state.m_device, state.m_action))
    (*state.m_action->function)(state);
}

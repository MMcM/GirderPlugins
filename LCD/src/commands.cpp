/* Display commands 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "display.h"

LCD_API HWND LCD_DECL DisplayWindowParent()
{
  return SF.CoreVars->MainWindow;
}

LCD_API void LCD_DECL DisplayWin32Error(DWORD dwErr, LPCSTR msg, ...)
{
  char msgbuf[2048];
  va_list args;
  va_start(args, msg);

  HLOCAL pMsgBuf = NULL;
  char xbuf[16];
  LPSTR pMsg;
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		    NULL, dwErr, 0, (LPTSTR)&pMsgBuf, 0, NULL)) {
    pMsg = (LPSTR)pMsgBuf;
  }
  else {
    _snprintf(xbuf, sizeof(xbuf), "%lX", dwErr);
    pMsg = xbuf;
  }
  _snprintf(msgbuf, sizeof(msgbuf), "Error: %s", pMsg);
  if (NULL != pMsgBuf)
    LocalFree(pMsgBuf);

  _vsnprintf(msgbuf + strlen(msgbuf), sizeof(msgbuf) - strlen(msgbuf), msg, args);
  MessageBox(DisplayWindowParent(), msgbuf, "LCD Error", MB_OK | MB_ICONERROR);
}

void LCD_DECL DisplaySendEvent(LPCSTR event, LPCSTR payload)
{
  if (NULL == payload)
    SendEventEx((PCHAR)event, NULL, 0, PLUGINNUM, EVENT_MOD_NONE);
  else {
    char buf[128];
    buf[0] = 1;
    strncpy(buf+1, payload, sizeof(buf)-1);
    SendEventEx((PCHAR)event, buf, strlen(buf+1)+2, PLUGINNUM, EVENT_MOD_NONE);
  }
}

/* Local variables */
static CRITICAL_SECTION g_CS;   // Interlock devices and their settings.
static DisplayDeviceList g_devices; // Active device(s).
static BOOL g_inputEnabled = FALSE; // Enable input whenever device is open.

class DisplayCriticalSection
{
public:
  DisplayCriticalSection()
  {
    EnterCriticalSection(&g_CS);
  }

  ~DisplayCriticalSection() 
  {
    LeaveCriticalSection(&g_CS);
  }
};

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

void DisplayBeginConfigUpdate()
{
  EnterCriticalSection(&g_CS);
  DisplayClose();
}

void DisplayEndConfigUpdate()
{
  LeaveCriticalSection(&g_CS);
}

void DisplayClose()
{
  DisplayCriticalSection cs;
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext())
    device->Close();
  g_devices.Clear();
}

void DisplayUnload()
{
  DisplayCriticalSection cs;
  DisplayDeviceFactory::CloseAll();
}

PVOID DisplaySave()
{
  DisplayCriticalSection cs;
  return g_devices.Save();
}

void DisplayRestore(PVOID state)
{
  if (NULL == state)
    return;
  DisplayCriticalSection cs;
  g_devices.Restore(state);
}

BOOL DisplayEnableInput()
{
  DisplayCriticalSection cs;
  // Enable already open devices.
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext()) {
    if (!device->IsOpen()) continue;
    if (!device->EnableInput()) {
      DisplayDisableInput();
      return FALSE;
    }
  }
  g_inputEnabled = TRUE;
  return TRUE;
}

void DisplayDisableInput()
{
  DisplayCriticalSection cs;
  g_inputEnabled = FALSE;
  for (DisplayDevice *device = g_devices.GetFirst(); 
       NULL != device; 
       device = device->GetNext()) {
    device->DisableInput();
  }
}

int DisplayWidth(LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL == device)
    return 20;
  return device->GetWidth();
}

int DisplayHeight(LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL == device)
    return 4;
  return device->GetHeight();
}

void DisplayClose(LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL != device)
    device->Close();
}

static DisplayDevice *DisplayOpen(LPCSTR devname)
{
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL == device)
    return NULL;
  if (!device->Open())
    return NULL;
  if (g_inputEnabled)
    device->EnableInput();
  return device;
}

void DisplayString(int row, int col, int width, LPCSTR str, LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = DisplayOpen(devname);
  if (NULL != device)
    device->Display(row, col, width, str);
}

void DisplayCustomCharacter(int row, int col, LPCSTR bits, LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = DisplayOpen(devname);
  if (NULL != device)
    device->DisplayCustomCharacter(row, col, CustomCharacter(bits));
}

void DisplayGPO(int gpo, BOOL on, LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = DisplayOpen(devname);
  if (NULL != device)
    device->SetGPO(gpo, on);
}

void DisplayFanPower(int fan, double power, LPCSTR devname)
{
  if (power > 1)
    power = power / 100.0;      // Must have meant percentage.

  DisplayCriticalSection cs;
  DisplayDevice *device = DisplayOpen(devname);
  if (NULL != device)
    device->SetFanPower(fan, power);
}

int DisplayGetSetting(LPCSTR key, PVOID val, size_t vlen, LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL == device)
    return -1;

  HKEY hkey = device->GetSettingsKey();
  DWORD dwType, dwLen;
  int result = -1;
  dwLen = vlen;
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, key, NULL, &dwType, (LPBYTE)val, &dwLen)) {
    switch (dwType) {
    case REG_SZ:
      result = dwLen;
      break;
    case REG_DWORD:
      result = -4;
      break;
    }
  }
  RegCloseKey(hkey);
  return result;
}

void DisplaySetSetting(LPCSTR key, PVOID val, int vlen, LPCSTR devname)
{
  DisplayCriticalSection cs;
  DisplayDevice *device = (NULL == devname) ? 
    g_devices.GetDefault() : g_devices.Get(devname);
  if (NULL == device)
    return;


  HKEY hkey = device->GetSettingsKey();
  if (vlen >= 0)
    device->SetSettingString(hkey, key, (LPCSTR)val);
  else if (vlen == -4)
    device->SetSettingInt(hkey, key, *(int*)val);
  else
    device->SetSettingString(hkey, key, NULL);

  // Only allow changes while closed.
  device->Close();
  device->LoadSettings(hkey);

  RegCloseKey(hkey);
}

/*** Actual command routines ***/

class DisplayCommandState : public DisplayCriticalSection
{
public:
  PCommand m_command;
  DisplayActionDeviceType m_devtype;
  DisplayDevice *m_device;
  DisplayAction *m_action;
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(PCommand command,
                      PCHAR status, int statuslen)
    : m_command(command),
      m_status(status), m_statuslen(statuslen) 
  {
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
  if (g_inputEnabled)
    state.m_device->EnableInput();
  return TRUE;
}

// Display Close
void DisplayClose(DisplayCommandState& state)
{
  for (DisplayDevice *dev = state.m_device; NULL != dev; dev = dev->GetNext()) {
    dev->Close();
    if (devALL != state.m_devtype) break;
  }
  state.SetStatus("Display closed.");
}

// Display Clear
void DisplayClear(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  state.m_device->Clear();
  state.SetStatus("Display cleared.");
}

// ivalue1: row
// ivalue2: col (-1 = marquee)
// ivalue3: width (-1 = rest of line)
void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) return; 
  state.m_device->Display(strtol(state.m_command->Action.iValue1, NULL, 10),
                          strtol(state.m_command->Action.iValue2, NULL, 10),
                          strtol(state.m_command->Action.iValue3, NULL, 10),
                          str);
  state.SetStatus(str);
}

// Display String
// svalue1: string to display (variables expanded).
// See DisplayCommon for position.
void DisplayString(DisplayCommandState& state)
{
  PCHAR buf = ParseString(state.m_command->Action.sValue1);
  DisplayCommon(state, buf);
  SafeFree(buf);
}

// Display Variable
// svalue1: name of variable.
// See DisplayCommon for position.
void DisplayVariable(DisplayCommandState& state)
{
  char var[128];
  var[0] = '[';
  strncpy(var+1, state.m_command->Action.sValue1, sizeof(var)-2);
  strncat(var, "]", sizeof(var));
  PCHAR buf = ParseString(var);
  DisplayCommon(state, buf);
  SafeFree(buf);
}

// Display Filename / URL Variable, trimming directory and extension.
// svalue1: name of variable.
// See DisplayCommon for position.
void DisplayFilename(DisplayCommandState& state)
{
  char buf[1024];
  buf[0] = '[';
  strncpy(buf+1, state.m_command->Action.sValue1, sizeof(buf)-2);
  strncat(buf, "]", sizeof(buf));
  PCHAR buf2 = ParseString(buf);
  strcpy(buf, buf2);
  SafeFree(buf2);
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

// Display Current Date/Time
// svalue1: format (default HH:MM:SS)
// See DisplayCommon for position.
void DisplayCurrentTime(DisplayCommandState& state)
{
  const char *fmt = state.m_command->Action.sValue1;
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

// Display Screen
// svalue1: multi-line string (variables expanded)
// ivalue1: line bitmask
// ivalue2: marquee bitmask
void DisplayScreen(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 

  int nrows = state.m_device->GetHeight();

  PCHAR buf = ParseString(state.m_command->Action.sValue1);
  DWORD enabled = strtoul(state.m_command->Action.iValue1, NULL, 10);
  DWORD marquee = strtoul(state.m_command->Action.iValue2, NULL, 10);
  for (int pass = 0; pass <= 1; pass++) { // Do marquee after others.
    PCHAR pval = buf;
    for (int i = 0; i < nrows; i++) {
      if (!(enabled & (1 << i))) {
        if (pass == !!(marquee & (1 << i))) {
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
  SafeFree(buf);
  state.SetStatus("LCD screen");
}

// Display Character (numerical code)
// svalue1: code (variables expanded)
// ivalue1: row
// ivalue2: col
void DisplayCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  PCHAR buf = ParseString(state.m_command->Action.sValue1);
  char ch = (char)strtoul(buf, NULL, 0);
  SafeFree(buf);
  state.m_device->DisplayCharacter(strtol(state.m_command->Action.iValue1, NULL, 10),
                                   strtol(state.m_command->Action.iValue2, NULL, 10),
                                   ch);
  char buf2[16];
  _snprintf(buf2, sizeof(buf2), "%c", ch);
  state.SetStatus(buf2);
}

// Display Custom Character
// svalue1: character definition (variables expanded)
// ivalue1: row
// ivalue2: col
void DisplayCustomCharacter(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  PCHAR buf = ParseString(state.m_command->Action.sValue1);
  CustomCharacter cust(buf);
  SafeFree(buf);
  state.m_device->DisplayCustomCharacter(strtol(state.m_command->Action.iValue1, NULL, 10),
                                         strtol(state.m_command->Action.iValue2, NULL, 10),
                                         cust);
  state.SetStatus("Custom character displayed");
}

// Set Keypad Legend
// svalue1: button name / code (variables expanded)
// svalue3: legend (variables expanded)
void DisplayKeypadLegend(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  PCHAR buf1 = ParseString(state.m_command->Action.sValue1);
  PCHAR buf2 = ParseString(state.m_command->Action.sValue3);
  state.m_device->SetKeypadLegend(buf1, buf2);
  SafeFree(buf2);
  SafeFree(buf1);
}

// Set General Purpose Output
// ivalue1: GPO # (1-based)
// bvalue1: on/off
void DisplayGPO(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  state.m_device->SetGPO(strtol(state.m_command->Action.iValue1, NULL, 10), 
                         StringToBool(state.m_command->Action.bValue1));
  state.SetStatus("GPO set");
}

// Set Fan Power
// ivalue1: fan # (1-based)
// svalue1: power (percentage)
void DisplayFanPower(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) return; 
  PCHAR buf = ParseString(state.m_command->Action.sValue1);
  double n = strtod(buf, NULL);
  SafeFree(buf);
  state.m_device->SetFanPower(strtol(state.m_command->Action.iValue1, NULL, 10),
                              n / 100);
  char buf2[32];
  _snprintf(buf2, sizeof(buf2), "Power %.f%%", n);
  state.SetStatus(buf2);
}

void DisplayCommand(PCommand command, PCHAR status, int statuslen)
{
  DisplayCommandState state(command, status, statuslen);
  if (FindDisplayAction(g_devices, command, 
                        state.m_devtype, state.m_device, 
                        state.m_action))
    (*state.m_action->function)(state);
}

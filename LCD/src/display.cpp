/* Display commands */

#include "stdafx.h"
#include "display.h"

HKEY DisplayDevice::GetSettingsKey()
{
  HKEY hkey;
  if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
                                    "Software\\Girder3\\SoftPlugins\\LCD", 
                                    &hkey))
    return hkey;
  else
    return NULL;
}

BOOL DisplayDevice::GetSettingString(HKEY hkey, LPCSTR valkey,
                                     LPSTR value, size_t vallen)
{
  BYTE buf[1024];
  DWORD dwType, dwLen;
  dwLen = sizeof(buf);
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, valkey, NULL, &dwType, buf, &dwLen)) {
    switch (dwType) {
    case REG_SZ:
      strncpy(value, (LPCSTR)buf, vallen);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL DisplayDevice::GetSettingInt(HKEY hkey, LPCSTR valkey, int& value)
{
  BYTE buf[32];
  DWORD dwType, dwLen;
  dwLen = sizeof(buf);
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, valkey, NULL, &dwType, buf, &dwLen)) {
    switch (dwType) {
    case REG_DWORD:
      value = (int)*(DWORD*)&buf;
      return TRUE;
    case REG_SZ:
      value = atoi((LPCSTR)buf);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL DisplayDevice::GetSettingBool(HKEY hkey, LPCSTR valkey, BOOL& value)
{
  BYTE buf[32];
  DWORD dwType, dwLen;
  dwLen = sizeof(buf);
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, valkey, NULL, &dwType, buf, &dwLen)) {
    switch (dwType) {
    case REG_DWORD:
      value = !!*(DWORD*)&buf;
      return TRUE;
    case REG_SZ:
      value = !_stricmp((LPCSTR)buf, "True");
      return TRUE;
    }
  }
  return FALSE;
}

BOOL DisplayDevice::GetSettingBinary(HKEY hkey, LPCSTR valkey,
                                     LPBYTE value, size_t vallen)
{
  BYTE buf[1024];
  DWORD dwType, dwLen;
  dwLen = sizeof(buf);
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, valkey, NULL, &dwType, buf, &dwLen)) {
    switch (dwType) {
    case REG_BINARY:
      if (dwLen > vallen)
        dwLen = vallen;
      memcpy(value, buf, dwLen);
      return TRUE;
    }
  }
  return FALSE;
}

void DisplayDevice::SetSettingString(HKEY hkey, LPCSTR valkey, LPCSTR value)
{
  if (NULL == value)
    RegDeleteValue(hkey, valkey);
  else
    RegSetValueEx(hkey, valkey, NULL, REG_SZ, (BYTE*)value, strlen(value));
}

void DisplayDevice::SetSettingInt(HKEY hkey, LPCSTR valkey, int value)
{
  RegSetValueEx(hkey, valkey, NULL, REG_DWORD, (BYTE*)&value, sizeof(int));
}

void DisplayDevice::SetSettingBool(HKEY hkey, LPCSTR valkey, BOOL value)
{
  SetSettingString(hkey, valkey, (value) ? "True" : "False");
}

void DisplayDevice::SetSettingBinary(HKEY hkey, LPCSTR valkey, 
                                     LPCBYTE value, size_t vallen)
{
  if (NULL == value)
    RegDeleteValue(hkey, valkey);
  else
    RegSetValueEx(hkey, valkey, NULL, REG_BINARY, value, vallen);
}

DisplayDevice *DisplayDevice::Create(HWND parent, LPCSTR lib, LPCSTR dev)
{
  HKEY hkey = GetSettingsKey();
  
  LPCSTR libname = "SIMLCD";
  LPCSTR devname = NULL;

  char libbuf[MAX_PATH], devbuf[256];
  if (NULL != lib)
    libname = lib;
  else if (GetSettingString(hkey, "Library", libbuf, sizeof(libbuf)))
    libname = libbuf;
  if (NULL != dev)
    devname = dev;
  else if (GetSettingString(hkey, "Device", devbuf, sizeof(devbuf)))
    devname = devbuf;
  
  HMODULE hlib = LoadLibrary(libname);
  if (NULL == hlib) {
    DisplayWin32Error(parent, GetLastError());
    return NULL;
  }

  CreateFun_t func = (CreateFun_t)GetProcAddress(hlib, "CreateDisplayDevice");
  if (NULL == hlib) {
    DisplayWin32Error(parent, GetLastError());
    return NULL;
  }
  
  DisplayDevice *device = (*func)(parent, devname);
  if (NULL == hlib)
    return NULL;

  device->LoadSettings(hkey);
  RegCloseKey(hkey);
  return device;
}

DisplayDevice::DisplayDevice()
{
  for (int i = 0; i < sizeof(m_characterMap); i++)
    m_characterMap[i] = i;

  m_open = FALSE;
  m_cols = m_rows = 0;
  m_buffer = NULL;
  memset(m_customLastUse, 0, sizeof(m_customLastUse));

  m_marquee = NULL;
  m_marqueeRow = m_marqueeLen = m_marqueePos = 0;
  m_marqueeTimer = 0;
  m_marqueePixelWidth = 6;
  m_marqueeSpeed = 1000;

  m_portType = portNONE;
  memset(&m_port, 0, sizeof(m_port));
  m_portSpeed = 0;
  m_portRTS = m_portDTR = TRUE;
  m_portHandle = NULL;

  m_contrast = m_brightness = 50;

  m_enableInput = FALSE;
  m_inputThread = m_inputStopEvent = m_outputEvent = NULL;
}

DisplayDevice::~DisplayDevice()
{
}

void DisplayDevice::LoadSettings(HKEY hkey)
{
  GetSettingBinary(hkey, "CharacterMap", m_characterMap, sizeof(m_characterMap));

  if (HasSetSize()) {
    GetSettingInt(hkey, "Cols", m_cols);
    GetSettingInt(hkey, "Rows", m_rows);
  }

  GetSettingInt(hkey, "MarqueePixelWidth", m_marqueePixelWidth);
  GetSettingInt(hkey, "MarqueeSpeed", m_marqueeSpeed);

  if (HasContrast()) {
    GetSettingInt(hkey, "Contrast", m_contrast);
  }
  if (HasBrightness()) {
    GetSettingInt(hkey, "Brightness", m_brightness);
  }
    
  if (HasKeypad()) {
    GetSettingBool(hkey, "EnableInput", m_enableInput);
  }

  switch (GetPortType()) {
  case portSERIAL:
    GetSettingInt(hkey, "PortSpeed", m_portSpeed);
    GetSettingBool(hkey, "PortRTS", m_portRTS);
    GetSettingBool(hkey, "PortDTR", m_portDTR);
    /* falls through */
  case DisplayDevice::portPARALLEL:
    GetSettingString(hkey, "Port", m_port, sizeof(m_port));
    break;
  }

  DeviceLoadSettings(hkey);
}

void DisplayDevice::SaveSettings(HKEY hkey)
{
  SetSettingBinary(hkey, "CharacterMap", m_characterMap, sizeof(m_characterMap));

  if (HasSetSize()) {
    SetSettingInt(hkey, "Cols", m_cols);
    SetSettingInt(hkey, "Rows", m_rows);
  }

  SetSettingInt(hkey, "MarqueePixelWidth", m_marqueePixelWidth);
  SetSettingInt(hkey, "MarqueeSpeed", m_marqueeSpeed);

  if (HasContrast()) {
    SetSettingInt(hkey, "Contrast", m_contrast);
  }
  if (HasBrightness()) {
    SetSettingInt(hkey, "Brightness", m_brightness);
  }

  if (HasKeypad()) {
    SetSettingBool(hkey, "EnableInput", m_enableInput);
  }

  switch (GetPortType()) {
  case portSERIAL:
    SetSettingInt(hkey, "PortSpeed", m_portSpeed);
    SetSettingBool(hkey, "PortRTS", m_portRTS);
    SetSettingBool(hkey, "PortDTR", m_portDTR);
    /* falls through */
  case DisplayDevice::portPARALLEL:
    SetSettingString(hkey, "Port", m_port);
    break;
  }

  DeviceSaveSettings(hkey);
}

BOOL DisplayDevice::Open()
{
  if (IsOpen())
    return TRUE;

  m_buffer = (LPBYTE)malloc(m_cols * m_rows);
  memset(m_buffer, m_characterMap[' '], m_cols * m_rows);
  
  switch (m_portType) {
  case portSERIAL:
    break;
  }

  if (!DeviceOpen())
    return FALSE;

  m_open = TRUE;
  return TRUE;
}

void DisplayDevice::Close()
{
  if (!IsOpen())
    return;

  m_open = FALSE;

  if (NULL != m_marquee) {
    DeviceClearMarquee();
    free(m_marquee);
    m_marquee = NULL;
  }

  DeviceClose();

  switch (m_portType) {
  case portSERIAL:
    break;
  }

  if (NULL != m_buffer) {
    free(m_buffer);
    m_buffer = NULL;
  }
}

void DisplayDevice::Clear()
{
  if (NULL != m_marquee) {
    DeviceClearMarquee();
    free(m_marquee);
    m_marquee = NULL;
  }
  DeviceClear();
  memset(m_buffer, m_characterMap[' '], m_cols * m_rows);
}

void DisplayDevice::Display(int row, int col, int width, LPCSTR str)
{
  if ((row < 0) || (row >= m_rows))
    return;

  BOOL bDrawingMarquee = FALSE;
  if (col < 0) {
    int length;
    LPCSTR endl = strchr(str, '\n');
    if (NULL == endl)
      length = strlen(str);
    else {
      length = endl - str;
      while ((length > 0) &&
             ('\r' == str[length-1]))
        length--;
    }
    if (length > m_cols) {
      // Marquee mode and text wide enough to need it.
      if (NULL != m_marquee) {
        if ((row == m_marqueeRow) &&
            (length == m_marqueeLen)) {
          BOOL match = TRUE;
          for (int i = 0; i < length; i++) {
            if (m_marquee[i] != m_characterMap[(unsigned char)str[i]]) {
              match = FALSE;
              break;
            }
          }
          if (match)
            return;             // Same as presently active.
        }
        DeviceClearMarquee();
        if (row != m_marqueeRow) {
          // Only one row allowed to do a marquee at a time; display
          // what fits of the old row in ordinary mode.
          memcpy(m_buffer + (m_marqueeRow * m_cols), m_marquee, m_cols);
          DisplayInternal(m_marqueeRow, 0, m_marquee, m_cols);
        }
        free(m_marquee);
        m_marquee = NULL;
      }
      m_marquee = (LPBYTE)malloc(length);
      for (int i = 0; i < length; i++)
        m_marquee[i] = m_characterMap[(unsigned char)str[i]];
      m_marqueeLen = length;
      m_marqueeRow = row;
      DeviceSetMarquee();
#ifdef _DEBUG
      {
        char dbuf[1024];
        sprintf(dbuf, "LCD: @%d<<<: '", row);
        char *ep = dbuf + strlen(dbuf);
        size_t nb = length;
        if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
          nb = sizeof(dbuf) - (ep - dbuf) - 3;
        memcpy(ep, m_marquee, nb);
        strcpy(ep + nb, "'\n");
        OutputDebugString(dbuf);
      }
#endif
      return;
    }
    // Fits as normal rest of line.
    col = 0;
  }
  else if (col >= m_cols)
    return;

  if ((NULL != m_marquee) && (row == m_marqueeRow)) {
    // Displaying over a previous marquee row.
    DeviceClearMarquee();
    // Display what fits of scrolling in ordinary mode.
    memcpy(m_buffer + (row * m_cols), m_marquee, m_cols);
    DisplayInternal(row, 0, m_marquee, m_cols);
    free(m_marquee);
    m_marquee = NULL;
  }

  int ncols = m_cols - col;
  if ((width <= 0) || (width > ncols))
    width = ncols;
  LPBYTE start = NULL, end;
  LPBYTE bp = m_buffer + (row * m_cols) + col;
  while (width-- > 0) {
    unsigned char ch = *str;
    if (('\0' == ch) || ('\n' == ch) || ('\r' == ch))
      ch = ' ';
    else
      str++;
    BYTE b = m_characterMap[ch];
    if (b != *bp) {
      if (NULL == start)
        start = bp;
      end = bp;
      *bp = b;
    }
    else if (NULL == start)
      col++;
    bp++;
  }
  if (NULL == start) 
    return;    // No change

  end++;
  DisplayInternal(row, col, start, (end - start));
}

void DisplayDevice::DisplayCharacter(int row, int col, char ch)
{
  if ((row < 0) || (row >= m_rows) ||
      (col < 0) || (col >= m_cols))
    return;

  if ((NULL != m_marquee) && (row == m_marqueeRow)) {
    // Displaying over a previous marquee row.
    DeviceClearMarquee();
    // Display what fits of scrolling in ordinary mode.
    memcpy(m_buffer + (row * m_cols), m_marquee, m_cols);
    DisplayInternal(row, 0, m_marquee, m_cols);
    free(m_marquee);
    m_marquee = NULL;
  }

  BYTE b = m_characterMap[(unsigned char)ch];

  LPBYTE bp = m_buffer + (row * m_cols) + col;
  if (b == *bp) 
    return;
  *bp = b;

  DisplayInternal(row, col, &b, 1);
}

void DisplayDevice::DisplayInternal(int row, int col, LPCBYTE str, int length)
{
  DeviceDisplay(row, col, str, length);

#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: @%d,%d: '", row, col);
    char *ep = dbuf + strlen(dbuf);
    size_t nb = length;
    if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
      nb = sizeof(dbuf) - (ep - dbuf) - 3;
    memcpy(ep, str, nb);
    strcpy(ep + nb, "'\n");
    OutputDebugString(dbuf);
  }
#endif
}

void DisplayDevice::SetSize(int width, int height)
{
  Close();
  m_cols = width;
  m_rows = height;
}

static DisplayDevice *g_marqueeDevice;

static void WINAPI MarqueeTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
  g_marqueeDevice->StepSimulatedMarquee();
}

void DisplayDevice::SetSimulatedMarquee()
{
  memcpy(m_buffer + (m_marqueeRow * m_cols), m_marquee, m_cols);
  DisplayInternal(m_marqueeRow, 0, m_marquee, m_cols);
  m_marqueePos = m_cols;
  g_marqueeDevice = this;
  m_marqueeTimer = SetTimer(NULL, 0, 
                            // From msec / pixel to ms / char assuming 6 pixels per char.
                            (m_marqueeSpeed * 6) / m_marqueePixelWidth,
                            MarqueeTimer);
}

void DisplayDevice::ClearSimulatedMarquee()
{
  KillTimer(NULL, m_marqueeTimer);
  m_marqueeTimer = 0;
  g_marqueeDevice = NULL;
}

void DisplayDevice::StepSimulatedMarquee()
{
  LPBYTE bp = m_buffer + (m_marqueeRow * m_cols);
  memmove(bp, bp+1, m_cols-1);
  bp[m_cols-1] = m_marquee[m_marqueePos++];
  DisplayInternal(m_marqueeRow, 0, bp, m_cols);
  if (m_marqueePos >= m_marqueeLen)
    m_marqueePos = 0;
}

void CustomCharacter::LoadFromString(LPCSTR defn)
{
  size_t index = 0;
  while (index < NCUSTCHARS) {
    // strtoul does not allow newlines, which are convenient for this.
    while (('\0' != *defn) && (NULL != strchr(" \t\r\n", *defn)))
      defn++;
    // strtoul does not allow binary, which is convenient for this.
    char *endptr;
    unsigned long bits;
    if (('0' == defn[0]) && (('b' == defn[1]) || ('B' == defn[1])))
      bits = strtoul(defn + 2, &endptr, 2);
    else
      bits = strtoul(defn, &endptr, 0);
    m_bits[index++] = (BYTE)bits;
    defn = endptr;
  }
}

void DisplayDevice::DisplayCustomCharacter(int row, int col, 
                                           const CustomCharacter& cust)
{
  int index = DefineCustomCharacter(cust);
  m_customLastUse[index] = GetTickCount();

  BYTE b = m_characterMap[index];
  m_buffer[row * m_cols + col] = b;
  DisplayInternal(row, col, &b, 1);
}

int DisplayDevice::DefineCustomCharacter(const CustomCharacter& cust)
{
  // Check for identical character already defined.
  for (int i = 0; i < NCUSTCHARS; i++) {
    if (m_customCharacters[i] == cust) {
      return i;
    }
  }

  // Reuse character not current in use or least recently used.

  int mapcust[256];
  memset(mapcust, -1, sizeof(mapcust));
  for (i = 0; i < NCUSTCHARS; i++) {
    BYTE b = m_characterMap[i];
    mapcust[b] = i;
  }

  int usage[NCUSTCHARS];           // Count current usage from display buffer.
  memset(usage, 0, sizeof(usage));
  int bufsiz = m_rows * m_cols;
  for (i = 0; i < bufsiz; i++) {
    int cust = mapcust[m_buffer[i]];
    if ((cust >= 0) && (cust < NCUSTCHARS))
      usage[cust]++;
  }

  int best = -1;
  for (i = 0; i < NCUSTCHARS; i++) {
    if (usage[i] == 0) {
      best = i;
      break;
    }
    if ((best < 0) ||
        (usage[i] < usage[best]) ||
        ((usage[i] == usage[best]) &&
         (m_customLastUse[i] < m_customLastUse[best])))
      best = i;
  }
  DeviceDefineCustomCharacter(best, cust);
  m_customCharacters[best] = cust;
  return best;
}

void DisplayDevice::Test()
{
  CustomCharacter cust1("0b11110 0b10001 0b10001 0b11110 0b10100 0b10010 0b10001");
  CustomCharacter cust2("0b01111 0b10001 0b10001 0b01111 0b00101 0b01001 0b10001");

  Display(0, 0, 10, "LCD 2.1");
  Display(1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  DisplayCustomCharacter(0, 10, cust1);
  DisplayCustomCharacter(0, 11, cust2);
  DisplayCustomCharacter(0, 12, cust1);
  DisplayCustomCharacter(0, 13, cust2);
  for (int i = 2; i < m_rows; i++) {
    Display(i, i, -1, "***");
  }
}

BOOL DisplayDevice::OpenSerial(BOOL asynch)
{
  CloseSerial();

  if (asynch)
    m_outputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  m_portHandle = CreateFile(m_port, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                            OPEN_EXISTING, (asynch) ? FILE_FLAG_OVERLAPPED : 0, NULL);
  if (NULL == m_portHandle) {
    DisplayWin32Error(NULL, GetLastError());
    return FALSE;
  }

  DCB dcb;
  if (!GetCommState(m_portHandle, &dcb)) {
    DisplayWin32Error(NULL, GetLastError());
    return FALSE;
  }
  dcb.BaudRate = m_portSpeed;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fRtsControl = (m_portRTS) ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
  dcb.fDtrControl = (m_portDTR) ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
  if (!SetCommState(m_portHandle, &dcb)) {
    DisplayWin32Error(NULL, GetLastError());
    return FALSE;
  }

  return TRUE;
}

BOOL DisplayDevice::WriteSerial(LPBYTE data, size_t len)
{
  DWORD nb;
  if (NULL == m_outputEvent)
    return WriteFile(m_portHandle, data, len, &nb, NULL);

  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = m_outputEvent;
  if (WriteFile(m_portHandle, data, len, &nb, &overlapped))
    return TRUE;
  if (ERROR_IO_PENDING != GetLastError())
    return FALSE;
  return GetOverlappedResult(m_portHandle, &overlapped, &nb, TRUE);
}

void DisplayDevice::CloseSerial()
{
  if (NULL != m_portHandle) {
    CloseHandle(m_portHandle);
    m_portHandle = NULL;
  }
  if (NULL != m_outputEvent) {
    CloseHandle(m_outputEvent);
    m_outputEvent = NULL;
  }
}

static DWORD WINAPI SerialInputThread(LPVOID lpParam)
{
  DisplayDevice *device = (DisplayDevice *)lpParam;

  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  while (TRUE) {
    HANDLE handles[2];
    int nh = 0;
    handles[nh++] = device->m_inputStopEvent;

    BYTE buf[1];
    DWORD nb;
    HANDLE ph = device->m_portHandle;
    if (NULL != ph) {
      if (ReadFile(ph, buf, sizeof(buf), &nb, &overlapped)) {
        if (nb > 0)
          device->DeviceInput(buf[0]);
        continue;
      }
      if (ERROR_IO_PENDING == GetLastError())
        handles[nh++] = overlapped.hEvent;
    }
    if (WAIT_OBJECT_0 == WaitForMultipleObjects(nh, handles, FALSE, INFINITE)) {
      if (NULL != ph)
        CancelIo(ph);
      break;
    }
    if (GetOverlappedResult(ph, &overlapped, &nb, TRUE)) {
      if (nb > 0)
        device->DeviceInput(buf[0]);
    }
  }

  CloseHandle(overlapped.hEvent);
  return 0;
}

BOOL DisplayDevice::EnableSerialInput()
{
  m_inputStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  DWORD dwThreadId;
  m_inputThread = CreateThread(NULL, 0, SerialInputThread, this, 0, &dwThreadId);
  if (NULL == m_inputThread) {
    DisplayWin32Error(NULL, GetLastError());
    return FALSE;
  }
  return TRUE;
}

void DisplayDevice::DisableSerialInput()
{
  if (NULL == m_inputThread)
    return;

  SetEvent(m_inputStopEvent);
  CloseHandle(m_inputStopEvent);
  m_inputStopEvent = NULL;

  WaitForSingleObject(m_inputThread, INFINITE);
  CloseHandle(m_inputThread);
  m_inputThread = NULL;
}

BOOL DisplayDevice::DeviceOpen()
{
  return TRUE;
}

void DisplayDevice::DeviceClose()
{
}

void DisplayDevice::DeviceClear()
{
}

void DisplayDevice::DeviceSetMarquee()
{
  SetSimulatedMarquee();
}

void DisplayDevice::DeviceClearMarquee()
{
  ClearSimulatedMarquee();
}

BOOL DisplayDevice::DeviceHasSetSize()
{
  return FALSE;
}

BOOL DisplayDevice::DeviceHasContrast()
{
  return FALSE;
}

BOOL DisplayDevice::DeviceHasBrightness()
{
  return FALSE;
}

BOOL DisplayDevice::DeviceHasBacklight()
{
  return FALSE;
}

BOOL DisplayDevice::DeviceHasKeypad()
{
  return FALSE;
}

BOOL DisplayDevice::DeviceEnableInput()
{
  return FALSE;
}

void DisplayDevice::DeviceDisableInput()
{
}

void DisplayDevice::DeviceInput(BYTE b)
{
  char buf[8];
  sprintf(buf, "%02X", b);
  DisplaySendEvent(buf);
}

int DisplayDevice::DeviceGetGPOs()
{
  return 0;
}

void DisplayDevice::DeviceSetGPO(int gpo, BOOL on)
{
}

void DisplayDevice::DeviceLoadSettings(HKEY hkey)
{
}

void DisplayDevice::DeviceSaveSettings(HKEY hkey)
{
}

LCD_API void DisplayWin32Error(HWND parent, DWORD dwErr)
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

/* Display commands 
$Header$
*/

#include "stdafx.h"
#include "display.h"

#ifdef _DEBUG
#define _TRACE
#endif

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
    RegSetValueEx(hkey, valkey, NULL, REG_SZ, (LPBYTE)value, strlen(value));
}

void DisplayDevice::SetSettingInt(HKEY hkey, LPCSTR valkey, int value)
{
  RegSetValueEx(hkey, valkey, NULL, REG_DWORD, (LPBYTE)&value, sizeof(int));
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

BOOL Delay::LoadSetting(HKEY hkey, LPCSTR valkey)
{
  BYTE buf[64];
  DWORD dwType, dwLen;
  dwLen = sizeof(buf);
  if (ERROR_SUCCESS == RegQueryValueEx(hkey, valkey, NULL, &dwType, buf, &dwLen)) {
    switch (dwType) {
    case REG_DWORD:
      // For compatibility and simplicity, an integer setting is a
      // number of milliseconds.
      SetDelay(((double)*(DWORD*)&buf) / 1000.0);
      return TRUE;
    case REG_SZ:
      if (NULL == strchr((LPCSTR)buf, '.')) {
        // Again for compatibility, a string without a decimal point
        // is milliseconds.
        SetDelay(strtod((LPCSTR)buf, NULL) / 1000.0);
      }
      else {
        // A fraction of a second.
        SetDelay(strtod((LPCSTR)buf, NULL));
      }
      return TRUE;
    }
  }
  return FALSE;
}

void Delay::SaveSetting(HKEY hkey, LPCSTR valkey) const
{
  char buf[64];
  sprintf(buf, "%g", m_delay);
  if (NULL == strchr(buf, '.'))
    strcat(buf, ".0");          // See compatibility interpretation above.
  RegSetValueEx(hkey, valkey, NULL, REG_SZ, (LPBYTE)buf, strlen(buf));
}

DisplayDevice::DisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  m_factory = factory;
  m_devtype = (NULL == devtype) ? NULL : _strdup(devtype);
  m_next = NULL;
  m_name = NULL;
  m_default = FALSE;
  m_enabled = TRUE;

  for (int i = 0; i < sizeof(m_characterMap); i++)
    m_characterMap[i] = i;

  m_cols = m_rows = 0;
  m_buffer = NULL;
  m_open = FALSE;

  m_marquee = NULL;
  m_marqueePixelWidth = 6;
  m_marqueeSpeed = 1000;
  m_marqueeTimer = 0;

  m_portType = portNONE;
  memset(m_port, 0, sizeof(m_port));
  m_portSpeed = 0;
  m_portRTS = m_portDTR = TRUE;
  m_portHandle = NULL;

  m_contrast = m_brightness = 50;

  m_enableInput = FALSE;
  m_inputThread = m_inputStopEvent = m_outputEvent = NULL;
}

DisplayDevice::DisplayDevice(const DisplayDevice& other)
{
  m_name = (NULL == other.m_name) ? NULL : _strdup(other.m_name);
  m_factory = other.m_factory;
  m_devtype = (NULL == other.m_devtype) ? NULL : _strdup(other.m_devtype);
  m_default = other.m_default;
  m_enabled = other.m_enabled;

  memcpy(m_characterMap, other.m_characterMap, sizeof(m_characterMap));

  m_cols = other.m_cols;
  m_rows = other.m_rows;
  m_buffer = NULL;
  m_open = FALSE;

  m_marquee = NULL;
  m_marqueePixelWidth = other.m_marqueePixelWidth;
  m_marqueeSpeed = other.m_marqueeSpeed;
  m_marqueeTimer = 0;

  m_portType = other.m_portType;
  memcpy(m_port, other.m_port, sizeof(m_port));
  m_portSpeed = other.m_portSpeed;
  m_portRTS = other.m_portRTS;
  m_portDTR = other.m_portDTR;
  m_portHandle = NULL;

  m_contrast = other.m_contrast;
  m_brightness = other.m_brightness;

  m_enableInput = other.m_enableInput;
  m_inputThread = m_inputStopEvent = m_outputEvent = NULL;
}

DisplayDevice::~DisplayDevice()
{
  free(m_name);
  free(m_devtype);
}

void DisplayDevice::SetName(LPCSTR name)
{
  free(m_name);
  m_name = (NULL == name) ? NULL : _strdup(name);
}

DisplayBuffer::DisplayBuffer(int rows, int cols, BYTE space)
  : m_rows(rows), m_cols(cols), m_space(space)
{
  m_bytes = (LPBYTE)malloc(rows * cols);
  memset(m_bytes, space, rows * cols);
  memset(m_customLastUse, 0, sizeof(m_customLastUse));
}

DisplayBuffer::DisplayBuffer(const DisplayBuffer& other)
  : m_rows(other.m_rows), m_cols(other.m_cols), m_space(other.m_space)
{
  m_bytes = (LPBYTE)malloc(m_rows * m_cols);
  memcpy(m_bytes, other.m_bytes, m_rows * m_cols);
  memcpy(m_customLastUse, other.m_customLastUse, sizeof(m_customLastUse));
  for (int i = 0; i < NCUSTCHARS; i++) {
    if (m_customLastUse[i] == 0)
      continue;                 // Never used.
    m_customCharacters[i] = other.m_customCharacters[i];
  }
}

DisplayBuffer::~DisplayBuffer()
{
  free(m_bytes);
}

int DisplayBuffer::FindCustomCharacter(const CustomCharacter& cust)
{
  // Check for identical character already defined.
  for (int i = 0; i < NCUSTCHARS; i++) {
    if (m_customLastUse[i] == 0)
      continue;                 // Never used; device state unknown.
    if (m_customCharacters[i] == cust)
      return i;
  }
  return -1;
}

int DisplayBuffer::AllocateCustomCharacter(const CustomCharacter& cust,
                                           LPCBYTE characterMap)
{
  // Reuse character not current in use or least recently used.

  int mapcust[256];
  memset(mapcust, -1, sizeof(mapcust));
  for (int i = 0; i < NCUSTCHARS; i++) {
    BYTE b = characterMap[i];
    mapcust[b] = i;
  }

  int usage[NCUSTCHARS];           // Count current usage from display buffer.
  memset(usage, 0, sizeof(usage));
  int bufsiz = m_rows * m_cols;
  for (i = 0; i < bufsiz; i++) {
    int cust = mapcust[m_bytes[i]];
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
  m_customCharacters[best] = cust;
  return best;
}

void DisplayBuffer::UseCustomCharacter(int index)
{
  m_customLastUse[index] = GetTickCount();
}

HKEY DisplayDevice::GetSettingsKey()
{
  HKEY root = DisplayDeviceList::GetSettingsKey();
  if (NULL == m_name)
    return root;
  HKEY subkey = NULL;
  RegOpenKey(root, m_name, &subkey);
  RegCloseKey(root);
  return subkey;
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

  m_buffer = new DisplayBuffer(m_rows, m_cols, m_characterMap[' ']);

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
    delete m_marquee;
    m_marquee = NULL;
  }

  DeviceClose();

  switch (m_portType) {
  case portSERIAL:
    break;
  }

  if (NULL != m_buffer) {
    delete m_buffer;
    m_buffer = NULL;
  }
}

void DisplayDevice::Clear()
{
  if (NULL != m_marquee) {
    DeviceClearMarquee();
    delete m_marquee;
    m_marquee = NULL;
  }
  DeviceClear();
  m_buffer->Clear();
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
      Marquee *marquee = new Marquee(row, str, length, m_characterMap);
      if (NULL != m_marquee) {
        if (*marquee == *m_marquee) {
          delete marquee;
          return;               // Same as presently active.
        }
        DeviceClearMarquee();
        if (row != m_marquee->GetRow()) {
          // Only one row allowed to do a marquee at a time; display
          // what fits of the old row in ordinary mode.
          memcpy(m_buffer->GetBuffer(m_marquee->GetRow(), 0),
                 m_marquee->GetBytes(), m_cols);
          DisplayInternal(m_marquee->GetRow(), 0, m_marquee->GetBytes(), m_cols);
        }
        delete m_marquee;
        m_marquee = NULL;
      }
      SetMarqueeInternal(marquee);
      return;
    }
    // Fits as normal rest of line.
    col = 0;
  }
  else if (col >= m_cols)
    return;

  CheckMarqueeOverlap(row);

  int ncols = m_cols - col;
  if ((width <= 0) || (width > ncols))
    width = ncols;
  LPBYTE start = NULL, end;
  LPBYTE bp = m_buffer->GetBuffer(row, col);
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

  CheckMarqueeOverlap(row);

  BYTE b = m_characterMap[(unsigned char)ch];

  LPBYTE bp = m_buffer->GetBuffer(row, col);
  if (b == *bp) 
    return;
  *bp = b;

  DisplayInternal(row, col, &b, 1);
}

void DisplayDevice::DisplayInternal(int row, int col, LPCBYTE str, int length)
{
  DeviceDisplay(row, col, str, length);

#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: @%d,%d: ", row, col);
    char *ep = dbuf + strlen(dbuf);
    if ((1 == length) && (*str < NCUSTCHARS)) {
      *ep++ = '0' + *str;
    }
    else {
      *ep++ = '\'';
      size_t nb = length;
      if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
        nb = sizeof(dbuf) - (ep - dbuf) - 3;
      memcpy(ep, str, nb);
      ep += nb;
      *ep++ = '\'';
    }
    strcpy(ep, "\n");
    OutputDebugString(dbuf);
  }
#endif
}

void DisplayDevice::DefineCustomCharacterInternal(int index, const CustomCharacter& cust)
{
  DeviceDefineCustomCharacter(index, cust);

#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: [%d] =>", index);
    char *ep = dbuf + strlen(dbuf);
    for (int i = 0; i < NCUSTROWS; i++) {
      BYTE b = cust.GetBits()[i];
      _itoa(b + (1 << NCUSTCOLS), ep+2, 2);  // Forcing leading zero.
      memcpy(ep, " 0b", 3);
      ep += (3 + NCUSTCOLS);
    }
    strcpy(ep, "\n");
    OutputDebugString(dbuf);
  }
#endif
}

void DisplayDevice::SetMarqueeInternal(Marquee *marquee)
{
  m_marquee = marquee;
  DeviceSetMarquee();
#ifdef _TRACE
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: @%d<<<: '", marquee->GetRow());
    char *ep = dbuf + strlen(dbuf);
    size_t nb = marquee->GetLength();
    if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
      nb = sizeof(dbuf) - (ep - dbuf) - 3;
    memcpy(ep, m_marquee->GetBytes(), nb);
    strcpy(ep + nb, "'\n");
    OutputDebugString(dbuf);
  }
#endif
}

void DisplayDevice::CheckMarqueeOverlap(int row)
{
  if ((NULL != m_marquee) && (row == m_marquee->GetRow())) {
    // Displaying over a previous marquee row.
    DeviceClearMarquee();
    // Display what fits of scrolling in ordinary mode.
    memcpy(m_buffer->GetBuffer(row, 0), m_marquee->GetBytes(), m_cols);
    DisplayInternal(row, 0, m_marquee->GetBytes(), m_cols);
    delete m_marquee;
    m_marquee = NULL;
  }
}

void DisplayDevice::SetSize(int width, int height)
{
  Close();
  m_cols = width;
  m_rows = height;
}

Marquee::Marquee(int row, LPCSTR str, int length, LPCBYTE characterMap)
  : m_row(row), m_len(length), m_pos(0)
{
  m_bytes = (LPBYTE)malloc(length);
  for (int i = 0; i < length; i++)
    m_bytes[i] = characterMap[(unsigned char)str[i]];
}

Marquee::Marquee(const Marquee& other)
  : m_row(other.m_row), m_len(other.m_len), m_pos(0)
{
  m_bytes = (LPBYTE)malloc(m_len);
  memcpy(m_bytes, other.m_bytes, m_len);
}

int Marquee::operator==(const Marquee& other) const
{
  if (m_row != other.m_row)
    return FALSE;
  if (m_len != other.m_len)
    return FALSE;
  return !memcmp(m_bytes, other.m_bytes, m_len);
}

Marquee::~Marquee()
{
  free(m_bytes);
}

int Marquee::NextPosition()
{
  int result = m_pos++;
  if (m_pos >= m_len)
    m_pos = 0;
  return result;
}

static DisplayDevice *g_marqueeDevice;

static void WINAPI MarqueeTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
  g_marqueeDevice->StepSimulatedMarquee();
}

void DisplayDevice::SetSimulatedMarquee()
{
  memcpy(m_buffer->GetBuffer(m_marquee->GetRow(), 0), m_marquee->GetBytes(), m_cols);
  DisplayInternal(m_marquee->GetRow(), 0, m_marquee->GetBytes(), m_cols);
  m_marquee->SetPosition(m_cols);
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
  LPBYTE bp = m_buffer->GetBuffer(m_marquee->GetRow(), 0);
  memmove(bp, bp+1, m_cols-1);
  bp[m_cols-1] = m_marquee->GetBytes()[m_marquee->NextPosition()];
  DisplayInternal(m_marquee->GetRow(), 0, bp, m_cols);
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
  CheckMarqueeOverlap(row);

  int index = DefineCustomCharacter(cust);
  BYTE b = m_characterMap[index];
  *m_buffer->GetBuffer(row, col) = b;
  DisplayInternal(row, col, &b, 1);
}

int DisplayDevice::DefineCustomCharacter(const CustomCharacter& cust)
{
  int index = m_buffer->FindCustomCharacter(cust);
  if (index < 0) {
    index = m_buffer->AllocateCustomCharacter(cust, m_characterMap);
    DefineCustomCharacterInternal(index, cust);
  }
  m_buffer->UseCustomCharacter(index);
  return index;
}

struct SaveState {
  DisplayBuffer *m_buffer;
  Marquee *m_marquee;
};

PVOID DisplayDevice::Save()
{
  if (!IsOpen())
    return NULL;

  SaveState *result = new SaveState;
  result->m_buffer = new DisplayBuffer(*m_buffer);
  result->m_marquee = (NULL == m_marquee) ? NULL : new Marquee(*m_marquee);
  return result;
}

void DisplayDevice::Restore(PVOID state)
{
  if (NULL == state) {
    Close();
    return;
  }

  if (NULL != m_marquee) {
    DeviceClearMarquee();
    delete m_marquee;
    m_marquee = NULL;
  }

  if (NULL != m_buffer) {
    delete m_buffer;
    m_buffer = NULL;
  }

  SaveState *sstate = (SaveState *)state;
  DisplayBuffer *buffer = sstate->m_buffer;
  Marquee *marquee = sstate->m_marquee;
  delete sstate;
  
  if (!m_open) {
    if (!DeviceOpen())
      return;
    m_open = TRUE;
  }

  m_buffer = buffer;
  DeviceClear();

  for (int index = 0; index < NCUSTCHARS; index++) {
    if (m_buffer->IsCustomCharacterUsed(index)) {
      DefineCustomCharacterInternal(index, m_buffer->GetCustomCharacter(index));
      m_buffer->UseCustomCharacter(index);
    }
  }

  BYTE space = m_characterMap[' '];
  for (int row = 0; row < m_rows; row++) {
    if ((NULL != marquee) &&
        (row == marquee->GetRow()))
      SetMarqueeInternal(marquee);
    else {
      int col = 0;
      LPCBYTE buf = m_buffer->GetBuffer(row, col);
      int width = m_cols;
      while ((width > 0) &&
             (space == *buf)) {
        buf++;
        col++;
        width--;
      }
      while ((width > 0) &&
             (space == buf[width-1])) {
        width--;
      }
      if (width > 0)
        DisplayInternal(row, col, buf, width);
    }
  }
}

void Delay::Wait() const
{
  if (UNKNOWN == m_method) {
    if (m_delay <= 0)
      m_method = NONE;          // No wait.
    else {
      if (m_delay < .001) {     // Fraction of a millisecond.
        LONGLONG llFreq;
        if (QueryPerformanceFrequency((LARGE_INTEGER*)&llFreq)) {
          m_arg.llCount = (LONGLONG)ceil((double)llFreq * m_delay);
          m_method = HIRES_COUNTER;
        }
        // TODO: Need some alternatives here for short delays without
        // hardware support.
      }
      if (UNKNOWN == m_method) {
        m_arg.dwMillis = (DWORD)ceil(m_delay * 1000.0);
        m_method = SLEEP;
      }
    }
  }

  switch (m_method) {
  case NONE:
    break;
  case SLEEP:
    Sleep(m_arg.dwMillis);
    break;
  case HIRES_COUNTER:
    {
      LONGLONG llCounter, llEnd;
      QueryPerformanceCounter((LARGE_INTEGER*)&llCounter);
      llEnd = llCounter + m_arg.llCount;
      do {
        QueryPerformanceCounter((LARGE_INTEGER*)&llCounter);
      }
      while (llCounter < llEnd);
    }
    break;
  }
}

void DisplayDevice::Test()
{
  CustomCharacter cust1("0b11110 0b10001 0b10001 0b11110 0b10100 0b10010 0b10001");
  CustomCharacter cust2("0b01111 0b10001 0b10001 0b01111 0b00101 0b01001 0b10001");

  Display(0, 0, 10, "LCD 3.0");
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
  if (INVALID_HANDLE_VALUE == m_portHandle) {
    DisplayWin32Error(GetLastError());
    return FALSE;
  }

  DCB dcb;
  if (!GetCommState(m_portHandle, &dcb)) {
    DisplayWin32Error(GetLastError());
    return FALSE;
  }
  dcb.BaudRate = m_portSpeed;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fRtsControl = (m_portRTS) ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
  dcb.fDtrControl = (m_portDTR) ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
  if (!SetCommState(m_portHandle, &dcb)) {
    DisplayWin32Error(GetLastError());
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
  device->DeviceSerialInputThread();
  return 0;
}

void DisplayDevice::DeviceSerialInputThread()
{
  HANDLE stopEvent = m_inputStopEvent;
  if (NULL == stopEvent) return;

  HANDLE portHandle = m_portHandle;
  if (NULL == portHandle) return;

  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  while (TRUE) {
    HANDLE handles[2];
    int nh = 0;
    handles[nh++] = stopEvent;

    // Even if the device has multi-byte input, it will need to
    // reassemble across buffer boundaries, so use the simplest
    // protocol for passing it the data.
    BYTE buf[1];
    DWORD nb;
    if (ReadFile(portHandle, buf, sizeof(buf), &nb, &overlapped)) {
      if (nb > 0)
        DeviceInput(buf[0]);
      continue;
    }
    if (ERROR_IO_PENDING == GetLastError())
      handles[nh++] = overlapped.hEvent;
    if (WAIT_OBJECT_0 == WaitForMultipleObjects(nh, handles, FALSE, INFINITE))
      break;
    if (GetOverlappedResult(portHandle, &overlapped, &nb, TRUE)) {
      if (nb > 0)
        DeviceInput(buf[0]);
    }
  }

  CancelIo(portHandle);
  CloseHandle(overlapped.hEvent);
}

BOOL DisplayDevice::EnableSerialInput()
{
  m_inputStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  DWORD dwThreadId;
  m_inputThread = CreateThread(NULL, 0, SerialInputThread, this, 0, &dwThreadId);
  if (NULL == m_inputThread) {
    DisplayWin32Error(GetLastError());
    return FALSE;
  }
  return TRUE;
}

void DisplayDevice::DisableSerialInput()
{
  if (NULL == m_inputThread)
    return;

  SetEvent(m_inputStopEvent);
  WaitForSingleObject(m_inputThread, INFINITE);

  CloseHandle(m_inputStopEvent);
  m_inputStopEvent = NULL;

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

CRITICAL_SECTION DisplayDeviceFactory::g_CS;
DisplayDeviceFactory *DisplayDeviceFactory::g_extent = NULL;

DisplayDeviceFactory::DisplayDeviceFactory(LPCSTR name, HMODULE lib, CreateFun_t entry)
  : m_lib(lib), m_entry(entry)
{
  m_name = _strdup(name);
  m_next = g_extent;
  g_extent = this;
}

DisplayDeviceFactory::~DisplayDeviceFactory()
{
  FreeLibrary(m_lib);
  free(m_name);
}

DisplayDeviceFactory *DisplayDeviceFactory::GetFactory(LPCSTR name)
{
  for (DisplayDeviceFactory *fact = g_extent; NULL != fact; fact = fact->m_next) {
    if (!_stricmp(name, fact->m_name)) {
      return fact;
    }
  }

  EnterCriticalSection(&g_CS);

  HMODULE lib = LoadLibrary(name);
  if (NULL == lib) {
    DisplayWin32Error(GetLastError());
    LeaveCriticalSection(&g_CS);
    return NULL;
  }

  CreateFun_t entry = (CreateFun_t)GetProcAddress(lib, "CreateDisplayDevice");
  if (NULL == entry) {
    DisplayWin32Error(GetLastError());
    FreeLibrary(lib);
    LeaveCriticalSection(&g_CS);
    return NULL;
  }

  fact = new DisplayDeviceFactory(name, lib, entry);
  LeaveCriticalSection(&g_CS);
  return fact;
}

void DisplayDeviceFactory::CloseAll()
{
  while (TRUE) {
    DisplayDeviceFactory *fact = g_extent;
    if (NULL == fact) break;
    g_extent = fact->m_next;
    delete fact;
  }
}

DisplayDevice *DisplayDeviceList::GetDefault()
{
  LoadFromRegistry();
  return m_head;
}

DisplayDevice *DisplayDeviceList::Get(LPCSTR name)
{
  LoadFromRegistry();
  for (DisplayDevice *dev = m_head; NULL != dev; dev = dev->m_next) {
    if (!strcmp(name, dev->GetName())) {
      return dev;
    }
  }
  return NULL;
}
  
void DisplayDeviceList::Clear()
{
  while (TRUE) {
    DisplayDevice *dev = m_head;
    if (NULL == dev) break;
    m_head = dev->m_next;
    delete dev;
  }
  m_tail = NULL;
  m_loaded = FALSE;
}

void DisplayDeviceList::Replace(DisplayDevice *odev, DisplayDevice *ndev)
{
  if (NULL == odev) {
    if (NULL == ndev) return;
    if (NULL == m_tail) {
      m_tail = m_head = ndev;
    }
    else {
      ndev->m_next = m_tail;
      m_tail = ndev;
    }
    return;
  }
  DisplayDevice *prev = NULL;
  while (TRUE) {
    DisplayDevice *next = (NULL == prev) ? m_head : prev->m_next;
    if (NULL == next) {
      // odev not found.
      Replace(NULL, ndev);
      break;
    }
    if (odev == next) {
      if (NULL == ndev) {
        if (NULL == prev) {
          m_head = m_head->m_next;
          if (NULL == m_head)
            m_tail = NULL;
        }
        else {
          prev->m_next = odev->m_next;
          if (odev == m_tail)
            m_tail = odev->m_next;
        }
      }
      else {
        if (NULL == prev) {
          m_head = ndev;
        }
        else {
          prev->m_next = ndev;
        }
        ndev->m_next = odev->m_next;
        if (odev == m_tail)
          m_tail = ndev;
      }
      break;
    }
    prev = next;
  }
  delete odev;
}

void DisplayDeviceList::SetDefault(DisplayDevice *dev)
{
  dev->SetDefault(TRUE);
  if (dev == m_head)
    return;                     // Already at head.

  if (NULL == m_head) {
    m_head = m_tail = dev;
  }
  else {
    // Remove dev if already present.
    DisplayDevice *prev = m_head;
    while (TRUE) {
      DisplayDevice *next = prev->m_next;
      if (NULL == next)
        break;
      if (dev == next) {
        prev->m_next = dev->m_next;
        break;
      }
      prev = next;
    }

    m_head->SetDefault(FALSE);
    dev->m_next = m_head;
    m_head = dev;
  }
}

void DisplayDeviceList::LoadFromRegistry(BOOL all)
{
  if (m_loaded) return;

  HKEY hkey = GetSettingsKey();
  char defname[128];
  if (!DisplayDevice::GetSettingString(hkey, NULL, defname, sizeof(defname))) {
    m_head = m_tail = LoadFromRegistry(hkey);
    m_head->SetDefault(TRUE);
  }
  else {
    DWORD index = 0;
    char subname[128];
    while (ERROR_SUCCESS == RegEnumKey(hkey, index++, subname, sizeof(subname))) {
      HKEY subkey;
      if (ERROR_SUCCESS != RegOpenKey(hkey, subname, &subkey))
        continue;
      BOOL defdev = !strcmp(defname, subname);
      BOOL enabled = TRUE;
      DisplayDevice::GetSettingBool(subkey, "Enabled", enabled);
      if (enabled || defdev || all) {
        DisplayDevice *dev = LoadFromRegistry(subkey);
        if (NULL == m_head) {
          m_head = m_tail = dev;
        }
        else if (defdev) {
          dev->m_next = m_head;
          m_head = dev;
        }
        else {
          m_tail->m_next = dev;
          m_tail = dev;
        }
        dev->SetName(subname);
        dev->SetDefault(defdev);
        dev->SetEnabled(enabled);
      }
      RegCloseKey(subkey);
    }
  }
  RegCloseKey(hkey);
  m_loaded = TRUE;
}

DisplayDevice *DisplayDeviceList::LoadFromRegistry(HKEY hkey)
{
  char libbuf[MAX_PATH], devbuf[256];
  LPCSTR libname = "SIMLCD";    // The default display type.
  LPCSTR devtype = NULL;

  if (DisplayDevice::GetSettingString(hkey, "Library", libbuf, sizeof(libbuf)))
    libname = libbuf;
  if (DisplayDevice::GetSettingString(hkey, "Device", devbuf, sizeof(devbuf)))
    devtype = devbuf;

  DisplayDeviceFactory *fact = DisplayDeviceFactory::GetFactory(libname);
  if (NULL == fact) return NULL;
  DisplayDevice *dev = fact->CreateDisplayDevice(devtype);
  dev->LoadSettings(hkey);
  return dev;
}

void DisplayDeviceList::SaveToRegistry()
{
  // Clear out all previous settings.
  SHDeleteKey(HKEY_LOCAL_MACHINE, "Software\\Girder3\\SoftPlugins\\LCD");

  HKEY hkey = GetSettingsKey();

  for (DisplayDevice *dev = m_head; NULL != dev; dev = dev->m_next) {
    LPCSTR name = dev->GetName();
    if (NULL == name) {
      SaveToRegistry(hkey, dev);
    }
    else {
      HKEY subkey;
      if (ERROR_SUCCESS == RegCreateKey(hkey, name, &subkey)) {
        if (dev->IsDefault())
          DisplayDevice::SetSettingString(hkey, NULL, name);
        DisplayDevice::SetSettingBool(subkey, "Enabled", dev->IsEnabled());
        SaveToRegistry(hkey, dev);
        RegCloseKey(subkey);
      }
    }
  }
  
  RegCloseKey(hkey);
}

void DisplayDeviceList::SaveToRegistry(HKEY hkey, DisplayDevice *dev)
{
  DisplayDevice::SetSettingString(hkey, "Library", dev->GetFactory()->GetName());
  DisplayDevice::SetSettingString(hkey, "Device", dev->GetDeviceType());
  dev->SaveSettings(hkey);
}

HKEY DisplayDeviceList::GetSettingsKey()
{
  HKEY hkey;
  if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
                                    "Software\\Girder3\\SoftPlugins\\LCD", 
                                    &hkey))
    return hkey;
  else
    return NULL;
}

PVOID DisplayDeviceList::Save()
{
  // TODO: Handle all devices.
  if (NULL == m_head)
    return NULL;
  else
    return m_head->Save();
}

void DisplayDeviceList::Restore(PVOID state)
{
  LoadFromRegistry();
  // TODO: Handle all devices.
  if (NULL != m_head)
    m_head->Restore(state);
  // Memory is lost if original no longer present.
}

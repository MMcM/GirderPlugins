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

  m_enableKeypad = m_enableFans = m_enableSensors = FALSE;
  m_fans = NULL;
  m_sensors = NULL;
  m_fanInterval = m_sensorInterval = 0;

  m_inputThread = m_inputEvent = m_inputStopEvent = m_outputEvent = NULL;
}

DisplayDevice::DisplayDevice(const DisplayDevice& other)
  : m_inputMap(other.m_inputMap)
{
  m_factory = other.m_factory;
  m_devtype = (NULL == other.m_devtype) ? NULL : _strdup(other.m_devtype);
  m_next = NULL;
  m_name = (NULL == other.m_name) ? NULL : _strdup(other.m_name);
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

  m_enableKeypad = other.m_enableKeypad;
  m_enableFans = other.m_enableFans;
  m_enableSensors = other.m_enableSensors;
  FanMonitor **pfan = &m_fans;
  for (FanMonitor *fan = other.m_fans; NULL != fan; fan = fan->GetNext()) {
    FanMonitor *nfan = new FanMonitor(*fan);
    *pfan = nfan;
    pfan = &nfan->GetNext();
  }
  *pfan = NULL;
  DOWSensor **psensor = &m_sensors;
  for (DOWSensor *sensor = other.m_sensors; NULL != sensor; sensor = sensor->GetNext()) {
    DOWSensor *nsensor = new DOWSensor(*sensor);
    *psensor = nsensor;
    psensor = &nsensor->GetNext();
  }
  *psensor = NULL;
  m_fanInterval = other.m_fanInterval;
  m_sensorInterval = other.m_sensorInterval;

  m_inputThread = m_inputEvent = m_inputStopEvent = m_outputEvent = NULL;
}

DisplayDevice::~DisplayDevice()
{
  free(m_name);
  free(m_devtype);
  while (NULL != m_fans) {
    FanMonitor *fan = m_fans;
    m_fans = fan->GetNext();
    delete fan;
  }
  DOWSensor::Destroy(m_sensors);
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
  for (int i = 0; i < MAXCUSTCHARS; i++) {
    if (m_customLastUse[i] == 0)
      continue;                 // Never used.
    m_customCharacters[i] = other.m_customCharacters[i];
  }
}

DisplayBuffer::~DisplayBuffer()
{
  free(m_bytes);
}

int DisplayBuffer::FindCustomCharacter(const CustomCharacter& cust, int ncust)
{
  // Check for identical character already defined.
  for (int i = 0; i < ncust; i++) {
    if (m_customLastUse[i] == 0)
      continue;                 // Never used; device state unknown.
    if (m_customCharacters[i] == cust)
      return i;
  }
  return -1;
}

int DisplayBuffer::AllocateCustomCharacter(const CustomCharacter& cust,
                                           int ncust,
                                           LPCBYTE characterMap)
{
  // Reuse character not current in use or least recently used.

  int mapcust[256];
  memset(mapcust, -1, sizeof(mapcust));
  for (int i = 0; i < ncust; i++) {
    BYTE b = characterMap[i];
    mapcust[b] = i;
  }

  int usage[MAXCUSTCHARS];      // Count current usage from display buffer.
  memset(usage, 0, sizeof(usage));
  int bufsiz = m_rows * m_cols;
  for (i = 0; i < bufsiz; i++) {
    int cust = mapcust[m_bytes[i]];
    if ((cust >= 0) && (cust < ncust))
      usage[cust]++;
  }

  int best = -1;
  for (i = 0; i < ncust; i++) {
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
    GetSettingBool(hkey, "EnableInput", m_enableKeypad); // Compatible value name.
    m_inputMap.LoadFromRegistry(hkey);
  }

  if (HasFans()) {
    GetSettingBool(hkey, "EnableFans", m_enableFans);
    if (IM_EDITABLE == HasFanInterval())
      GetSettingInt(hkey, "FanInterval", *(int*)&m_fanInterval);
    FanMonitor::LoadFromRegistry(hkey, &m_fans);
  }

  if (HasSensors()) {
    GetSettingBool(hkey, "EnableSensors", m_enableSensors);
    if (IM_EDITABLE == HasSensorInterval())
      GetSettingInt(hkey, "SensorInterval", *(int*)&m_sensorInterval);
    DOWSensor::LoadFromRegistry(hkey, &m_sensors);
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
    SetSettingBool(hkey, "EnableInput", m_enableKeypad); // Compatible value name.
    m_inputMap.SaveToRegistry(hkey);
  }

  if (HasFans()) {
    SetSettingBool(hkey, "EnableFans", m_enableFans);
    if (IM_EDITABLE == HasFanInterval())
      SetSettingInt(hkey, "FanInterval", m_fanInterval);
    FanMonitor::SaveToRegistry(hkey, m_fans);
  }

  if (HasSensors()) {
    SetSettingBool(hkey, "EnableSensors", m_enableSensors);
    if (IM_EDITABLE == HasSensorInterval())
      SetSettingInt(hkey, "SensorInterval", m_sensorInterval);
    DOWSensor::SaveToRegistry(hkey, m_sensors);
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
    if ((1 == length) && (*str < MAXCUSTCHARS)) {
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
  while (index < NCUSTROWS) {
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
  int ncust = DeviceNCustomCharacters();
  int index = m_buffer->FindCustomCharacter(cust, ncust);
  if (index < 0) {
    index = m_buffer->AllocateCustomCharacter(cust, ncust, m_characterMap);
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

  int ncust = DeviceNCustomCharacters();
  for (int index = 0; index < ncust; index++) {
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

void DisplayDevice::MapInput(LPCSTR input)
{
  input = m_inputMap.Get(input);
  if (NULL != input)
    DisplaySendEvent(input);
}

void DisplayDevice::ResetInputMap()
{
  // Defaults come from the device specific constructor.
  DisplayDevice *fresh = m_factory->CreateDisplayDevice(m_devtype);
  m_inputMap = fresh->m_inputMap;
  delete fresh;
}

FanMonitor *DisplayDevice::GetFan(int n, LPCSTR createPrefix)
{
  FanMonitor **prev = &m_fans;
  while (TRUE) {
    FanMonitor *fan = *prev;
    if (NULL == fan)
      break;
    if (fan->GetNumber() == n)
      return fan;
    prev = &fan->GetNext();
  }
  if (NULL == createPrefix)
    return NULL;
  char buf[32];
  sprintf(buf, "%s%d", createPrefix, n);
  FanMonitor *fan = new FanMonitor(buf, n);
  *prev = fan;
  return fan;
}

void DisplayDevice::Test()
{
  CustomCharacter cust1("0b11110 0b10001 0b10001 0b11110 0b10100 0b10010 0b10001");
  CustomCharacter cust2("0b01111 0b10001 0b10001 0b01111 0b00101 0b01001 0b10001");

  Display(0, 0, 10, "LCD 3.2");
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

  if (asynch) {
    m_outputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_inputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  }
  
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

BOOL DisplayDevice::WriteSerial(LPBYTE data, DWORD len)
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

BOOL DisplayDevice::ReadSerial(LPBYTE data, DWORD len, LPDWORD plen, DWORD timeout)
{
  if (NULL == m_inputEvent)
    return ReadFile(m_portHandle, data, len, plen, NULL);
  
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = m_inputEvent;
  if (ReadFile(m_portHandle, data, len, plen, &overlapped))
    return TRUE;
  if (ERROR_IO_PENDING != GetLastError())
    return FALSE;

  HANDLE handles[2];
  DWORD nh = 0;
  if (NULL != m_inputStopEvent)
    handles[nh++] = m_inputStopEvent;
  handles[nh++] = overlapped.hEvent;
  DWORD hn = WaitForMultipleObjects(nh, handles, FALSE, timeout);
  if (hn != nh-1)
    CancelIo(m_portHandle);     // Timeout or stopped.
  // It may still complete before CancelIo takes effect.
  if (GetOverlappedResult(m_portHandle, &overlapped, plen, FALSE))
    return TRUE;
  if (WAIT_TIMEOUT == hn) {
    *plen = 0;
    return TRUE;
  }
  return FALSE;
}

void DisplayDevice::CloseSerial()
{
  if (NULL != m_portHandle) {
    CloseHandle(m_portHandle);
    m_portHandle = NULL;
  }
  if (NULL != m_inputEvent) {
    CloseHandle(m_inputEvent);
    m_inputEvent = NULL;
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
  while (TRUE) {
    // This default just treats each input byte as keypad input.
    // Something more complicated is needed if there are multi-byte inputs
    // or polling tasks or anything like that.
    BYTE buf[1];
    DWORD nb;
    if (!ReadSerial(buf, sizeof(buf), &nb))
      break;
    if (m_enableKeypad && (nb > 0)) {
      char input[8];
      sprintf(input, "%02X", buf[0]);
      MapInput(input);
    }
  }
}

BOOL DisplayDevice::EnableSerialInput()
{
  m_inputStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  DWORD dwThreadId;
  m_inputThread = CreateThread(NULL, 0, SerialInputThread, this, 0, &dwThreadId);
  if (NULL == m_inputThread) {
    DisplayWin32Error(GetLastError());
    DisableSerialInput();
    return FALSE;
  }
  return TRUE;
}

void DisplayDevice::DisableSerialInput()
{
  if (NULL != m_inputThread) {
    SetEvent(m_inputStopEvent);
    WaitForSingleObject(m_inputThread, INFINITE);
    CloseHandle(m_inputThread);
    m_inputThread = NULL;
  }

  if (NULL != m_inputStopEvent) {
    CloseHandle(m_inputStopEvent);
    m_inputStopEvent = NULL;
  }
}

int DisplayDevice::DeviceNCustomCharacters()
{
  return MAXCUSTCHARS;
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
  return TRUE;
}

void DisplayDevice::DeviceDisableInput()
{
}

BOOL DisplayDevice::DeviceHasKeypadLegends()
{
  return FALSE;
}

LPCSTR *DisplayDevice::DeviceGetKeypadButtonChoices()
{
  return NULL;
}

LPCSTR *DisplayDevice::DeviceGetKeypadLegendChoices()
{
  return NULL;
}

void DisplayDevice::DeviceSetKeypadLegend(LPCSTR button, LPCSTR legend)
{
  m_inputMap.Put(button, legend);
}

int DisplayDevice::DeviceGetNGPOs()
{
  return 0;
}

void DisplayDevice::DeviceSetGPO(int gpo, BOOL on)
{
}

int DisplayDevice::DeviceGetNFans()
{
  return 0;
}

void DisplayDevice::DeviceSetFanPower(int fan, double dutyCycle)
{
}

DisplayDevice::IntervalMode DisplayDevice::DeviceHasFanInterval()
{
  return IM_NONE;
}

BOOL DisplayDevice::DeviceHasSensors()
{
  return FALSE;
}

DisplayDevice::IntervalMode DisplayDevice::DeviceHasSensorInterval()
{
  return IM_NONE;
}

void DisplayDevice::DeviceDetectSensors(LPCSTR prefix)
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
  
BOOL DisplayDeviceList::Contains(DisplayDevice *kdev)
{
  for (DisplayDevice *dev = m_head; NULL != dev; dev = dev->m_next) {
    if (dev == kdev) {
      return TRUE;
    }
  }
  return FALSE;
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
      m_tail->m_next = ndev;
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
            m_tail = prev;
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
        if (dev == m_tail)
          m_tail = prev;
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
    if ((NULL == m_head) && (strlen(defname) > 0)) {
      char libbuf[MAX_PATH];
      if (DisplayDevice::GetSettingString(hkey, "Library", libbuf, sizeof(libbuf))) {
        // If there is a default value that is not empty, and no child
        // keys, and what looks to be a definition in the root, load
        // it with that name.  This simplifies the transition into
        // multiple device mode while keeping an existing display
        // device.
        DisplayDevice *dev = LoadFromRegistry(hkey);
        dev->SetName(defname);
        dev->SetDefault(TRUE);
        m_head = m_tail = dev;
      }
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
  BOOL emptyMultiDev = FALSE;
  if (NULL == m_head) {
    HKEY hkey = GetSettingsKey();
    char defname[128];
    if (DisplayDevice::GetSettingString(hkey, NULL, defname, sizeof(defname))) {
      // If there are no display devices, but there is a marker for
      // multiple device mode, need to remember to put back a marker
      // to stay in that mode.
      emptyMultiDev = TRUE;
    }
    RegCloseKey(hkey);
  }

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
        SaveToRegistry(subkey, dev);
        RegCloseKey(subkey);
      }
    }
  }

  if (emptyMultiDev)
    // Write the empty string in place of the missing value.
    DisplayDevice::SetSettingString(hkey, NULL, "");
  
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

struct NamedSaveState {
  LPSTR m_name;
  PVOID m_state;
  NamedSaveState *m_next;
};

PVOID DisplayDeviceList::Save()
{
  if (NULL == m_head)
    return NULL;
  else if (NULL == m_head->m_next)
    return m_head->Save();
  else {
    NamedSaveState *nstate = NULL;
    for (DisplayDevice *dev = m_head; NULL != dev; dev = dev->m_next) {
      PVOID state = dev->Save();
      if (NULL == state) continue;
      NamedSaveState *nnstate = new NamedSaveState;
      nnstate->m_name = (NULL == dev->GetName()) ? NULL : _strdup(dev->GetName());
      nnstate->m_state = state;
      nnstate->m_next = nstate;
      nstate = nnstate;
    }
    return nstate;
  }
}

void DisplayDeviceList::Restore(PVOID state)
{
  LoadFromRegistry();
  if (NULL == m_head) return;
  if (NULL == m_head->m_next)
    m_head->Restore(state);
  else {
    NamedSaveState *nstate = (NamedSaveState *)state;
    while (NULL != nstate) {
      DisplayDevice *dev;
      if (NULL == nstate->m_name)
        dev = m_head;
      else
        dev = Get(nstate->m_name);
      if (NULL != dev)
        dev->Restore(nstate->m_state);
      NamedSaveState *nnstate = nstate->m_next;
      free(nstate->m_name);
      delete nstate;
      nstate = nnstate;
    }
  }
}

class InputMapEntry
{
  friend class InputMap;

protected:
  InputMapEntry(LPCSTR input, LPCSTR event) {
    m_input = _strdup(input);
    m_event = _strdup(event);
    m_next = NULL;
  }
  ~InputMapEntry() {
    free(m_input);
    free(m_event);
  }
  InputMapEntry(const InputMapEntry& other) {
    m_input = _strdup(other.m_input);
    m_event = (NULL == other.m_event) ? NULL : _strdup(other.m_event);
    m_next = NULL;
  }
  InputMapEntry(LPCSTR input, size_t ilen, LPCSTR event, size_t elen) {
    m_input = (LPSTR)malloc(ilen+1);
    memcpy(m_input, input, ilen);
    m_input[ilen] = '\0';
    m_event = (LPSTR)malloc(elen+1);
    memcpy(m_event, event, elen);
    m_event[elen] = '\0';
    m_next = NULL;
  }
  
  void SetEvent(LPCSTR event) {
    free(m_event);
    m_event = (NULL == event) ? NULL : _strdup(event);
  }

  LPSTR m_input, m_event;
  InputMapEntry *m_next;
};

void InputMap::Copy(const InputMap& other)
{
  m_passUnknownInput = other.m_passUnknownInput;
  InputMapEntry **pentry = &m_entries;
  for (InputMapEntry *entry = other.m_entries; NULL != entry; entry = entry->m_next) {
    InputMapEntry *nentry = new InputMapEntry(*entry);
    *pentry = nentry;
    pentry = &nentry->m_next;
  }
  *pentry = NULL;
}

LPCSTR InputMap::Get(LPCSTR input) const
{
  for (InputMapEntry *entry = m_entries; NULL != entry; entry = entry->m_next) {
    if (!strcmp(input, entry->m_input)) {
      return entry->m_event;
    }
  }
  if (m_passUnknownInput)
    return input;
  else
    return NULL;
}

void InputMap::Put(LPCSTR input, LPCSTR event)
{
  for (InputMapEntry *entry = m_entries; NULL != entry; entry = entry->m_next) {
    if (!strcmp(input, entry->m_input)) {
      entry->SetEvent(event);
      return;
    }
  }
  entry = new InputMapEntry(input, event);
  entry->m_next = m_entries;
  m_entries = entry;
}

void InputMap::Clear()
{
  while (NULL != m_entries) {
    InputMapEntry *entry = m_entries;
    m_entries = entry->m_next;
    delete entry;
  }
}

BOOL InputMap::Enum(PVOID& state, LPCSTR& input, LPCSTR& event) const
{
  InputMapEntry *entry = (NULL == state) ? m_entries : ((InputMapEntry*)state)->m_next;
  if (NULL == entry) return FALSE;
  input = entry->m_input;
  event = entry->m_event;
  state = entry;
  return TRUE;
}

void InputMap::LoadFromString(LPCSTR defn)
{
  Clear();
  InputMapEntry **pentry = &m_entries;

  int index, istart, iend, estart, eend;
  index = 0;
  istart = iend = estart = eend = -1;
  while (TRUE) {
    char ch = defn[index];
    if ((' ' == ch) || ('\t' == ch) || ('\r' == ch) || ('\n' == ch) || ('\0' == ch)) {
      if ((istart >= 0) && (iend < 0))
        iend = index;
      else if ((estart >= 0) && (eend < 0))
        eend = index;
      if (('\r' == ch) || ('\n' == ch) || ('\0' == ch)) {
        if ((iend > istart) && (eend > estart)) {
          InputMapEntry *entry = new InputMapEntry(defn + istart, iend - istart,
                                                   defn + estart, eend - estart);
          *pentry = entry;
          pentry = &entry->m_next;
        }
        if ('\0' == ch)
          break;
        istart = iend = estart = eend = -1;
      }
    }
    else {
      if (istart < 0)
        istart = index;
      else if ((iend > istart) && (estart < 0))
        estart = index;
    }
    index++;
  }

  *pentry = NULL;
}

void InputMap::LoadFromRegistry(HKEY hkey)
{
  HKEY subkey;
  if (ERROR_SUCCESS == RegOpenKey(hkey, "InputMap", &subkey)) {
    Clear();
    m_passUnknownInput = FALSE;
    InputMapEntry **pentry = &m_entries;

    char name[16], data[32];
    DWORD dwIndex, dwType, namel, datal;
    dwIndex = 0; 
    while (TRUE) {
      namel = sizeof(name); 
      datal = sizeof(data);
      if (ERROR_SUCCESS != RegEnumValue(subkey, dwIndex++, name, &namel, NULL, &dwType,
                                        (LPBYTE)data, &datal))
        break;
      if (REG_SZ == dwType) {
        if (namel > 0) {
          InputMapEntry *entry = new InputMapEntry(name, data);
          *pentry = entry;
          pentry = &entry->m_next;
        }
        else {
          m_passUnknownInput = (datal > 0);
        }
      }
    }

    *pentry = NULL;
    RegCloseKey(subkey);
  }
}

void InputMap::SaveToRegistry(HKEY hkey)
{
  RegDeleteKey(hkey, "InputMap");

  HKEY subkey;
  if (ERROR_SUCCESS == RegCreateKey(hkey, "InputMap", &subkey)) {
    DisplayDevice::SetSettingString(subkey, NULL,
                                    m_passUnknownInput ? "*" : NULL);
    for (InputMapEntry *entry = m_entries; NULL != entry; entry = entry->m_next) {
      DisplayDevice::SetSettingString(subkey, entry->m_input, entry->m_event);
    }
    RegCloseKey(subkey);
  }
}

FanMonitor::FanMonitor(LPCSTR name, int number)
{
  m_name = _strdup(name);
  m_number = number;
  m_ppr = 2;
  m_enabled = FALSE;
  m_anonymous = TRUE;
  m_updateTime = 0;
  m_next = NULL;
}

FanMonitor::FanMonitor(const FanMonitor& other)
{
  m_name = _strdup(other.m_name);
  m_number = other.m_number;
  m_ppr = other.m_ppr;
  m_enabled = other.m_enabled;
  m_anonymous = other.m_anonymous;
  m_updateTime = 0;
  m_next = NULL;
}

FanMonitor::~FanMonitor()
{
  free(m_name);
}

void FanMonitor::SetName(LPCSTR name)
{
  free(m_name);
  m_name = _strdup(name);
  m_anonymous = FALSE;
}

void FanMonitor::SetEnabled(BOOL enabled)
{
  m_enabled = enabled;
  m_anonymous = FALSE;
}

void FanMonitor::LoadFromRegistry(HKEY hkey, FanMonitor **fans)
{
  HKEY subkey;
  if (ERROR_SUCCESS == RegOpenKey(hkey, "Fans", &subkey)) {
    while (NULL != *fans) {
      FanMonitor *fan = *fans;
      *fans = fan->GetNext();
      delete fan;
    }

    int ppr = 0;

    char name[128], data[32];
    DWORD dwIndex, dwType, namel, datal;
    dwIndex = 0; 
    while (TRUE) {
      namel = sizeof(name); 
      datal = sizeof(data);
      if (ERROR_SUCCESS != RegEnumValue(subkey, dwIndex++, name, &namel, NULL, &dwType,
                                        (LPBYTE)data, &datal))
        break;
      if (REG_SZ == dwType) {
        if (namel == 0) {
          ppr = atoi(data);
        }
        else {
          FanMonitor *fan = new FanMonitor(name, data);
          if (ppr > 0) fan->SetPulsesPerRevolution(ppr);
          *fans = fan;
          fans = &fan->m_next;
        }
      }
    }

    *fans = NULL;
    
    RegCloseKey(subkey);
  }
}

void FanMonitor::SaveToRegistry(HKEY hkey, FanMonitor *fans)
{
  HKEY subkey;
  if (ERROR_SUCCESS == RegCreateKey(hkey, "Fans", &subkey)) {
    for (FanMonitor *fan = fans; NULL != fan; fan = fan->m_next) {
      if (fan->m_anonymous) continue;

      char buf[32];
      LPSTR pb = buf;
      if (!fan->m_enabled)
        *pb++ = '*';
      sprintf(pb, "%d", fan->m_number);
      DisplayDevice::SetSettingString(subkey, fan->m_name, buf);
    }
    if (NULL != fans) {
      char buf[16];
      sprintf(buf, "%d", fans->GetPulsesPerRevolution());
      DisplayDevice::SetSettingString(subkey, NULL, buf);
    }
    RegCloseKey(subkey);
  }
}

FanMonitor::FanMonitor(LPCSTR name, LPCSTR entry)
{
  m_name = _strdup(name);
  m_updateTime = 0;
  m_next = NULL;

  if ('*' == *entry) {
    m_enabled = FALSE;
    entry++;
  }
  else
    m_enabled = TRUE;
  m_anonymous = FALSE;
  m_number = atoi(entry);
  m_ppr = 2;
}

DOWSensor::DOWSensor(LPCSTR name, LPCBYTE rom)
{
  m_name = _strdup(name);
  memcpy(m_rom, rom, sizeof(m_rom));
  m_enabled = TRUE;
  m_value = NULL;
  m_updateTime = 0;
  m_next = NULL;
}

DOWSensor::DOWSensor(const DOWSensor& other)
{
  m_name = _strdup(other.m_name);
  memcpy(m_rom, other.m_rom, sizeof(m_rom));
  m_enabled = other.m_enabled;
  m_value = NULL;
  m_updateTime = 0;
  m_next = NULL;
}

DOWSensor::~DOWSensor()
{
  free(m_name);
  free(m_value);
}

void DOWSensor::SetName(LPCSTR name)
{
  free(m_name);
  m_name = _strdup(name);
}

void DOWSensor::Clear()
{
  free(m_value);
  m_value = NULL;
  m_updateTime = GetTickCount();
}

BOOL DOWSensor::LoadFromScratchpad(LPCBYTE pb, size_t nb)
{
  char buf[128];
  LPCSTR newvalue = NULL;

  switch (m_rom[0]) {
  case 0x10:                    // DS18S20
    if (nb >= 2) {
      double temp = ((double)*(SHORT UNALIGNED*)pb) / 2.0;
      if (nb >= 8) {
        temp = temp - 0.25 + ((pb[7] - pb[6]) / pb[7]);
      }
      sprintf(buf, "%.1f", temp);
      newvalue = buf;
    }
    break;
  case 0x22:                    // DS1822
  case 0x28:                    // DS18B20
    if (nb >= 2) {
      double temp = ((double)*(SHORT UNALIGNED*)pb) / 16.0;
      sprintf(buf, "%.1f", temp);
      newvalue = buf;
    }
    break;
  }

  m_updateTime = GetTickCount();
  
  if (NULL == newvalue) {
    if (NULL == m_value)
      return FALSE;
    free(m_value);
    m_value = NULL;
    return TRUE;
  }
  if ((NULL != m_value) && !strcmp(newvalue, m_value))
    return FALSE;
  free(m_value);
  m_value = _strdup(newvalue);
  return TRUE;
}

void DOWSensor::LoadFromRegistry(HKEY hkey, DOWSensor **sensors)
{
  HKEY subkey;
  if (ERROR_SUCCESS == RegOpenKey(hkey, "Sensors", &subkey)) {
    Destroy(*sensors);

    char name[128], data[32];
    DWORD dwIndex, dwType, namel, datal;
    dwIndex = 0; 
    while (TRUE) {
      namel = sizeof(name); 
      datal = sizeof(data);
      if (ERROR_SUCCESS != RegEnumValue(subkey, dwIndex++, name, &namel, NULL, &dwType,
                                        (LPBYTE)data, &datal))
        break;
      if (REG_SZ == dwType) {
        if (namel > 0) {
          DOWSensor *sensor = new DOWSensor(name, data);
          *sensors = sensor;
          sensors = &sensor->m_next;
        }
      }
    }

    *sensors = NULL;
    
    RegCloseKey(subkey);
  }
}

DOWSensor::DOWSensor(LPCSTR name, LPCSTR entry)
{
  m_name = _strdup(name);
  m_value = NULL;
  m_updateTime = 0;
  m_next = NULL;

  char buf[32];
  strncpy(buf, entry, sizeof(buf));
  LPSTR pb = buf;

  if ('*' == *pb) {
    m_enabled = FALSE;
    pb++;
  }
  else
    m_enabled = TRUE;
  for (int i = 7; i >= 0; i--) {
    m_rom[i] = (BYTE)strtoul(pb + i * 2, NULL, 16);
    pb[i * 2] = '\0';
  }
}

void DOWSensor::SaveToRegistry(HKEY hkey, DOWSensor *sensors)
{
  HKEY subkey;
  if (ERROR_SUCCESS == RegCreateKey(hkey, "Sensors", &subkey)) {
    for (DOWSensor *sensor = sensors; NULL != sensor; sensor = sensor->m_next) {
      char buf[32];
      LPSTR pb = buf;
      if (!sensor->m_enabled)
        *pb++ = '*';
      for (int i = 0; i < 8; i++)
        sprintf(pb + i * 2, "%02X", sensor->m_rom[i]);
      DisplayDevice::SetSettingString(subkey, sensor->m_name, buf);
    }
    RegCloseKey(subkey);
  }
}

// Determine a basis for naming newly detected sensors.  Want a number
// at least as large as the number already named, but also larger than
// any name ending with a number already.
int DOWSensor::GetNewNameIndex(LPCSTR prefix, DOWSensor *sensors)
{
  int n = 0;
  int m = 0;
  for (DOWSensor *sensor = sensors; NULL != sensor; sensor = sensor->m_next) {
    n++;
    if (!_strnicmp(sensor->GetName(), prefix, strlen(prefix))) {
      char *endptr = NULL;
      long s = strtol(sensor->GetName() + strlen(prefix), &endptr, 10);
      if (*endptr == '\0') {
        if (m < s)
          m = s;
      }
    }
  }
  if (n < m)
    n = m;
  return n;
}

// These next couple also ensure that the right heap is used.

DOWSensor *DOWSensor::Create(LPCSTR prefix, int index, LPBYTE rom)
{
  char name[128];
  sprintf(name, "%s%d", prefix, index);
  return new DOWSensor(name, rom);
}

void DOWSensor::Destroy(DOWSensor *sensors)
{
  while (NULL != sensors) {
    DOWSensor *sensor = sensors;
    sensors = sensor->m_next;
    delete sensor;
  }
}


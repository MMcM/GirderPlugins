/* Matrix Orbital device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

// See http://www.matrixorbital.com/products.htm for list of
// supported displays.

typedef BYTE (*BrightnessFunc_t)(int);

// This is used by LCDs for backlight brightness, except older models
// that do not have any way of setting it.
// Some VFD manuals (e.g. VK-240-24-USB sect. 8.8) claim to use this,
// but don't.
static BYTE BrightnessContinuous(int percent)
{
  return (percent * 0xFF) / 100;
}

// This is what most VFD manuals claim for brightness.
// None seem to actually use it.
static BYTE BrightnessSteppedUp(int percent)
{
  if (percent <= 25)
    return 0;
  else if (percent <= 50)
    return 1;
  else if (percent <= 75)
    return 2;
  else
    return 3;
}

// This is what VFDs actually seem to use.
// Only the VK-204-25 claims this.
static BYTE BrightnessSteppedDown(int percent)
{
  if (percent > 75)
    return 0;
  else if (percent > 50)
    return 1;
  else if (percent > 25)
    return 2;
  else
    return 3;
}

struct DeviceEntry {
  const char *devtype;
  int cols;
  int rows;
  BOOL vfd;
  BrightnessFunc_t brightnessFunc;
  BOOL keypad;
  int gpos;
  BOOL dow_pwm;
} DeviceEntries[] = {
  { "LCD2021", 20, 2, FALSE, NULL, FALSE, 1 },
  { "LCD4021", 40, 2, FALSE, NULL, FALSE, 1 },
  { "LCD2041", 20, 4, FALSE, NULL, FALSE, 1 },
  { "LCD4041", 40, 4, FALSE, NULL, FALSE, 1 },
  { "LK162", 16, 2, FALSE, NULL, TRUE, 6 },
  { "LK202", 20, 2, FALSE, NULL, TRUE, 6 },
  { "LK204", 20, 4, FALSE, NULL, TRUE, 6 },
  { "LK204PC", 20, 4, FALSE, BrightnessContinuous, TRUE, 7, TRUE },
  { "LK202U", 20, 2, FALSE, BrightnessContinuous, TRUE, 7, TRUE },
  { "LK204U", 20, 4, FALSE, BrightnessContinuous, TRUE, 7, TRUE },
  { "LK402", 40, 2, FALSE, NULL, TRUE, 7 },
  { "LK404", 40, 4, FALSE, NULL, TRUE, 1 },
  { "VFD2021", 20, 2, TRUE, BrightnessSteppedDown, FALSE, 1 },
  { "VFD2041", 20, 4, TRUE, BrightnessSteppedDown, FALSE, 1 },
  { "VK202", 20, 2, TRUE, BrightnessSteppedDown, TRUE, 6 },
  { "VK204", 20, 4, TRUE, BrightnessSteppedDown, TRUE, 6 },
  { "VK202U", 20, 2, TRUE, BrightnessSteppedDown, TRUE, 7, TRUE },
  { "VK204U", 20, 4, TRUE, BrightnessSteppedDown, TRUE, 7, TRUE },
};

#define countof(x) sizeof(x)/sizeof(x[0])

const char *KEYPAD_7 = 
  "E	Right\n"
  "H	Enter\n"
  "I	Up\n"
  "J	Down\n"
  "M	F2\n"
  "N	F1\n"
  "O	Left\n"
  ;
const char *KEYPAD_15 = 
  "C	F2\n"
  "D	Play\n"
  "E	FF\n"
  "F	Enter\n"
  "I	F3\n"
  "J	Up\n"
  "K	Down\n"
  "O	F1\n"
  "P	Rew\n"
  "Q	Stop\n"
  "R	Right\n"
  "U	MatrixOrbital\n"
  "V	Prev\n"
  "W	Next\n"
  "X	Left\n"
  ;

struct DisplayReturnPacket {
  BYTE preamble[2];
  BYTE size;
  BYTE type;
  BYTE data[1];
};

const BYTE CONT = 0x80;

class MatrixOrbitalDisplay : public DisplayDevice
{
public:
  MatrixOrbitalDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  MatrixOrbitalDisplay(const MatrixOrbitalDisplay& other);
  ~MatrixOrbitalDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasKeypad();
  virtual BOOL DeviceEnableInput();
  virtual void DeviceDisableInput();
  virtual void DeviceSerialInputThread();
  virtual int DeviceGetNGPOs();
  virtual void DeviceSetGPO(int gpo, BOOL on);
  virtual int DeviceGetNFans();
  virtual void DeviceSetFanPower(int fan, double dutyCycle);
  virtual IntervalMode DeviceHasFanInterval();
  virtual BOOL DeviceHasSensors();
  virtual IntervalMode DeviceHasSensorInterval();
  virtual void DeviceDetectSensors(LPCSTR prefix);
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
  DWORD CheckFans();
  DWORD CheckSensors();
  void UpdateSensors(BOOL detect);
  DisplayReturnPacket *ReadReturnPacket();

  BOOL m_vfd, m_keypad, m_dow_pwm;
  BrightnessFunc_t m_brightnessFunc;
  int m_gpos;
  int m_debounceTime;
};

MatrixOrbitalDisplay::MatrixOrbitalDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devtype, entry->devtype)) {
      m_cols = entry->cols;
      m_rows = entry->rows;
      m_vfd = entry->vfd;
      m_keypad = entry->keypad;
      if (m_keypad)
        // You can hook any kind of keypad membrane.  But the standard
        // drive bay inserts have these two arrangements for one bay
        // and two.
        m_inputMap.LoadFromString((m_rows < 4) ? KEYPAD_7 : KEYPAD_15);
      m_dow_pwm = entry->dow_pwm;
      m_brightnessFunc = entry->brightnessFunc;
      m_gpos = entry->gpos;
      break;
    }
  }
  m_portType = portSERIAL;
  strcpy(m_port, "COM1");
  m_portSpeed = CBR_19200;

  // This is pretty much the minimum to (1) get back the two
  // ISO-8859-1L characters in funny places, (2) prevent damage from
  // displaying an FE being interpreted as some command, and (3) null
  // out formatting control characters (BS, CR, etc.)
  m_characterMap['\\'] = 0x8C;
  m_characterMap[0x8C] = '\\';
  m_characterMap['~'] = 0x8E;
  m_characterMap[0x8E] = '~';
  m_characterMap[0xFE] = ' ';
  for (i = 8; i < 16; i++)
    m_characterMap[i] = ' ';

  m_debounceTime = 52;

  m_fanInterval = m_sensorInterval = 5000;
}

MatrixOrbitalDisplay::MatrixOrbitalDisplay(const MatrixOrbitalDisplay& other)
  : DisplayDevice(other)
{
  m_vfd = other.m_vfd;
  m_keypad = other.m_keypad;
  m_dow_pwm = other.m_dow_pwm;
  m_brightnessFunc = other.m_brightnessFunc;
  m_gpos = other.m_gpos;
  m_debounceTime = other.m_debounceTime;
}

MatrixOrbitalDisplay::~MatrixOrbitalDisplay()
{
}

DisplayDevice *MatrixOrbitalDisplay::Duplicate() const
{
  return new MatrixOrbitalDisplay(*this);
}

BOOL MatrixOrbitalDisplay::DeviceOpen()
{
  if (!OpenSerial(m_enableKeypad || m_enableFans || m_enableSensors))
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x42;             // Display on ...
  buf[nb++] = 0x00;             // ... permanently
  buf[nb++] = 0xFE;
  buf[nb++] = 0x44;             // Auto line wrap off
  buf[nb++] = 0xFE;
  buf[nb++] = 0x52;             // Auto scroll off
  buf[nb++] = 0xFE;
  buf[nb++] = 0x4B;             // Underline cursor off
  buf[nb++] = 0xFE;
  buf[nb++] = 0x54;             // Block cursor off
  if (m_vfd) {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x59;           // Set brightness ...
    buf[nb++] = (*m_brightnessFunc)(m_brightness);
  }
  else {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x50;           // Set contrast ...
    buf[nb++] = BrightnessContinuous(m_contrast);
    if (NULL != m_brightnessFunc) {
      buf[nb++] = 0xFE;
      buf[nb++] = 0x99;           // Set backlight brightness ...
      buf[nb++] = (*m_brightnessFunc)(m_brightness);
    }
  }
  if (m_keypad) {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x55;           // Set debounce time ...
    buf[nb++] = (BYTE)((double)m_debounceTime / 6.554 + 0.5);
  }
  buf[nb++] = 0xFE;
  buf[nb++] = 0x58;             // Clear display
  WriteSerial(buf, nb);

  return TRUE;
}

void MatrixOrbitalDisplay::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x58;             // Clear display
  buf[nb++] = 0xFE;
  buf[nb++] = 0x46;             // Display off
  WriteSerial(buf, nb);

  CloseSerial();
}

void MatrixOrbitalDisplay::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x58;             // Clear display
  WriteSerial(buf, nb);
}

void MatrixOrbitalDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  if ((row == 0) && (col == 0)) {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x48;           // Send cursor home
  }
  else {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x47;           // Set cursor position ...
    buf[nb++] = col + 1;
    buf[nb++] = row + 1;
  }
  for (int i = 0; i < length; i++) {
    BYTE b = str[i];
    // There is no way to send an FE to DDRAM.  But that character is blank anyway.
    buf[nb++] = b;
  }
  WriteSerial(buf, nb);
}

void MatrixOrbitalDisplay::DeviceDefineCustomCharacter(int index, 
                                                       const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x4E;             // Define custom character ...
  buf[nb++] = index;
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
}

BOOL MatrixOrbitalDisplay::DeviceHasContrast()
{
  return !m_vfd;
}

BOOL MatrixOrbitalDisplay::DeviceHasBrightness()
{
  return (NULL != m_brightnessFunc);
}

BOOL MatrixOrbitalDisplay::DeviceHasKeypad()
{
  return m_keypad;
}

BOOL MatrixOrbitalDisplay::DeviceEnableInput()
{
  if (!m_enableKeypad && !m_enableFans && !m_enableSensors)
    return TRUE;

  if (!Open()) return FALSE;    // Get a port handle.
  return EnableSerialInput();
}

void MatrixOrbitalDisplay::DeviceDisableInput()
{
  DisableSerialInput();
}

void MatrixOrbitalDisplay::DeviceSerialInputThread()
{
  while (TRUE) {
    DWORD ftime = CheckFans();
    DWORD stime = CheckSensors();

    DWORD now = GetTickCount();
    DWORD timeout = INFINITE;

    if (INFINITE != ftime) {
      if (ftime <= now) continue;
      DWORD delta = ftime - now;
      if (timeout > delta)
        timeout = delta;
    }
    if (INFINITE != stime) {
      if (stime <= now) continue;
      DWORD delta = stime - now;
      if (timeout > delta)
        timeout = delta;
    }

    BYTE buf[1];
    DWORD nb;
    if (!ReadSerial(buf, sizeof(buf), &nb, timeout))
      break;
    if (m_enableKeypad && (nb > 0)) {
      char input[2];
      input[0] = buf[0];
      input[1] = '\0';
      MapInput(input);
    }
  }
}

int MatrixOrbitalDisplay::DeviceGetNGPOs()
{
  return m_gpos;
}

void MatrixOrbitalDisplay::DeviceSetGPO(int gpo, BOOL on)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = (on) ? 0x57 : 0x56; // General purpose output on/off ...
  if (m_gpos > 0)
    buf[nb++] = gpo;
  WriteSerial(buf, nb);
}

int MatrixOrbitalDisplay::DeviceGetNFans()
{
  if (m_dow_pwm)
    return 4;
  else
    return 0;
}

void MatrixOrbitalDisplay::DeviceSetFanPower(int fan, double dutyCycle)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0xC0;             // PWM Value
  buf[nb++] = fan;
  int pwm;
  if (dutyCycle < 0.0)
    pwm = 0;
  else if (dutyCycle > 1.0)
    pwm = 255;
  else
    pwm = (BYTE)(dutyCycle * 255.0);
  buf[nb++] = pwm;
  WriteSerial(buf, nb);
}

DisplayDevice::IntervalMode MatrixOrbitalDisplay::DeviceHasFanInterval()
{
  return (m_dow_pwm) ? IM_EDITABLE : IM_NONE;
}

DWORD MatrixOrbitalDisplay::CheckFans()
{
  // TODO: add monitoring.
  return INFINITE;
}

BOOL MatrixOrbitalDisplay::DeviceHasSensors()
{
  return m_dow_pwm;
}

DisplayDevice::IntervalMode MatrixOrbitalDisplay::DeviceHasSensorInterval()
{
  return (m_dow_pwm) ? IM_EDITABLE : IM_NONE;
}

void MatrixOrbitalDisplay::DeviceDetectSensors(LPCSTR prefix)
{
  if (!Open()) return;          // Get a port handle.

  int n = DOWSensor::GetNewNameIndex(prefix, m_sensors);

  DOWSensor *oldSensors = m_sensors;
  DOWSensor **psensor = &m_sensors;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0xC8;             // 1-wire
  buf[nb++] = 2;                // Search
  WriteSerial(buf, nb);
  
  while (TRUE) {
    DisplayReturnPacket *drp = ReadReturnPacket();
    if (NULL == drp) break;
    if ((0x31 == drp->type) && (10 == (drp->size & ~CONT)) &&
        (0x00 == drp->data[0]) && (0x00 == drp->data[9])) {
      DOWSensor *sensor = NULL;
      DOWSensor **prev = &oldSensors;
      while (TRUE) {
        sensor = *prev;
        if (NULL == sensor) break;
        if (!memcmp(sensor->GetROM(), drp->data+1, 8)) {
          *prev = sensor->GetNext(); // Unlink and reuse.
          break;
        }
        prev = &sensor->GetNext();
      }
      if (NULL == sensor)
        sensor = DOWSensor::Create(prefix, ++n, drp->data+1);
      *psensor = sensor;
      psensor = &sensor->GetNext();
    }
    BOOL cont = ((drp->size & CONT) != 0);
    free(drp);
    if (!cont) break;
  }

  *psensor = NULL;
  
  DOWSensor::Destroy(oldSensors);

  UpdateSensors(TRUE);
  Close();
}

DWORD MatrixOrbitalDisplay::CheckSensors()
{
  // Since we process them all in one broadcast, we just need to find
  // the first enabled sensor to get the update time.
  DOWSensor *sensor = m_sensors;
  while ((NULL != sensor) && !sensor->IsEnabled())
    sensor = sensor->GetNext();
  if (NULL == sensor)
    return INFINITE;

  DWORD nextUpdate = sensor->GetUpdateTime() + m_sensorInterval;
  if (nextUpdate > GetTickCount())
    return nextUpdate;
  
  UpdateSensors(FALSE);

  // Just in case the update didn't take, we return a time relative to
  // now so that we are sure of waiting.
  return GetTickCount() + m_sensorInterval;
}

void MatrixOrbitalDisplay::UpdateSensors(BOOL detect)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0xC8;             // 1-wire
  buf[nb++] = 1;                // Transaction
  buf[nb++] = 0x01;             // Reset bus, no CRC
  buf[nb++] = 16;               // Send 16 bits
  buf[nb++] = 0;                // Read 0 bits
  buf[nb++] = 0xCC;             // Skip ROM
  buf[nb++] = 0x44;             // Convert T
  WriteSerial(buf, nb);
  
  DisplayReturnPacket *drp = ReadReturnPacket();
  if (NULL == drp) return;
  free(drp);

  for (DOWSensor *sensor = m_sensors; NULL != sensor; sensor = sensor->GetNext()) {
    if (!detect && !sensor->IsEnabled()) continue;

    nb = 0;
    buf[nb++] = 0xFE;
    buf[nb++] = 0xC8;           // 1-wire
    buf[nb++] = 1;              // Transaction
    buf[nb++] = 0x03;           // Reset bus, CRC
    buf[nb++] = 80;             // Send 80 bits
    buf[nb++] = 72;             // Read 72 bits
    buf[nb++] = 0x55;           // Match ROM
    memcpy(buf+nb, sensor->GetROM(), 8); // ROM
    nb += 8;
    buf[nb++] = 0xBE;             // Read Scratchpad
    WriteSerial(buf, nb);
  
    drp = ReadReturnPacket();
    if (NULL == drp) continue;
    if ((0x31 == drp->type) && (11 == (drp->size & ~CONT)) &&
        (0x00 == drp->data[0]) && (0x00 == drp->data[10])) {
      if (sensor->LoadFromScratchpad(drp->data+1, 9)) {
        if (!detect) {
          DisplaySendEvent(sensor->GetName(), sensor->GetValue());
        }
      }
    }    
    free(drp);
  }
}

void MatrixOrbitalDisplay::DeviceLoadSettings(HKEY hkey)
{
  if (m_keypad)
    GetSettingInt(hkey, "DebounceTime", m_debounceTime);
}

void MatrixOrbitalDisplay::DeviceSaveSettings(HKEY hkey)
{
  if (m_keypad)
    SetSettingInt(hkey, "DebounceTime", m_debounceTime);
}

DisplayReturnPacket *MatrixOrbitalDisplay::ReadReturnPacket()
{
  // We are expecting a packet response.  Therefore there are at least
  // 4 bytes waiting.  It is also possible that the user pressed a
  // keypad button just as our request was being sent out.  In that
  // case, that byte is ahead of the actual packet and needs to be
  // dealt with.  Additionally, this routine can be called from the
  // main thread or a UI thread or the input thread.  In the last
  // case, it must deal with a stop request from the main thread.  In
  // the other cases, no input thread is running (yet), so no
  // synchronization there is needed.
  BYTE hdr[4];
  size_t off = 0;
  while (off < sizeof(hdr)) {
    DWORD nb = sizeof(hdr) - off;
    if (!ReadSerial(hdr + off, nb, &nb, 100))
      return NULL;
    while ((off == 0) && (nb > 0)) {
      BYTE b = hdr[off];
      if (0x23 == b) {
        off++;
        nb--;
      }
      else {
        // This is not the expected first preamble byte; it may be a button push.
        if (('A' <= b) && ('Z' >= b)) {
          if (m_enableKeypad && (NULL != m_inputThread)) {
            char input[2];
            input[0] = b;
            input[1] = '\0';
            MapInput(input);
          }
          if (--nb > 0)
            memmove(hdr, hdr+1, nb);
          continue;
        }
        // Or garbage.
        PurgeComm(m_portHandle, PURGE_RXCLEAR);
        return NULL;
      }
    }
    if ((off == 1) && (nb > 0) && (0x2A != hdr[off])) {
      // Garbage in the second byte cannot be accounted for by timing.
      PurgeComm(m_portHandle, PURGE_RXCLEAR);
      return NULL;
    }
    off += nb;
  }
  size_t len = sizeof(hdr) + (hdr[2] & ~CONT);
  LPBYTE result = (LPBYTE)malloc(len);
  memcpy(result, hdr, sizeof(hdr));
  while (off < len) {
    DWORD nb = len - off;
    if (!ReadSerial(result + off, nb, &nb, 100)) {
      free(result);
      return NULL;
    }
    off += nb;
  }
  return (DisplayReturnPacket *)result;
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new MatrixOrbitalDisplay(factory, devtype);
}

/* Called by windows */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,  LPVOID lpReserved)
{
  switch(dwReason) {
  case DLL_PROCESS_ATTACH:
    g_hInstance = (HINSTANCE)hModule;
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

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
  { "LK204PC", 20, 4, FALSE, BrightnessContinuous, TRUE, 6, TRUE },
  { "LK202U", 20, 2, FALSE, BrightnessContinuous, TRUE, 6, TRUE },
  { "LK204U", 20, 4, FALSE, BrightnessContinuous, TRUE, 6, TRUE },
  { "LK402", 40, 2, FALSE, NULL, TRUE, 7 },
  { "LK404", 40, 4, FALSE, NULL, TRUE, 1 },
  { "VFD2021", 20, 2, TRUE, BrightnessSteppedDown, FALSE, 1 },
  { "VFD2041", 20, 4, TRUE, BrightnessSteppedDown, FALSE, 1 },
  { "VK202", 20, 2, TRUE, BrightnessSteppedDown, TRUE, 6 },
  { "VK204", 20, 4, TRUE, BrightnessSteppedDown, TRUE, 6 },
  { "VK202U", 20, 2, TRUE, BrightnessSteppedDown, TRUE, 6, TRUE },
  { "VK204U", 20, 4, TRUE, BrightnessSteppedDown, TRUE, 6, TRUE },
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
  virtual void DeviceInput(BYTE b);
  virtual int DeviceGetGPOs();
  virtual void DeviceSetGPO(int gpo, BOOL on);
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
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
  if (!OpenSerial(m_enableInput))
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
  if (!Open()) return FALSE;    // Get a port handle.
  return EnableSerialInput();
}

void MatrixOrbitalDisplay::DeviceDisableInput()
{
  DisableSerialInput();
}

void MatrixOrbitalDisplay::DeviceInput(BYTE b)
{
  char buf[2];
  buf[0] = b;
  buf[1] = '\0';
  MapInput(buf);
}

int MatrixOrbitalDisplay::DeviceGetGPOs()
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

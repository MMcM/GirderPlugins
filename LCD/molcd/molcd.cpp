/* Matrix Orbital device implementation */

#include "stdafx.h"

HINSTANCE g_hInstance;

// See http://www.matrixorbital.com/products.htm for list of
// supported displays.

struct DeviceEntry {
  const char *devname;
  int cols;
  int rows;
  BOOL vfd;
  BOOL keypad;
  int gpos;
} DeviceEntries[] = {
  { "LCD2021", 20, 2, FALSE, FALSE, 1 },
  { "LCD4021", 40, 2, FALSE, FALSE, 1 },
  { "LCD2041", 20, 4, FALSE, FALSE, 1 },
  { "LCD4041", 40, 4, FALSE, FALSE, 1 },
  { "LK162", 16, 2, FALSE, TRUE, 6 },
  { "LK202", 20, 2, FALSE, TRUE, 6 },
  { "LK204", 20, 4, FALSE, TRUE, 6 },
  { "LK402", 40, 2, FALSE, TRUE, 7 },
  { "LK404", 40, 4, FALSE, TRUE, 1 },
  { "VFD2021", 20, 2, TRUE, FALSE, 1 },
  { "VFD2041", 20, 4, TRUE, FALSE, 1 },
  { "VK202", 20, 2, TRUE, TRUE, 6 },
  { "VK204", 20, 4, TRUE, TRUE, 6 },
};

#define countof(x) sizeof(x)/sizeof(x[0])

class MatrixOrbitalDisplay : public DisplayDevice
{
public:
  MatrixOrbitalDisplay(HWND parent, LPCSTR devname);
  ~MatrixOrbitalDisplay();
  
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
  BOOL m_vfd, m_keypad;
  int m_gpos;
  int m_debounceTime;
};

MatrixOrbitalDisplay::MatrixOrbitalDisplay(HWND parent, LPCSTR devname)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devname, entry->devname)) {
      m_cols = entry->cols;
      m_rows = entry->rows;
      m_vfd = entry->vfd;
      m_keypad = entry->keypad;
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

MatrixOrbitalDisplay::~MatrixOrbitalDisplay()
{
}

inline static BYTE contrast(int percent)
{
  return (percent * 0xFF) / 100;
}

inline static BYTE brightness(int percent)
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
    buf[nb++] = brightness(m_brightness);
  }
  else {
    buf[nb++] = 0xFE;
    buf[nb++] = 0x50;           // Set contrast ...
    buf[nb++] = contrast(m_contrast);
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
  return m_vfd;
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
  DisplaySendEvent(buf);
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
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  return new MatrixOrbitalDisplay(parent, name);
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

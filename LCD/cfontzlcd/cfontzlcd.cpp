/* Crystalfontz device implementation */

#include "stdafx.h"

HINSTANCE g_hInstance;

// See http://www.crystalfontz.com/products/index-ser.html for list of
// supported displays.  (Also note that the f isn't capitalized on
// that site.)

struct DeviceEntry {
  const char *devname;
  int cols;
  int rows;
  BOOL backlight;
} DeviceEntries[] = {
  { "632SS", 16, 2, FALSE },
  { "632SG", 16, 2, TRUE },
//{ "633", 16, 2, TRUE },       // Has keypad, but different instruction set.
  { "634SS", 20, 4, FALSE },
  { "634SG", 20, 4, TRUE },
};

#define countof(x) sizeof(x)/sizeof(x[0])

class CrystalfontzLCD : public DisplayDevice
{
public:
  CrystalfontzLCD(HWND parent, LPCSTR devname);
  ~CrystalfontzLCD();
  
  virtual void DeviceDisplay(int row, int col, LPCSTR str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceSetMarquee();
  virtual void DeviceClearMarquee();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasBacklight();
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
  BOOL m_backlight;
  char m_firmware[4];
};

CrystalfontzLCD::CrystalfontzLCD(HWND parent, LPCSTR devname)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devname, entry->devname)) {
      m_cols = entry->cols;
      m_rows = entry->rows;
      m_backlight = entry->backlight;
      break;
    }
  }
  m_portType = portSERIAL;
  strcpy(m_port, "COM1");
  m_portSpeed = CBR_19200;

  // The interesting transition is from 1.3 to 2.0 because the
  // character set changed.
  strcpy(m_firmware, "2.0");
}

CrystalfontzLCD::~CrystalfontzLCD()
{
}

BOOL CrystalfontzLCD::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  // Wait while boot screen is displayed.  Another possibility would
  // be to buffer a bunch of spaces to make up the time.
  Sleep(1500);

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 4;                // Hide Cursor
  if (m_backlight) {
    buf[nb++] = 14;             // Backlight Control
    buf[nb++] = (BYTE)m_brightness;
  }
  buf[nb++] = 15;               // Contrast Control
  buf[nb++] = (BYTE)m_contrast;
  buf[nb++] = 20;               // Scroll OFF
  buf[nb++] = 24;               // Wrap OFF
  buf[nb++] = 12;               // Form Feed
  WriteSerial(buf, nb);

  return TRUE;
}

void CrystalfontzLCD::DeviceClose()
{
  CloseSerial();
}

void CrystalfontzLCD::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 12;               // Form Feed
  WriteSerial(buf, nb);
}

void CrystalfontzLCD::DeviceDisplay(int row, int col, LPCSTR str, int length)
{
  BYTE buf[128];
  int nb = 0;
  if ((row == 0) && (col == 0))
    buf[nb++] = 1;               // Cursor Home
  else {
    buf[nb++] = 17;              // Set Cursor Position
    buf[nb++] = col;
    buf[nb++] = row;
  }
  for (int i = 0; i < length; i++) {
    char ch = str[i];
    // TODO: Translate character set?
    buf[nb++] = ch;
  }
  WriteSerial(buf, nb);
}

void CrystalfontzLCD::DeviceSetMarquee()
{
  // Initialize with start of string, but do not remember that.
  DeviceDisplay(m_marqueeRow, 0, m_marquee, m_cols);
  memset(m_buffer + (m_marqueeRow * m_cols), 0xFE, m_cols);

  BYTE buf[128];
  int nb = 0;
  for (int i = 0; i < 20; i++) {
    buf[nb++] = 21;             // Set Scrolling Marquee Characters
    buf[nb++] = i;
    char ch;
    if (i + m_cols < m_marqueeLen)
      ch = m_marquee[i + m_cols];
    else
      ch = ' ';
    // TODO: Translate character set?
    buf[nb++] = ch;
  }
  buf[nb++] = 22;               // Enable Scrolling Marquee
  buf[nb++] = m_marqueeRow;
  buf[nb++] = (BYTE)m_marqueePixelWidth;
  buf[nb++] = (BYTE)((m_marqueeSpeed * 96) / 1000);
  WriteSerial(buf, nb);
}

void CrystalfontzLCD::DeviceClearMarquee()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 22;               // Enable Scrolling Marquee
  buf[nb++] = 255;              // disable
  buf[nb++] = (BYTE)m_marqueePixelWidth;
  buf[nb++] = (BYTE)((m_marqueeSpeed * 96) / 1000);
  WriteSerial(buf, nb);
}

BOOL CrystalfontzLCD::DeviceHasContrast()
{
  return TRUE;
}

BOOL CrystalfontzLCD::DeviceHasBrightness()
{
  return m_backlight;
}

BOOL CrystalfontzLCD::DeviceHasBacklight()
{
  return m_backlight;
}

void CrystalfontzLCD::DeviceLoadSettings(HKEY hkey)
{
  GetSettingString(hkey, "CrystalfontzFirmware", m_firmware, sizeof(m_firmware));
}

void CrystalfontzLCD::DeviceSaveSettings(HKEY hkey)
{
  SetSettingString(hkey, "CrystalfontzFirmware", m_firmware);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  return new CrystalfontzLCD(parent, name);
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

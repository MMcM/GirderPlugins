/* Crystalfontz device implementation 
$Header$
*/

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
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceSetMarquee();
  virtual void DeviceClearMarquee();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasBacklight();

protected:
  BOOL m_backlight;
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

  // Custom characters are 128-135.
  for (i = 0; i < 8; i++) {
    m_characterMap[i] = 128 + i;
    m_characterMap[128 + i] = i;
  }
  // Version 2.0 of the CGROM has various other rearrangements,
  // including the following for ISO-8859-1L.
  if (FALSE) {
    m_characterMap['$'] = 0xA2; // Has ¤
    m_characterMap[0xA2] = '$';
    m_characterMap['@'] = 0xA0; // Has ¡
    m_characterMap[0xA0] = '@';
    m_characterMap['['] = 0xFA; // Has Ä
    m_characterMap[0xFA] = '[';
    m_characterMap['\\'] = 0xFB; // Has Ö
    m_characterMap[0xFB] = '\\';
    m_characterMap[']'] = 0xFC; // Has Ñ
    m_characterMap[0xFC] = ']';
    m_characterMap['^'] = 0x0D; // Has Ü
    m_characterMap[0x0D] = '^';
    m_characterMap['_'] = 0xC4; // Has §
    m_characterMap[0xC4] = '_';
  //m_characterMap['`'] = ???;  // Has ¿ -- backquote does not appear to be anyplace
    m_characterMap['{'] = 0xFD; // Has ä
    m_characterMap[0xFD] = '{';
    m_characterMap['|'] = 0xFE; // Has ö
    m_characterMap[0xFE] = '|';
    m_characterMap['}'] = 0xFF; // Has ñ
    m_characterMap[0xFF] = '}';
    m_characterMap['~'] = 0xCE; // Has ü
    m_characterMap[0xCE] = '~';
  }
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

void CrystalfontzLCD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
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
    BYTE b = str[i];
    if (b < 32) {
      // A control character: escape it.
      buf[nb++] = 30;           // Send Data Directly to the LCD Controller
      buf[nb++] = 1;            // Data Memory
    }
    buf[nb++] = b;
  }
  WriteSerial(buf, nb);
}

void CrystalfontzLCD::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 25;               // Set Custom Character Bitmap
  buf[nb++] = index;
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
}

void CrystalfontzLCD::DeviceSetMarquee()
{
  // Initialize with start of string, but do not remember that.
  DeviceDisplay(m_marquee->GetRow(), 0, m_marquee->GetBytes(), m_cols);
  memset(m_buffer->GetBuffer(m_marquee->GetRow(), 0), 0xFE, m_cols);

  BYTE buf[128];
  int nb = 0;
  for (int i = 0; i < 20; i++) {
    buf[nb++] = 21;             // Set Scrolling Marquee Characters
    buf[nb++] = i;
    char ch;
    if (i + m_cols < m_marquee->GetLength())
      ch = m_marquee->GetBytes()[i + m_cols];
    else
      ch = ' ';
    // TODO: Translate character set?
    buf[nb++] = ch;
  }
  buf[nb++] = 22;               // Enable Scrolling Marquee
  buf[nb++] = m_marquee->GetRow();
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

/* Crystalfontz command stream device implementation 
$Header$
*/

#include "stdafx.h"

#include "streamlcd.h"

CrystalfontzStreamLCD::CrystalfontzStreamLCD
(DisplayDeviceFactory *factory, LPCSTR devtype,
 int cols, int rows, int backlight)
  : DisplayDevice(factory, devtype)
{
  m_cols = cols;
  m_rows = rows;
  m_backlight = backlight;
  m_bootDelay = 1.5;

  m_portType = portSERIAL;
  strcpy(m_port, "COM1");
  m_portSpeed = CBR_19200;

  // Custom characters are 128-135.
  for (int i = 0; i < 8; i++) {
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

CrystalfontzStreamLCD::CrystalfontzStreamLCD(const CrystalfontzStreamLCD& other)
  : DisplayDevice(other)
{
  m_backlight = other.m_backlight;
  m_bootDelay = other.m_bootDelay;
}

CrystalfontzStreamLCD::~CrystalfontzStreamLCD()
{
}

DisplayDevice *CrystalfontzStreamLCD::Duplicate() const
{
  return new CrystalfontzStreamLCD(*this);
}

BOOL CrystalfontzStreamLCD::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  // Wait while boot screen is displayed.
  m_bootDelay.Wait();

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

void CrystalfontzStreamLCD::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 12;               // Form Feed
  if (m_backlight) {
    buf[nb++] = 14;             // Backlight Control
    buf[nb++] = 0;              // Off
  }
  WriteSerial(buf, nb);

  CloseSerial();
}

void CrystalfontzStreamLCD::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 12;               // Form Feed
  WriteSerial(buf, nb);
}

void CrystalfontzStreamLCD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
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

void CrystalfontzStreamLCD::DeviceDefineCustomCharacter(int index, 
                                                        const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 25;               // Set Custom Character Bitmap
  buf[nb++] = index;
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
}

void CrystalfontzStreamLCD::DeviceSetMarquee()
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

void CrystalfontzStreamLCD::DeviceClearMarquee()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 22;               // Enable Scrolling Marquee
  buf[nb++] = 255;              // disable
  buf[nb++] = (BYTE)m_marqueePixelWidth;
  buf[nb++] = (BYTE)((m_marqueeSpeed * 96) / 1000);
  WriteSerial(buf, nb);
}

BOOL CrystalfontzStreamLCD::DeviceHasContrast()
{
  return TRUE;
}

BOOL CrystalfontzStreamLCD::DeviceHasBrightness()
{
  return (m_backlight != NOBACKLIGHT);
}

BOOL CrystalfontzStreamLCD::DeviceHasBacklight()
{
  return (m_backlight != NOBACKLIGHT);
}

#if 0
void CrystalfontzStreamLCD::DeviceLoadSettings(HKEY hkey)
{
  m_bootDelay.LoadSetting(hkey, "BootDelay");
}

void CrystalfontzStreamLCD::DeviceSaveSettings(HKEY hkey)
{
  m_bootDelay.SaveSetting(hkey, "BootDelay");
}
#endif

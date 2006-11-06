/* Pertelian device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

#define countof(x) sizeof(x)/sizeof(x[0])

class X2040Display : public DisplayDevice
{
public:
  X2040Display(DisplayDeviceFactory *factory, LPCSTR devtype);
  X2040Display(const X2040Display& other);
  ~X2040Display();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceHasBrightness();
  virtual void DeviceWriteRaw(LPBYTE data, DWORD len);
};

X2040Display::X2040Display(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 20;
  m_rows = 4;
  m_portType = portSERIAL;
  strcpy(m_port, "COM3");
  m_portSpeed = CBR_19200;
}

X2040Display::X2040Display(const X2040Display& other)
  : DisplayDevice(other)
{
}

X2040Display::~X2040Display()
{
}


DisplayDevice *X2040Display::Duplicate() const
{
  return new X2040Display(*this);
}

BOOL X2040Display::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;             // Set instruction mode
  buf[nb++] = 0x38;             // Function set with 8-bit, 2 lines, and 5x7 dots
  buf[nb++] = 0xFE;
  buf[nb++] = 0x06;             // Entry mode set
  buf[nb++] = 0xFE;
  buf[nb++] = 0x10;             // Cursor/display shift; cursor move
  buf[nb++] = 0xFE;
  buf[nb++] = 0x0C;             // Display On; cursor off; do not blink 
  buf[nb++] = 0xFE;
  buf[nb++] = (m_brightness > 0) ? 0x03 : 0x02; // Backlight on / off.
  buf[nb++] = 0xFE;
  buf[nb++] = 0x01;             // Clear display
  WriteSerial(buf, nb);

  return TRUE;
}

void X2040Display::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x01;             // Clear display
  buf[nb++] = 0xFE;
  buf[nb++] = 0x02;             // Backlight off.
  buf[nb++] = 0xFE;
  buf[nb++] = 0x08;             // Display Off
  WriteSerial(buf, nb);

  CloseSerial();
}

void X2040Display::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x01;             // Clear display
  WriteSerial(buf, nb);
}

void X2040Display::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  int d = col + (row % 2) * 0x40;
  if (row >= 2)
    d += m_cols;                // 4x20 is folded: bottom rows take right cols.
  if (m_rows == 1)
    d += (col % 8) * (0x40 - 8);
  buf[nb++] = 0xFE;
  buf[nb++] = 0x80 + d;         // Set DDRAM address
  memcpy(buf + nb, m_buffer->GetBuffer(row, 0), m_cols);
  nb += m_cols;
  buf[nb++] = 0;
  WriteSerial(buf, nb);
}

void X2040Display::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xFE;
  buf[nb++] = 0x40 + (index * NCUSTROWS); // Set CGRAM address
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
}

BOOL X2040Display::DeviceHasBrightness()
{
  return TRUE;
}

void X2040Display::DeviceWriteRaw(LPBYTE data, DWORD len)
{
  WriteSerial(data, len);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new X2040Display(factory, devtype);
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

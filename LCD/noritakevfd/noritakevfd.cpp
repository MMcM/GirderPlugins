/* Noritake VFD device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

// TODO: I am sure that there are other models with different sizes if not protocols.

#define countof(x) sizeof(x)/sizeof(x[0])

class NoritakeVFDDisplay : public DisplayDevice
{
public:
  NoritakeVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  NoritakeVFDDisplay(const NoritakeVFDDisplay& other);
  ~NoritakeVFDDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceHasBrightness();
};

NoritakeVFDDisplay::NoritakeVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 20;
  m_rows = 4;
  m_portType = portSERIAL;
  strcpy(m_port, "COM2");
  m_portSpeed = CBR_19200;
}

NoritakeVFDDisplay::NoritakeVFDDisplay(const NoritakeVFDDisplay& other)
  : DisplayDevice(other)
{
}

NoritakeVFDDisplay::~NoritakeVFDDisplay()
{
}


DisplayDevice *NoritakeVFDDisplay::Duplicate() const
{
  return new NoritakeVFDDisplay(*this);
}

#define ESC 0x1B

BOOL NoritakeVFDDisplay::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x0E;             // Clear Display
  buf[nb++] = ESC;
  buf[nb++] = 'L';              // Luminance Control
  buf[nb++] = (m_brightness * 0xFF) / 100;
  WriteSerial(buf, nb);

  return TRUE;
}

void NoritakeVFDDisplay::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x0E;             // Clear Display
  WriteSerial(buf, nb);

  CloseSerial();
}

void NoritakeVFDDisplay::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x0E;             // Clear Display
  WriteSerial(buf, nb);
}

void NoritakeVFDDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = ESC;
  buf[nb++] = 'H';              // Position Cursor
  buf[nb++] = (row * m_cols) + col;
  for (int i = 0; i < length; i++) {
    BYTE b = str[i];
    buf[nb++] = b;
  }
  WriteSerial(buf, nb);
}

void NoritakeVFDDisplay::DeviceDefineCustomCharacter(int index, 
                                                     const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = ESC;
  buf[nb++] = 'C';              // Set User Font
  buf[nb++] = index;
  // LSB on left; densely packed.
  memset(buf+nb, 0, 5);
  for (int i = 0; i < 7; i++) {
    BYTE b = cust.GetBits()[i];
    for (int j = 0; j < 5; j++) {
      if ((b >> (4-j)) & 1) {
        int bit = i*5+j;
        buf[nb + (bit / 8)] |= (1 << (bit % 8));
      }
    }
  }
  nb += 5;
  WriteSerial(buf, nb);
}

BOOL NoritakeVFDDisplay::DeviceHasBrightness()
{
  return TRUE;
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new NoritakeVFDDisplay(factory, devtype);
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

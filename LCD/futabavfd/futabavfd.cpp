/* Futaba VFD device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

#define countof(x) sizeof(x)/sizeof(x[0])

class FutabaVFDDisplay : public DisplayDevice
{
public:
  FutabaVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  FutabaVFDDisplay(const FutabaVFDDisplay& other);
  ~FutabaVFDDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
};

FutabaVFDDisplay::FutabaVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  // TODO: There are models with sizes other than just these three.
  if (NULL == devtype) {
    m_cols = 20;                // Compatibility with older settings
    m_rows = 4;
  }
  else if (!strcmp(devtype, "M202")) {
    m_cols = 20;
    m_rows = 2;
  }
  else if (!strcmp(devtype, "M204")) {
    m_cols = 20;
    m_rows = 4;
  }
  else if (!strcmp(devtype, "M402")) {
    m_cols = 40;
    m_rows = 2;
  }
  else {
    m_cols = 20;                // Default
    m_rows = 4;
  }
  m_portType = portSERIAL;
  strcpy(m_port, "COM2");
  m_portSpeed = CBR_9600;
}

FutabaVFDDisplay::FutabaVFDDisplay(const FutabaVFDDisplay& other)
  : DisplayDevice(other)
{
}

FutabaVFDDisplay::~FutabaVFDDisplay()
{
}


DisplayDevice *FutabaVFDDisplay::Duplicate() const
{
  return new FutabaVFDDisplay(*this);
}

BOOL FutabaVFDDisplay::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x1F;				// Reset and clear display
  // TODO: Can we turn off the display?
  WriteSerial(buf, nb);

  return TRUE;
}

void FutabaVFDDisplay::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x1F;				// Reset and clear display
  WriteSerial(buf, nb);

  CloseSerial();
}

void FutabaVFDDisplay::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x1F;				// Reset and clear display
  WriteSerial(buf, nb);
}

void FutabaVFDDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x10;             // Display Position
  buf[nb++] = (row * m_cols) + col;
  for (int i = 0; i < length; i++) {
    BYTE b = str[i];
    buf[nb++] = b;
  }
  WriteSerial(buf, nb);
}

void FutabaVFDDisplay::DeviceDefineCustomCharacter(int index, 
                                                   const CustomCharacter& cust)
{
#if 0
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x03;             // Define Character
  // TODO: This is any printing byte (0x20 - 0xFF), so need to map them.
  buf[nb++] = index;
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
#endif
}

// TODO: Some models can set the brightness with 0x04:
// 100% - 0xFF, 60% - 0x60, 40% - 0x40, 20% - 0x20.

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new FutabaVFDDisplay(factory, devtype);
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

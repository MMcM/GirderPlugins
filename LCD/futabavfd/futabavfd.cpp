/* Futaba VFD device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

#define countof(x) sizeof(x)/sizeof(x[0])

class FutabaVFDDisplay : public DisplayDevice
{
public:
  FutabaVFDDisplay(HWND parent, LPCSTR devname);
  ~FutabaVFDDisplay();
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
};

FutabaVFDDisplay::FutabaVFDDisplay(HWND parent, LPCSTR devname)
{
  m_cols = 20;
  m_rows = 4;
  m_portType = portSERIAL;
  strcpy(m_port, "COM2");
  m_portSpeed = CBR_9600;
}

FutabaVFDDisplay::~FutabaVFDDisplay()
{
}


BOOL FutabaVFDDisplay::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x1F;				// Reset and clear display
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
  buf[nb++] = 0x10;
  buf[nb++] = (row*20)+col;
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
  // TODO: Need control code for define custom char (aka set CGRAM address).
  buf[nb++] = ??;
  buf[nb++] = index;
  for (int i = 0; i < NCUSTROWS; i++)
    buf[nb++] = cust.GetBits()[i];
  WriteSerial(buf, nb);
#endif
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  return new FutabaVFDDisplay(parent, name);
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

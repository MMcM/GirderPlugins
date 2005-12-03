/* Noritake VFD serial device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

#include "noritakevfd.cpp"

class NoritakeVFDSerialDisplay : public NoritakeVFDDisplay
{
public:
  NoritakeVFDSerialDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  NoritakeVFDSerialDisplay(const NoritakeVFDSerialDisplay& other);
  ~NoritakeVFDSerialDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();

  virtual BOOL Write(LPBYTE data, DWORD len);
};

NoritakeVFDSerialDisplay::NoritakeVFDSerialDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : NoritakeVFDDisplay(factory, devtype)
{
  m_portType = portSERIAL;
  strcpy(m_port, "COM2");
  m_portSpeed = CBR_19200;
}

NoritakeVFDSerialDisplay::NoritakeVFDSerialDisplay(const NoritakeVFDSerialDisplay& other)
  : NoritakeVFDDisplay(other)
{
}

NoritakeVFDSerialDisplay::~NoritakeVFDSerialDisplay()
{
}


DisplayDevice *NoritakeVFDSerialDisplay::Duplicate() const
{
  return new NoritakeVFDSerialDisplay(*this);
}

BOOL NoritakeVFDSerialDisplay::DeviceOpen()
{
  if (!OpenSerial(FALSE, FALSE)) // Parity according to Control Panel.
    return FALSE;

  return NoritakeVFDDisplay::DeviceOpen();
}

void NoritakeVFDSerialDisplay::DeviceClose()
{
  NoritakeVFDDisplay::DeviceClose();

  CloseSerial();
}

BOOL NoritakeVFDSerialDisplay::Write(LPBYTE data, DWORD len)
{
  return WriteSerial(data, len);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new NoritakeVFDSerialDisplay(factory, devtype);
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

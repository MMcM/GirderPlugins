/* SoundGraph iMON VFD device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

class iMONDisplay : public DisplayDevice
{
public:
  iMONDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  iMONDisplay(const iMONDisplay& other);
  ~iMONDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual int DeviceNCustomCharacters();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
};

iMONDisplay::iMONDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 16;
  m_rows = 2;
}

iMONDisplay::iMONDisplay(const iMONDisplay& other)
  : DisplayDevice(other)
{
}

iMONDisplay::~iMONDisplay()
{
}


DisplayDevice *iMONDisplay::Duplicate() const
{
  return new iMONDisplay(*this);
}

BOOL iMONDisplay::DeviceOpen()
{
  return iMONVFD_Init(VFDHW_IMON_VFD);
}

void iMONDisplay::DeviceClose()
{
  iMONVFD_Uninit();
}

void iMONDisplay::DeviceClear()
{
}

void iMONDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  char line1[17], line2[17];
  memcpy(line1, m_buffer->GetBuffer(0, 0), m_cols);
  line1[m_cols] = '\0';
  memcpy(line2, m_buffer->GetBuffer(1, 0), m_cols);
  line2[m_cols] = '\0';
  iMONVFD_SetText(line1, line2);
}

int iMONDisplay::DeviceNCustomCharacters()
{
  return 0;
}

void iMONDisplay::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new iMONDisplay(factory, devtype);
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

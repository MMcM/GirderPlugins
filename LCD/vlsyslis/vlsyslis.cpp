/* VLSystem L.I.S. (Lost in Space) device implementation 
$Header$
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

#define countof(x) sizeof(x)/sizeof(x[0])

// TODO: Device has AFC (4x automatic fan control) but I don't know
// the protocol.  (Looks like maybe a variant of the A1 sequence with
// something other than 0 as the next byte.)

class LIS2Display : public DisplayDevice
{
public:
  LIS2Display(DisplayDeviceFactory *factory, LPCSTR devtype);
  LIS2Display(const LIS2Display& other);
  ~LIS2Display();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
};

LIS2Display::LIS2Display(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 20;
  m_rows = 2;
  m_portType = portSERIAL;
  strcpy(m_port, "COM2");
  m_portSpeed = CBR_19200;
}

LIS2Display::LIS2Display(const LIS2Display& other)
  : DisplayDevice(other)
{
}

LIS2Display::~LIS2Display()
{
}


DisplayDevice *LIS2Display::Duplicate() const
{
  return new LIS2Display(*this);
}

BOOL LIS2Display::DeviceOpen()
{
  if (!OpenSerial())
    return FALSE;

  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xA0;             // Clear
  WriteSerial(buf, nb);

  return TRUE;
}

void LIS2Display::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xA0;             // Clear
  WriteSerial(buf, nb);

  CloseSerial();
}

void LIS2Display::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0xA0;             // Clear
  WriteSerial(buf, nb);
}

void LIS2Display::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = (row == 1) ? 0xA2 : 0xA1; // Display row
  buf[nb++] = 0;
  buf[nb++] = 0xA7;
  memcpy(buf + nb, m_buffer->GetBuffer(row, 0), m_cols);
  nb += m_cols;
  buf[nb++] = 0;
  WriteSerial(buf, nb);
}

void LIS2Display::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  for (int i = 0; i < NCUSTROWS; i++) {
    buf[nb++] = 0xAB;           // Define CG
    buf[nb++] = index;
    buf[nb++] = i;
    buf[nb++] = cust.GetBits()[i];
  }
  WriteSerial(buf, nb);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new LIS2Display(factory, devtype);
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

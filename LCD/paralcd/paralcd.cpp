/* Parallel port HD44780 device implementation 
$Header$
*/

// See http://www.mswanson.com/~nstuart/lcd/ or
// http://www.geocities.com/ResearchTriangle/1495/ee_lcd.html for a
// description of the hardware wiring between the parallel port and
// the controller.

// See http://www.driverlinx.com/Software_Products/softwareproducts.htm
// for the PortIO driver used.

#include "stdafx.h"

HINSTANCE g_hInstance;

class ParallelLCD : public DisplayDevice
{
public:
  ParallelLCD(DisplayDeviceFactory *factory, LPCSTR devtype);
  ParallelLCD(const ParallelLCD& other);
  ~ParallelLCD();

  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasSetSize();
  virtual void DeviceWriteRaw(LPBYTE data, DWORD len);
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
  void WriteIR(BYTE b, int dev);
  void WriteDR(BYTE b, int dev);

  int m_portAddr;
  Delay m_strobeDelay, m_commandDelay, m_extraDelay;
  int m_ndevs;
};

ParallelLCD::ParallelLCD(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 20;
  m_rows = 4;
  m_portType = portPARALLEL;
  strcpy(m_port, "LPT1");
  m_strobeDelay = .002;
  m_commandDelay = .005;
  m_extraDelay = 0;
  m_ndevs = 1;
}

ParallelLCD::ParallelLCD(const ParallelLCD& other)
  : DisplayDevice(other)
{
}

ParallelLCD::~ParallelLCD()
{
}

DisplayDevice *ParallelLCD::Duplicate() const
{
  return new ParallelLCD(*this);
}

BOOL ParallelLCD::DeviceOpen()
{
  if (!strcmp(m_port, "LPT1"))
    m_portAddr = 0x378;
  else if (!strcmp(m_port, "LPT2"))
    m_portAddr = 0x278;
  else if (!strcmp(m_port, "LPT3"))
    m_portAddr = 0x3BC;
  else
    m_portAddr = strtol(m_port, NULL, 16);

  m_ndevs = ((m_rows * m_cols) + 79) / 80; // 80 chars / device, round up.

  for (int dev = 0; dev < m_ndevs; dev++) {
    WriteIR(0x38, dev); // Function set: DL=1 (8-bit), N=1 (2 lines), F=0 (5x8)
    WriteIR(0x38, dev); // again
    WriteIR(0x06, dev); // Entry mode set: I/D=1 (incr), S=0 (no shift)
    WriteIR(0x0C, dev); // Display control: D=1 (on), C=0 (cursor off), B=0 (blink off)
    m_extraDelay.Wait();
    WriteIR(0x01, dev); // Clear display
    m_extraDelay.Wait();
  }

  return TRUE;
}

void ParallelLCD::DeviceClose()
{
  for (int dev = 0; dev < m_ndevs; dev++) {
    WriteIR(0x01, dev); // Clear display
    m_extraDelay.Wait();
    WriteIR(0x08, dev); // Display control: D=0 (off), C=0 (cursor off), B=0 (blink off)
  }
}

void ParallelLCD::DeviceClear()
{
  for (int dev = 0; dev < m_ndevs; dev++) {
    WriteIR(0x01, dev); // Clear display
    m_extraDelay.Wait();
  }
}

void ParallelLCD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  int dev = (row * m_cols) / 80;
  if (dev > 0)
    row -= (dev * 80) / m_cols;
  if ((row == 0) && (col == 0)) {
    WriteIR(0x02, dev);         // Return home
  }
  else {
    int d = col + (row % 2) * 0x40;
    if (row >= 2)
      d += m_cols;              // 4x20 is folded: bottom rows take right cols.
    if (m_rows == 1)
      d += (col % 8) * (0x40 - 8);
    WriteIR(0x80 + d, dev);     // Set DDRAM address
  }
  for (int i = 0; i < length; i++)
    WriteDR(str[i], dev);
}

void ParallelLCD::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
  for (int dev = 0; dev < m_ndevs; dev++) {
    WriteIR(0x40 + (index * NCUSTROWS), dev); // Set CGRAM address
    for (int i = 0; i < NCUSTROWS; i++)
      WriteDR(cust.GetBits()[i], dev);
  }
}

BOOL ParallelLCD::DeviceHasSetSize()
{
  return TRUE;
}

void ParallelLCD::DeviceWriteRaw(LPBYTE data, DWORD len)
{
  // Can only cover one register; take data.
  for (DWORD i = 0; i < len; i++)
    WriteDR(data[i], 0);
}

void ParallelLCD::DeviceLoadSettings(HKEY hkey)
{
  m_strobeDelay.LoadSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.LoadSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.LoadSetting(hkey, "ParallelExtraDelay");
}

void ParallelLCD::DeviceSaveSettings(HKEY hkey)
{
  m_strobeDelay.SaveSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.SaveSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.SaveSetting(hkey, "ParallelExtraDelay");
}

// Printer port definitions
#define PDR (m_portAddr)        // Pins 2-9 wired to DB0-DB7
#define PSR (m_portAddr + 1)
#define PCR (m_portAddr + 2)

#define STROBE 0x01             // Pin  1 (wired to E)   (inverted: 0 = H)
#define AF     0x02             // Pin 14 (wired to R/W) (inverted: 1 = L = MPU -> LCD)
#define INIT   0x04             // Pin 16 (wired to RS)  (0 = L = IR; 1 = H = DR)
#define SELE   0x08             // Pin 17 (wired to E2)  (inverted: 0 = H)

// Sort of like _out in DOS.
inline static void outport(int a, BYTE b)
{
  DlPortWritePortUchar(a, b);
}

inline static void delay(const Delay& delay)
{
  delay.Wait();
}

inline static BYTE eBit(int dev)
{
  switch (dev) {
  case 0:
  default:
    return STROBE;
  case 1:
    return SELE;
  }
}

void ParallelLCD::WriteIR(BYTE b, int dev)
{
  outport(PDR, b);
  outport(PCR, AF);
  delay(m_strobeDelay);
  outport(PCR, AF | eBit(dev));
  delay(m_commandDelay);
}

void ParallelLCD::WriteDR(BYTE b, int dev)
{
  outport(PDR, b);
  outport(PCR, AF | INIT);
  delay(m_strobeDelay);
  outport(PCR, AF | INIT | eBit(dev));
  delay(m_commandDelay);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new ParallelLCD(factory, devtype);
}

/* Called by windows */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
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

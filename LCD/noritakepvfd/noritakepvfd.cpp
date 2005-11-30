/* Parallel port HD44780 device implementation for the noritake CU40026SCPB-T20A
$Header$
*/

// See http://www.vfdworld.com/index.php?page=usingyourvfd 
// under "Noritake CU20045SCPB-T23A 4x20 VFD" for a
// description of the hardware wiring between the parallel port and
// the this vfd's controller.
// - or check the following -
// VFD Pinout:
// Function        Parallel Port Pin     VFD Pin
// Data 7          9                     1
// Data 6          8                     2
// Data 5          7                     3
// Data 4          6                     4
// Data 3          5                     5
// Data 2          4                     6
// Data 1          3                     7
// Data 0          2                     8
// WR              1                     9
// CS              16                    10
// SerIn (SIN)     None                  11
// BUSY            11                    12
// Gnd             Molex Black           13
// Gnd             Molex Black           14
// +5v             Molex Red             15
// +5v             Molex Red             16

// See http://www.driverlinx.com/Software_Products/softwareproducts.htm
// for the PortIO driver used.

/*
** Most code taken from the paralcd and noritakevfd projects, in this same solution,
** and reworked for the Noritake (ISE Electronics) Model CU40026SCPB-T20A using it's
** parallel connector, wiring diagram is as above.
**                                                    by Matthew.J.Edwards 9-July-2005
*/
/*
** Project Notes:
**  ->  Only 1 device (VFD) is supported, this is due to the CU40026SCPB-T20A not having an
**      enable pin, with out one of those on each display, it's a little hard to talk to them
**      individually.
**
**  ->  VFD timings (command, extra, strobe) are very lean, i.e. they are the lowest they can
**      go, well, at least the lowest I could get them before the display started crapping out,
**      but these are based on a P4 3Ghz (which really doesn't matter) and a 75cm cable.
*/

#include "stdafx.h"

HINSTANCE g_hInstance;

class NoritakeParallelVFD : public DisplayDevice
{
public:
  NoritakeParallelVFD(DisplayDeviceFactory *factory, LPCSTR devtype);
  NoritakeParallelVFD(const NoritakeParallelVFD& other);
  ~NoritakeParallelVFD();

  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceHasBrightness();

  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasSetSize();
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
  void WriteIR(BYTE b, int dev);
  void WriteDR(BYTE b, int dev);

  int m_portAddr;
  Delay m_strobeDelay, m_commandDelay, m_extraDelay;
  int m_ndevs;
};

NoritakeParallelVFD::NoritakeParallelVFD(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_cols = 40;
  m_rows = 2;
  m_portType = portPARALLEL;
  strcpy(m_port, "LPT1");

  m_commandDelay = .0005;
  m_extraDelay = .001;
  m_strobeDelay = .00005;
  m_ndevs = 1;
}

NoritakeParallelVFD::NoritakeParallelVFD(const NoritakeParallelVFD& other)
  : DisplayDevice(other)
{
}

NoritakeParallelVFD::~NoritakeParallelVFD()
{
}

DisplayDevice *NoritakeParallelVFD::Duplicate() const
{
  return new NoritakeParallelVFD(*this);
}

#define ESC 0x1B

BOOL NoritakeParallelVFD::DeviceOpen()
{
  if (!strcmp(m_port, "LPT1"))
    m_portAddr = 0x378;
  else if (!strcmp(m_port, "LPT2"))
    m_portAddr = 0x278;
  else if (!strcmp(m_port, "LPT3"))
    m_portAddr = 0x3BC;
  else
    m_portAddr = strtol(m_port, NULL, 16);

  m_extraDelay.Wait();

  WriteIR(ESC, 0);              // esc
  WriteIR('I', 0);              // Initialize display 'I' 0x49 (clear and cursor on at top left)

  m_extraDelay.Wait();
  m_extraDelay.Wait();

  WriteIR(0x16, 0);             // Cursor Off

  WriteIR(0x0E, 0);             // Clear Display

  WriteIR(0x11, 0);             // Normal Mode
  
  WriteIR(ESC, 0);              // esc
  WriteIR('L', 0);              // Luminance 'L' 0x4C
  WriteIR((m_brightness * 0xFF) / 100, 0); // 25%

  return TRUE;
}

void NoritakeParallelVFD::DeviceClose()
{
  WriteIR(0x0E, 0);             // Clear Display
  WriteIR(0x16, 0);             // Cursor Off

  WriteIR(ESC, 0);              // Esc
  WriteIR('H', 0);              // 'H' Cursor move 0x48
  WriteIR(0x00, 0);             // Return home
}

void NoritakeParallelVFD::DeviceClear()
{

  WriteIR(0x0e, 0);             // All written Characters are cleared. The Cursor doesn't move.

  WriteIR(ESC, 0);              // Esc
  WriteIR('H', 0);              // 'H' Cursor move 0x48
  WriteIR(0x00, 0);             // Return home cursor at (0,0)

  m_extraDelay.Wait();

}

void NoritakeParallelVFD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  WriteIR(ESC, 0);              // Esc
  WriteIR('H', 0);              // 'H' Cursor move 0x48
  WriteIR((row * m_cols) + col, 0); // Goto here!
  for (int i = 0; i < length; i++)
    WriteIR(str[i], 0);         // Write Character to vfd
}

void NoritakeParallelVFD::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
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
  for (i = 0; i < nb; i++)
    WriteIR(buf[i], 0);         // Write Character to vfd
}

BOOL NoritakeParallelVFD::DeviceHasBrightness()
{
  return TRUE;
}
BOOL NoritakeParallelVFD::DeviceHasSetSize()
{
  return TRUE;
}

void NoritakeParallelVFD::DeviceLoadSettings(HKEY hkey)
{
  m_strobeDelay.LoadSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.LoadSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.LoadSetting(hkey, "ParallelExtraDelay");
}

void NoritakeParallelVFD::DeviceSaveSettings(HKEY hkey)
{
  m_strobeDelay.SaveSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.SaveSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.SaveSetting(hkey, "ParallelExtraDelay");
}

// Printer port definitions
#define PDR (m_portAddr)        // Pins 2-9 wired to DB0-DB7
#define PSR (m_portAddr + 1)
#define PCR (m_portAddr + 2)

#define STROBE 0x01             // Pin  1 (wired to E)   (inverted: 0 = H)  (starts data Read / Write)
#define AF     0x02             // Pin 14 (wired to R/W) (inverted: 1 = L = MPU -> LCD) (selects read or write)
#define INIT   0x04             // Pin 16 (wired to RS)  (0 = L = IR; 1 = H = DR) (IR = instruction Register, DR = Data Register)
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

void NoritakeParallelVFD::WriteIR(BYTE b, int dev)
{
  outport(PCR, 0);              // strobe up to 0
//delay(m_strobeDelay);
  outport(PDR, b);              // place data onto port byte 0
  delay(m_commandDelay);
  outport(PCR, STROBE);         // strobe up to 1
  delay(m_strobeDelay);
  outport(PCR, 0);              // strobe up to 0
//delay(m_strobeDelay);
}

void NoritakeParallelVFD::WriteDR(BYTE b, int dev)
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
  return new NoritakeParallelVFD(factory, devtype);
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

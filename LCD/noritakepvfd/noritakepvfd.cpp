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

#include "../noritakevfd/noritakevfd.cpp"

class NoritakeVFDParallelDisplay : public NoritakeVFDDisplay
{
public:
  NoritakeVFDParallelDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  NoritakeVFDParallelDisplay(const NoritakeVFDParallelDisplay& other);
  ~NoritakeVFDParallelDisplay();

  DisplayDevice *Duplicate() const;

  virtual BOOL DeviceOpen();
  virtual void DeviceClose();

  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

  virtual BOOL Write(LPBYTE data, DWORD len);

protected:
  void WriteIR(BYTE b, int dev);

  int m_portAddr;
  Delay m_strobeDelay, m_commandDelay;
};

NoritakeVFDParallelDisplay::NoritakeVFDParallelDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : NoritakeVFDDisplay(factory, devtype)
{
  m_cols = 40;
  m_rows = 2;
  m_portType = portPARALLEL;
  strcpy(m_port, "LPT1");

  m_commandDelay = .0005;
  m_extraDelay = .001;
  m_strobeDelay = .00005;
}

NoritakeVFDParallelDisplay::NoritakeVFDParallelDisplay(const NoritakeVFDParallelDisplay& other)
  : NoritakeVFDDisplay(other)
{
}

NoritakeVFDParallelDisplay::~NoritakeVFDParallelDisplay()
{
}

DisplayDevice *NoritakeVFDParallelDisplay::Duplicate() const
{
  return new NoritakeVFDParallelDisplay(*this);
}

#define ESC 0x1B

BOOL NoritakeVFDParallelDisplay::DeviceOpen()
{
  if (!strcmp(m_port, "LPT1"))
    m_portAddr = 0x378;
  else if (!strcmp(m_port, "LPT2"))
    m_portAddr = 0x278;
  else if (!strcmp(m_port, "LPT3"))
    m_portAddr = 0x3BC;
  else
    m_portAddr = strtol(m_port, NULL, 16);

  return NoritakeVFDDisplay::DeviceOpen();
}

void NoritakeVFDParallelDisplay::DeviceClose()
{
  NoritakeVFDDisplay::DeviceClose();
}

void NoritakeVFDParallelDisplay::DeviceLoadSettings(HKEY hkey)
{
  m_strobeDelay.LoadSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.LoadSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.LoadSetting(hkey, "ParallelExtraDelay");
}

void NoritakeVFDParallelDisplay::DeviceSaveSettings(HKEY hkey)
{
  m_strobeDelay.SaveSetting(hkey, "ParallelStrobeDelay");
  m_commandDelay.SaveSetting(hkey, "ParallelCommandDelay");
  m_extraDelay.SaveSetting(hkey, "ParallelExtraDelay");
}

BOOL NoritakeVFDParallelDisplay::Write(LPBYTE data, DWORD len) 
{
  for (DWORD i = 0; i < len; i++)
    WriteIR(data[i], 0);
  return TRUE;
}

// Printer port definitions
#define PDR (m_portAddr)        // Pins 2-9 wired to DB0-DB7
#define PSR (m_portAddr + 1)
#define PCR (m_portAddr + 2)

#define STROBE 0x01             // Pin 1 (WR)

// Sort of like _out in DOS.
inline static void outport(int a, BYTE b)
{
  DlPortWritePortUchar(a, b);
}

inline static void delay(const Delay& delay)
{
  delay.Wait();
}

void NoritakeVFDParallelDisplay::WriteIR(BYTE b, int dev)
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

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new NoritakeVFDParallelDisplay(factory, devtype);
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

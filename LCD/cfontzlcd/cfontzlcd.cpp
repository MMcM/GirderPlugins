/* Crystalfontz device implementation 
$Header$
*/

#include "stdafx.h"

#include "streamlcd.h"
#include "packetlcd.h"

HINSTANCE g_hInstance;

// See http://www.crystalfontz.com/products/index-ser.html and
// index-usb.html for lists of supported displays.  (Also note that
// the f isn't capitalized on that site.)

struct DeviceEntry {
  const char *devtype;
  BOOL packet;
  int cols;
  int rows;
  int devmode;
} DeviceEntries[] = {
  { "632SS", FALSE, 16, 2, CrystalfontzStreamLCD::NOBACKLIGHT },
  { "632SG", FALSE, 16, 2, CrystalfontzStreamLCD::BACKLIGHT },
  { "634SS", FALSE, 20, 4, CrystalfontzStreamLCD::NOBACKLIGHT },
  { "634SG", FALSE, 20, 4, CrystalfontzStreamLCD::BACKLIGHT },
  { "633", TRUE, 16, 2, CrystalfontzPacketLCD::OLD_CMDS },
  { "631", TRUE, 20, 2, CrystalfontzPacketLCD::NEW_CMDS },
  { "631@L", TRUE, 17, 2, CrystalfontzPacketLCD::LEGENDS },
};

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devtype, entry->devtype)) {
      if (entry->packet)
        return new CrystalfontzPacketLCD(factory, devtype,
                                         entry->cols, entry->rows,
                                         entry->devmode);
      else
        return new CrystalfontzStreamLCD(factory, devtype,
                                         entry->cols, entry->rows,
                                         entry->devmode);
    }
  }
  return NULL;
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

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
  const char *devname;
  BOOL packet;
  int cols;
  int rows;
  BOOL backlight;
} DeviceEntries[] = {
  { "632SS", FALSE, 16, 2, FALSE },
  { "632SG", FALSE, 16, 2, TRUE },
  { "634SS", FALSE, 20, 4, FALSE },
  { "634SG", FALSE, 20, 4, TRUE },
  { "633", TRUE },
  { "631", TRUE },
};

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(name, entry->devname)) {
      if (entry->packet)
        return new CrystalfontzPacketLCD(name);
      else
        return new CrystalfontzStreamLCD(name, entry->cols, entry->rows, 
                                         entry->backlight);
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

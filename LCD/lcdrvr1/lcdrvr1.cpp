/* LCDriver 1.x device implementation */

#include "stdafx.h"

HINSTANCE g_hInstance;

class LCDriver1Display : public DisplayDevice
{
public:
  LCDriver1Display();
  ~LCDriver1Display();
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceSetMarquee();
  virtual void DeviceClearMarquee();
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

protected:
  BOOL m_marqueeSimulated;
};

LCDriver1Display::LCDriver1Display()
{
  m_cols = lcdGetWidth();
  m_rows = lcdGetHeight();

  m_marqueeSimulated = FALSE;
}

LCDriver1Display::~LCDriver1Display()
{
}

BOOL LCDriver1Display::DeviceOpen()
{
  if (LCD_RESULT_NOERROR != lcdRequest())
    return FALSE;

  lcdSetCursor(0);              // Hide cursor
  lcdSetWrapMode(0, 0);         // Turn off wrap mode
  lcdClearDisplay();            // Clear
  return TRUE;
}

void LCDriver1Display::DeviceClose()
{
  lcdFree();
}

void LCDriver1Display::DeviceClear()
{
  lcdClearDisplay();
}

void LCDriver1Display::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  lcdSetCursorPos(col, row);
  lcdSendString((const char *)str, length);
}

void LCDriver1Display::DeviceDefineCustomCharacter(int index, 
                                                   const CustomCharacter& cust)
{
  lcdSetCustomChar(index, (const char *)cust.GetBits());
}

void LCDriver1Display::DeviceSetMarquee()
{
  if (m_marqueeSimulated)
    SetSimulatedMarquee();
  else {
    // Don't know what's on the screen once it starts.
    memset(m_buffer + (m_marqueeRow * m_cols), 0xFE, m_cols);
    lcdEnableMarquee((const char *)m_marquee, m_marqueeLen, m_marqueeRow,
                     // Time units for CrystalFontz (the only
                     // supported hardware for this) are 1/96 sec.
                     m_marqueePixelWidth, (m_marqueeSpeed * 96) / 1000);
  }
}

void LCDriver1Display::DeviceClearMarquee()
{
  if (m_marqueeSimulated)
    ClearSimulatedMarquee();
  else
    lcdDisableMarquee();
}

void LCDriver1Display::DeviceLoadSettings(HKEY hkey)
{
  GetSettingBool(hkey, "MarqueeSimulated", m_marqueeSimulated);
}

void LCDriver1Display::DeviceSaveSettings(HKEY hkey)
{
  SetSettingBool(hkey, "MarqueeSimulated", m_marqueeSimulated);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  return new LCDriver1Display();
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

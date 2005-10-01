/* Device control functions. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

#include <initguid.h>

BOOL g_bHID = FALSE;
WORD g_wVersion = 0;

// Get a handle to the given instance of the given device class.
HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance)
{
  // Get handle to relevant device information set
  HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, 
                                      DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
  if (INVALID_HANDLE_VALUE == info) {
    return NULL;
  }

  // Get interface data for the requested instance
  SP_INTERFACE_DEVICE_DATA ifdata;
  ifdata.cbSize = sizeof(ifdata);
  if (!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instance, &ifdata)) {
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }

  // Get size of symbolic link name
  DWORD dwReqLen;
  SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &dwReqLen, NULL);
  PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)
    (new char[dwReqLen]);
  if (ifDetail == NULL) {
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }

  // Get symbolic link name
  ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
  if (!SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, dwReqLen, NULL, NULL)) {
    SetupDiDestroyDeviceInfoList(info);
    delete ifDetail;
    return NULL;
  }

  // Open file
  HANDLE rv = CreateFile(ifDetail->DevicePath, 
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, 
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
  if (rv == INVALID_HANDLE_VALUE) {
    rv = NULL;
  }

  delete ifDetail;
  SetupDiDestroyDeviceInfoList(info);
  return rv;
}

/*** PowerMate API for version 2.0 ***/

// define the PowerMate GUID
// {FC3DA4B7-1E9D-47f4-A7E3-151B97C163A6}
DEFINE_GUID(POWERMATE_GUID, 0xfc3da4b7, 0x1e9d, 0x47f4, 0xa7, 0xe3, 0x15, 0x1b, 0x97, 0xc1, 0x63, 0xa6);

#define IOCTL_POWERMATE_INFO CTL_CODE(                  \
        FILE_DEVICE_UNKNOWN,                            \
        0x802,                                          \
        METHOD_BUFFERED,                                \
        FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_SET_LED_BRIGHTNESS CTL_CODE(    \
        FILE_DEVICE_UNKNOWN,                            \
        0x807,                                          \
        METHOD_BUFFERED,                                \
        FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_DURING_SLEEP CTL_CODE(    \
        FILE_DEVICE_UNKNOWN,                            \
        0x808,                                          \
        METHOD_BUFFERED,                                \
        FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_ALWAYS CTL_CODE(          \
        FILE_DEVICE_UNKNOWN,                            \
        0x809,                                          \
        METHOD_BUFFERED,                                \
        FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_SPEED CTL_CODE(           \
        FILE_DEVICE_UNKNOWN,                            \
        0x80A,                                          \
        METHOD_BUFFERED,                                \
        FILE_ANY_ACCESS)

HANDLE OpenViaDriver()
{
  HANDLE hPowerMate = GetDeviceViaInterface((LPGUID)&POWERMATE_GUID, 0);
  if (NULL == hPowerMate)
    return NULL;
  
  DWORD dwBytesReturned;
  WORD wInfo[9];
  if (!DeviceIoControl(hPowerMate, IOCTL_POWERMATE_INFO,
                       NULL, 0, wInfo, sizeof(wInfo),
                       &dwBytesReturned, NULL)) {
    CloseHandle(hPowerMate);
    return NULL;
  }
  g_wVersion = wInfo[6];

  return hPowerMate;
}

/*** PowerMate HID API ***/

const WORD VENDOR_ID = 0x077D;
const WORD PRODUCT_ID = 0x0410;
const WORD VERSION_NUMBER = 0x0311;

HANDLE OpenViaHID()
{
  GUID hidGuid;
  HidD_GetHidGuid(&hidGuid);

  HANDLE hHidDev;
  HIDD_ATTRIBUTES hidAttrs;

  DWORD dwInstance = 0;
  while (TRUE) {
    hHidDev = GetDeviceViaInterface(&hidGuid, dwInstance++);
    if (NULL == hHidDev)
      return NULL;
    if (!HidD_GetAttributes(hHidDev, &hidAttrs)) {
      CloseHandle(hHidDev);
      continue;
    }

    if ((VENDOR_ID == hidAttrs.VendorID) &&
        (PRODUCT_ID == hidAttrs.ProductID)) {
      g_wVersion = hidAttrs.VersionNumber;
      return hHidDev;
    }

    CloseHandle(hHidDev);
  }
}

HANDLE OpenPowerMate()
{
  HANDLE hPowerMate = OpenViaDriver();
  if (NULL != hPowerMate)
    g_bHID = FALSE;
  else {
    hPowerMate = OpenViaHID();
    if (NULL != hPowerMate)
      g_bHID = TRUE;
  }
  
  if (NULL == hPowerMate)
    GirderLogMessageEx(PLUGINNAME, "Could not find PowerMate device.", GLM_ERROR_ICON);
  else {
    char buf[128];
    _snprintf(buf, sizeof(buf), "Found PowerMate version %X.", g_wVersion);
    GirderLogMessageEx(PLUGINNAME, buf, GLM_GOOD_ICON);
  }

  return hPowerMate;
}

/*** Control functions ***/

BOOL g_bRunning = FALSE;
HANDLE g_hEvent = NULL;

DWORD WINAPI DeviceThread(LPVOID lpParam)
{
  HANDLE hPowerMate = (HANDLE)lpParam;

  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  DWORD rc = 0;
  while (g_bRunning) {
    signed char data[8];
    DWORD dwBytesRequired, dwBytesRead;
    dwBytesRequired = sizeof(data);
    if (!g_bHID)
      dwBytesRequired = (g_wVersion < 0x200) ? 3 : 6;
    if (!ReadFile(hPowerMate, data, dwBytesRequired, &dwBytesRead, &overlapped)) {
      rc = GetLastError();
      if (ERROR_IO_PENDING != rc) break;
      WaitForSingleObject(overlapped.hEvent, INFINITE);
      if (!GetOverlappedResult(hPowerMate, &overlapped, &dwBytesRead, FALSE)) {
        rc = GetLastError();
        if (ERROR_IO_INCOMPLETE == rc) {
          // The main thread wants us to stop sending events.
          CancelIo(hPowerMate);
          if (!g_bRunning) {
            rc = 0;
          }
        }
        break;
      }
    }
    signed char *pdata = data;
    if (g_bHID) {
      // HID read returns the 6 request bytes plus a header.
      pdata++;
      dwBytesRequired--;
    }
    if (dwBytesRequired != dwBytesRead) continue;

    PCHAR event = NULL;
    PCHAR payload = NULL;
    size_t pllen = 0;
    char buf[16];

    int n = pdata[1];
    if (pdata[0] & 1) {
      if (n > 0)
        event = "PowerMate.RightDown";
      else if (n < 0) {
        event = "PowerMate.LeftDown";
        n = - n;
      }
      else
        event = "PowerMate.Down";
    }
    else {
      if (n > 0)
        event = "PowerMate.Right";
      else if (n < 0) {
        event = "PowerMate.Left";
        n = - n;
      }
      else
        event = "PowerMate.Up";
    }
    if (n != 0) {
      buf[0] = 1;
      sprintf(buf + 1, "%d", n);
      payload = buf;
      pllen = strlen(payload) + 1; // Including both count and null bytes.
    }
    SendEventEx(event, payload, pllen, PLUGINNUM, EVENT_MOD_NONE);
  }

  g_hEvent = NULL;
  CloseHandle(overlapped.hEvent);

  return rc;
}

HANDLE g_hPowerMate = NULL;
HANDLE g_hThread = NULL;

BOOL DeviceOpen()
{
  if (NULL == g_hPowerMate)
    g_hPowerMate = OpenPowerMate();
  return (NULL != g_hPowerMate);
}

void DeviceClose()
{
  DeviceStop();
  if (NULL != g_hPowerMate) {
    CloseHandle(g_hPowerMate);
    g_hPowerMate = NULL;
  }
}

BOOL DeviceStart()
{
  if (!DeviceOpen()) 
    return FALSE;
  g_bRunning = TRUE;
  DWORD dwThreadId;
  g_hThread = CreateThread(NULL, 0, DeviceThread, (LPVOID)g_hPowerMate, 0, &dwThreadId);
  return (NULL != g_hThread);
}

BOOL DeviceStop()
{
  if (!g_bRunning)
    return TRUE;
  g_bRunning = FALSE;
  if (NULL != g_hThread) {
    SetEvent(g_hEvent);         // Awaken from asynch read.
    WaitForSingleObject(g_hThread, 5000);
    CloseHandle(g_hThread);
    g_hThread = NULL;
  }
  return TRUE;
}

BOOL DeviceRequest(RequestType_t nReq, USHORT nVal)
{
  if (!DeviceOpen()) 
    return FALSE;
  
  if (!g_bHID) {
    DWORD dwCode = 0;
    switch (nReq) {
    case LED_BRIGHTNESS:
      dwCode = IOCTL_POWERMATE_SET_LED_BRIGHTNESS;
      break;
    case PULSE_SLEEP:
      dwCode = IOCTL_POWERMATE_PULSE_DURING_SLEEP;
      break;
    case PULSE_ALWAYS:
      dwCode = IOCTL_POWERMATE_PULSE_ALWAYS;
      break;
    case PULSE_SPEED:
      dwCode = IOCTL_POWERMATE_PULSE_SPEED;
      break;
    }
  
    DWORD dwBytesReturned;
    return DeviceIoControl(g_hPowerMate, dwCode,
                           &nVal, sizeof(nVal), 
                           NULL, 0, &dwBytesReturned, NULL);
  }
  else {
    UCHAR reportBuffer[9];
    memset(reportBuffer, 0, sizeof(reportBuffer));
    reportBuffer[1] = 0x41;
    reportBuffer[2] = 0x01;

    switch (nReq) {
    case LED_BRIGHTNESS:
      reportBuffer[3] = 0x01;
      reportBuffer[5] = (UCHAR)nVal;
      break;
    case PULSE_SLEEP:
      reportBuffer[3] = 0x02;
      reportBuffer[5] = (UCHAR)nVal;
      break;
    case PULSE_ALWAYS:
      reportBuffer[3] = 0x03;
      reportBuffer[5] = (UCHAR)nVal;
      break;
    case PULSE_SPEED:
      reportBuffer[3] = 0x04;
      reportBuffer[4] = 0x00;   // table 1
      if (nVal < 8) {
        reportBuffer[5] = 0x00; // divide
        reportBuffer[6] = (7-nVal) * 2;
      }
      else if (nVal > 8) {
        reportBuffer[5] = 0x02; // multiply
        reportBuffer[6] = (nVal-8) * 2;
      }
      else {
        reportBuffer[5] = 0x01; // normal
        reportBuffer[6] = 0;
      }
      break;
    }
    
    return HidD_SetFeature(g_hPowerMate, reportBuffer, sizeof(reportBuffer));
  }
}

/* Device control functions. */

#include "stdafx.h"
#include "plugin.h"

#include <initguid.h>

/*** PowerMate API for version 1.1 ***/

// define the PowerMate GUID
// {FC3DA4B7-1E9D-47f4-A7E3-151B97C163A6}
DEFINE_GUID(POWERMATE_GUID, 0xfc3da4b7, 0x1e9d, 0x47f4, 0xa7, 0xe3, 0x15, 0x1b, 0x97, 0xc1, 0x63, 0xa6);

#define IOCTL_POWERMATE_SET_LED_BRIGHTNESS CTL_CODE(    \
            FILE_DEVICE_UNKNOWN,            \
            0x807,                            \
            METHOD_BUFFERED,                \
            FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_DURING_SLEEP CTL_CODE(    \
            FILE_DEVICE_UNKNOWN,            \
            0x808,                            \
            METHOD_BUFFERED,                \
            FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_ALWAYS CTL_CODE(    \
            FILE_DEVICE_UNKNOWN,            \
            0x809,                            \
            METHOD_BUFFERED,                \
            FILE_ANY_ACCESS)

#define IOCTL_POWERMATE_PULSE_SPEED CTL_CODE(    \
            FILE_DEVICE_UNKNOWN,            \
            0x80A,                            \
            METHOD_BUFFERED,                \
            FILE_ANY_ACCESS)

HANDLE GetDeviceViaInterface( GUID* pGuid, DWORD instance)
{
	// Get handle to relevant device information set
	HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if(info==INVALID_HANDLE_VALUE)
		return NULL;

	// Get interface data for the requested instance
	SP_INTERFACE_DEVICE_DATA ifdata;
	ifdata.cbSize = sizeof(ifdata);
	if(!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instance, &ifdata))
	{
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get size of symbolic link name
	DWORD ReqLen;
	SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &ReqLen, NULL);
	PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(new char[ReqLen]);
	if( ifDetail==NULL)
	{
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get symbolic link name
	ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if( !SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, ReqLen, NULL, NULL))
	{
		SetupDiDestroyDeviceInfoList(info);
		delete ifDetail;
		return NULL;
	}

	// Open file
	HANDLE rv = CreateFile( ifDetail->DevicePath, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  //  FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL
	if( rv==INVALID_HANDLE_VALUE) rv = NULL;

	delete ifDetail;
	SetupDiDestroyDeviceInfoList(info);
	return rv;
}

/*** Control functions ***/

BOOL g_bRunning = FALSE;

DWORD WINAPI DeviceThread(LPVOID lpParam)
{
  HANDLE hPowerMate = (HANDLE)lpParam;
  while (g_bRunning) {
    signed char data[8];
    DWORD dwBytes;
    if (!ReadFile(hPowerMate, data, sizeof(data), &dwBytes, NULL)) {
      return GetLastError();                   // TODO: How to best report error?
    }
    if (dwBytes != 3) continue;

    PCHAR event = NULL;
    PCHAR payload = NULL;
    size_t pllen = 0;
    char buf[16];

    int n = data[1];
    if (data[0] & 1) {
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
    SF.send_event(event, payload, pllen, PLUGINNUM);
  }
  return 0;
}

HANDLE g_hPowerMate = NULL;
HANDLE g_hThread = NULL;

BOOL DeviceOpen()
{
  if (NULL == g_hPowerMate)
    g_hPowerMate = GetDeviceViaInterface((LPGUID)&POWERMATE_GUID, 0);
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

// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "Slinkx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CSlinkx properties

short CSlinkx::GetNumSlinkes()
{
  short result;
  GetProperty(0x1, VT_I2, (void*)&result);
  return result;
}

void CSlinkx::SetNumSlinkes(short propVal)
{
  SetProperty(0x1, VT_I2, propVal);
}

short CSlinkx::GetReportNoMatch()
{
  short result;
  GetProperty(0x2, VT_I2, (void*)&result);
  return result;
}

void CSlinkx::SetReportNoMatch(short propVal)
{
  SetProperty(0x2, VT_I2, propVal);
}

CString CSlinkx::GetLastError()
{
  CString result;
  GetProperty(0x3, VT_BSTR, (void*)&result);
  return result;
}

void CSlinkx::SetLastError(LPCTSTR propVal)
{
  SetProperty(0x3, VT_BSTR, propVal);
}

short CSlinkx::GetNumDevices()
{
  short result;
  GetProperty(0x9, VT_I2, (void*)&result);
  return result;
}

void CSlinkx::SetNumDevices(short propVal)
{
  SetProperty(0x9, VT_I2, propVal);
}

short CSlinkx::GetNumCM11As()
{
  short result;
  GetProperty(0xf, VT_I2, (void*)&result);
  return result;
}

void CSlinkx::SetNumCM11As(short propVal)
{
  SetProperty(0xf, VT_I2, propVal);
}

short CSlinkx::GetNumDXSs()
{
  short result;
  GetProperty(0x10, VT_I2, (void*)&result);
  return result;
}

void CSlinkx::SetNumDXSs(short propVal)
{
  SetProperty(0x10, VT_I2, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CSlinkx operations

long CSlinkx::AddDevice(LPCTSTR DeviceName, LPCTSTR DeviceFile, long IDNumber, long SlinkeNumber, long PortSelBits)
{
  long result;
  static BYTE parms[] =
    VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x4, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    DeviceName, DeviceFile, IDNumber, SlinkeNumber, PortSelBits);
  return result;
}

long CSlinkx::DeleteDevice(LPCTSTR DeviceName)
{
  long result;
  static BYTE parms[] =
    VTS_BSTR;
  InvokeHelper(0x5, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    DeviceName);
  return result;
}

long CSlinkx::Send(LPCTSTR DeviceName, LPCTSTR Message)
{
  long result;
  static BYTE parms[] =
    VTS_BSTR VTS_BSTR;
  InvokeHelper(0x6, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    DeviceName, Message);
  return result;
}

long CSlinkx::SendEx(LPCTSTR Message)
{
  long result;
  static BYTE parms[] =
    VTS_BSTR;
  InvokeHelper(0x7, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    Message);
  return result;
}

long CSlinkx::AddDeviceText(LPCTSTR DeviceName, LPCTSTR DeviceText, long IDNumber, long SlinkeNumber, long PortSelBits)
{
  long result;
  static BYTE parms[] =
    VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x8, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    DeviceName, DeviceText, IDNumber, SlinkeNumber, PortSelBits);
  return result;
}

CString CSlinkx::GetDeviceName(short DeviceNumber)
{
  CString result;
  static BYTE parms[] =
    VTS_I2;
  InvokeHelper(0xa, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
    DeviceNumber);
  return result;
}

short CSlinkx::GetNumCodes(LPCTSTR DeviceName)
{
  short result;
  static BYTE parms[] =
    VTS_BSTR;
  InvokeHelper(0xb, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
    DeviceName);
  return result;
}

CString CSlinkx::GetCodeName(LPCTSTR DeviceName, short CodeNum)
{
  CString result;
  static BYTE parms[] =
    VTS_BSTR VTS_I2;
  InvokeHelper(0xc, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
    DeviceName, CodeNum);
  return result;
}

long CSlinkx::Connect()
{
  long result;
  InvokeHelper(0xd, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
  return result;
}

long CSlinkx::Disconnect()
{
  long result;
  InvokeHelper(0xe, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
  return result;
}

void CSlinkx::AboutBox()
{
  InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

// Machine generated IDispatch wrapper class(es) created with ClassWizard
// $Header$
/////////////////////////////////////////////////////////////////////////////
// CSlinkx wrapper class

class CSlinkx : public COleDispatchDriver
{
public:
  CSlinkx() {}		// Calls COleDispatchDriver default constructor
  CSlinkx(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
  CSlinkx(const CSlinkx& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

public:
  CLSID const& GetClsid()
    {
      static CLSID const clsid
        = { 0x8671ac36, 0xf9ac, 0x11d1, { 0xa1, 0x81, 0x0, 0x40, 0x5, 0x5d, 0xb4, 0x87 } };
      return clsid;
    }
  BOOL Create(COleException* pError = NULL) 
    {
      return CreateDispatch(GetClsid(), pError);
    }

  // Attributes
public:
  short GetNumSlinkes();
  void SetNumSlinkes(short);
  short GetReportNoMatch();
  void SetReportNoMatch(short);
  CString GetLastError();
  void SetLastError(LPCTSTR);
  short GetNumDevices();
  void SetNumDevices(short);
  short GetNumCM11As();
  void SetNumCM11As(short);
  short GetNumDXSs();
  void SetNumDXSs(short);

  // Operations
public:
  long AddDevice(LPCTSTR DeviceName, LPCTSTR DeviceFile, long IDNumber, long SlinkeNumber, long PortSelBits);
  long DeleteDevice(LPCTSTR DeviceName);
  long Send(LPCTSTR DeviceName, LPCTSTR Message);
  long SendEx(LPCTSTR Message);
  long AddDeviceText(LPCTSTR DeviceName, LPCTSTR DeviceText, long IDNumber, long SlinkeNumber, long PortSelBits);
  CString GetDeviceName(short DeviceNumber);
  short GetNumCodes(LPCTSTR DeviceName);
  CString GetCodeName(LPCTSTR DeviceName, short CodeNum);
  long Connect();
  long Disconnect();
  void AboutBox();
};

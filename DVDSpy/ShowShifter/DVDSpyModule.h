// DVDSpyModule.h : Declaration of the CDVDSpyModule

#ifndef __DVDSPYMODULE_H_
#define __DVDSPYMODULE_H_

#include "resource.h"       // main symbols

// Doc implies that a subcat is optional.  It isn't.  Fortunately, it
// doesn't seem to need to be recognized.
// {A28DB4C8-99FB-4b75-8BF6-7B5F0B8281FC}
DEFINE_GUID(SUBCATID_CONTROL_SPY,
0xa28db4c8, 0x99fb, 0x4b75, 0x8b, 0xf6, 0x7b, 0x5f, 0xb, 0x82, 0x81, 0xfc);

class ControlMonitor;
class MonitorCallback;

/////////////////////////////////////////////////////////////////////////////
// CDVDSpyModule
class ATL_NO_VTABLE CDVDSpyModule : 
  public CComObjectRootEx<CComMultiThreadModel>,
  public CComCoClass<CDVDSpyModule, &CLSID_DVDSpyModule>,
  public ISupportErrorInfo,
  public IDispatchImpl<IDVDSpyModule, &IID_IDVDSpyModule, &LIBID_ShowShifterSpyLib>,
  public IDispatchImpl<IHMNTimerCallback, &IID_IHMNTimerCallback, &LIBID_ShowShifterSpyLib>
{
public:
  CDVDSpyModule()
  {
  }

BEGIN_CATEGORY_MAP(CTestModule)
IMPLEMENTED_CATEGORY(CATID_HMN_CONTROL_MODULE)
IMPLEMENTED_CATEGORY(SUBCATID_CONTROL_SPY)
END_CATEGORY_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_DVDSPYMODULE)
DECLARE_NOT_AGGREGATABLE(CDVDSpyModule)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDVDSpyModule)
  COM_INTERFACE_ENTRY(IDVDSpyModule)
  COM_INTERFACE_ENTRY2(IDispatch, IDVDSpyModule)
  COM_INTERFACE_ENTRY(ISupportErrorInfo)
  COM_INTERFACE_ENTRY(IHMNModule)
  COM_INTERFACE_ENTRY(IHMNTimerCallback)
END_COM_MAP()

// ISupportsErrorInfo
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IHMNModule
  STDMETHOD(Init)(IHMNVortress * pVortress);
  STDMETHOD(PostInit)(IHMNVortress * pVortress) { 
    return S_OK;
  }
  STDMETHOD(PreTerm)(IHMNVortress * pVortress) {
    return S_OK;
  }
  STDMETHOD(Term)(IHMNVortress * pVortress);
  STDMETHOD(Activate)(IHMNVortress * pVortress, IHMNModule*) {
    return E_FAIL;
  }
  STDMETHOD(Deactivate)(IHMNVortress * pVortress, IHMNModule*) {
    return E_FAIL;
  }
  STDMETHOD(FullScreen)(IHMNVortress * pVortress) { 
    return S_OK;
  }
  STDMETHOD(Windowed)(IHMNVortress * pVortress) { 
    return S_OK;
  }
  STDMETHOD(Hidden)(IHMNVortress * pVortress) { 
    return S_OK; 
  }
  STDMETHOD(ProcessUserInput)(DWORD dwCode, DWORD dwData, BOOL * bHandled) {
    return E_FAIL;
  }
  STDMETHOD(GetModuleInfo)(HMNModuleInfo* pModuleInfo) {
    if (NULL == pModuleInfo)
      return E_POINTER;
    if (!FillModuleInfo(pModuleInfo, _Module.GetModuleInstance(),
                        _T("DVDSpy ShowShifter Module"), GetObjectCLSID(),
                        *GetCategoryMap()[1].pcatid))
      return E_FAIL;
    return S_OK;
  }
  STDMETHOD(ParseCommand)(IHMNVortress * pVortress, BSTR cmd){
    return E_FAIL;
  }
  STDMETHOD(QueryTermFailed)(IHMNVortress* pVortress, EnumTermType ett){
    return S_OK;
  }
  STDMETHOD(QueryTerm)(IHMNVortress* pVortress, EnumTermType ett, VARIANT_BOOL* pBool) {
    if (NULL == pBool)
      return E_POINTER;
    *pBool = VARIANT_TRUE;
    return S_OK;
  }

// IHMNTimerCallback
  STDMETHOD(OnTimeout)(IHMNTimer *pTimer, DWORD dwUserData);

protected:
  CComPtr<IHMNVortress> m_pVortress;
  CComPtr<IHMNUserInterface> m_pUI;
  CComPtr<IHMNTimer> m_pTimer;
  // Timers are called in another thread.  Guard our data structures.
  // Another alternative would be to make the module apartment
  // threaded, so that the real work always got done in the SSF main
  // thread.  That may be necessary if the UI data structures are not
  // properly protected.
  CComAutoCriticalSection m_csMonitor;
  MonitorCallback *m_pDVDSpy;
  CComBSTR m_sModule;
  vector<ControlMonitor*> m_monitors;
};

#endif //__DVDSPYMODULE_H_

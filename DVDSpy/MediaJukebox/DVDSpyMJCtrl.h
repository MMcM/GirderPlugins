// DVDSpyMJCtrl.h : Declaration of the CDVDSpyMJCtrl
// $Header$

#ifndef __DVDSPYMJCTRL_H_
#define __DVDSPYMJCTRL_H_

#include "resource.h"       // main symbols
#include <atlctl.h>

extern LONG OpenDVDSpyMJReg(CRegKey& reg);

/////////////////////////////////////////////////////////////////////////////
// CDVDSpyMJCtrl
class ATL_NO_VTABLE CDVDSpyMJCtrl : 
  public CComObjectRootEx<CComSingleThreadModel>,
  public IDispatchImpl<IDVDSpyMJCtrl, &IID_IDVDSpyMJCtrl, &LIBID_DVDSPYMJLib>,
  public CComCompositeControl<CDVDSpyMJCtrl>,
  public IPersistStreamInitImpl<CDVDSpyMJCtrl>,
  public IOleControlImpl<CDVDSpyMJCtrl>,
  public IOleObjectImpl<CDVDSpyMJCtrl>,
  public IOleInPlaceActiveObjectImpl<CDVDSpyMJCtrl>,
  public IViewObjectExImpl<CDVDSpyMJCtrl>,
  public IOleInPlaceObjectWindowlessImpl<CDVDSpyMJCtrl>,
  public IPersistStorageImpl<CDVDSpyMJCtrl>,
  public ISpecifyPropertyPagesImpl<CDVDSpyMJCtrl>,
  public IQuickActivateImpl<CDVDSpyMJCtrl>,
  public IDataObjectImpl<CDVDSpyMJCtrl>,
  public IProvideClassInfo2Impl<&CLSID_DVDSpyMJCtrl, NULL, &LIBID_DVDSPYMJLib>,
  public CComCoClass<CDVDSpyMJCtrl, &CLSID_DVDSpyMJCtrl>
{
public:
  CDVDSpyMJCtrl();
  ~CDVDSpyMJCtrl();

DECLARE_REGISTRY_RESOURCEID(IDR_DVDSPYMJCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDVDSpyMJCtrl)
  COM_INTERFACE_ENTRY(IDVDSpyMJCtrl)
  COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(IViewObjectEx)
  COM_INTERFACE_ENTRY(IViewObject2)
  COM_INTERFACE_ENTRY(IViewObject)
  COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
  COM_INTERFACE_ENTRY(IOleInPlaceObject)
  COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
  COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
  COM_INTERFACE_ENTRY(IOleControl)
  COM_INTERFACE_ENTRY(IOleObject)
  COM_INTERFACE_ENTRY(IPersistStreamInit)
  COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
  COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
  COM_INTERFACE_ENTRY(IQuickActivate)
  COM_INTERFACE_ENTRY(IPersistStorage)
  COM_INTERFACE_ENTRY(IDataObject)
  COM_INTERFACE_ENTRY(IProvideClassInfo)
  COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CDVDSpyMJCtrl)
  PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
  PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
  // Example entries
  // PROP_ENTRY("Property Description", dispid, clsid)
  // PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CDVDSpyMJCtrl)
  CHAIN_MSG_MAP(CComCompositeControl<CDVDSpyMJCtrl>)
  COMMAND_HANDLER(IDC_RESET, BN_CLICKED, OnClickedReset)
  MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CDVDSpyMJCtrl)
  //Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()

  STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
  {
    if (dispid == DISPID_AMBIENT_BACKCOLOR)
    {
      SetBackgroundColorFromAmbient();
      FireViewChange();
    }
    return IOleControlImpl<CDVDSpyMJCtrl>::OnAmbientPropertyChange(dispid);
  }



// IViewObjectEx
  DECLARE_VIEW_STATUS(0)

// IDVDSpyMJCtrl
public:
  STDMETHOD(Reset)();
  STDMETHOD(get_RefreshInterval)(/*[out, retval]*/ short *pVal);
  STDMETHOD(put_RefreshInterval)(/*[in]*/ short newVal);
  STDMETHOD(Init)(/*[in]*/ LPDISPATCH pDisp);

  enum { IDD = IDD_DVDSPYMJCTRL };

protected:
  LRESULT OnClickedReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  IMJAutomationPtr m_pMJ;
  int m_nRefreshInterval;
};

#endif //__DVDSPYMJCTRL_H_

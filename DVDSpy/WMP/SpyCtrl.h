// SpyCtrl.h : Declaration of the CSpyCtrl

#ifndef __SPYCTRL_H_
#define __SPYCTRL_H_

#include "resource.h"       // main symbols

// Taken from the WMPUI.DLL type library.

EXTERN_C const IID IID_IWMPTextCtrl;
EXTERN_C const CLSID CLSID_TextControl;

MIDL_INTERFACE("237DAC8E-0E32-11D3-A2E2-00C04F79F88E")
IWMPTextCtrl : public IDispatch
{
 public:
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_backgroundColor( 
                                                                                        /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_backgroundColor( 
                                                                                        /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_fontFace( 
                                                                                 /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_fontFace( 
                                                                                 /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_fontStyle( 
                                                                                  /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_fontStyle( 
                                                                                  /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_fontSize( 
                                                                                 /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_fontSize( 
                                                                                 /* [in] */ long pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_foregroundColor( 
                                                                                        /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_foregroundColor( 
                                                                                        /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_hoverBackgroundColor( 
                                                                                             /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_hoverBackgroundColor( 
                                                                                             /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_hoverForegroundColor( 
                                                                                             /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_hoverForegroundColor( 
                                                                                             /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_hoverFontStyle( 
                                                                                       /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_hoverFontStyle( 
                                                                                       /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_value( 
                                                                              /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_value( 
                                                                              /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_toolTip( 
                                                                                /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_toolTip( 
                                                                                /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_disabledFontStyle( 
                                                                                          /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_disabledFontStyle( 
                                                                                          /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_disabledForegroundColor( 
                                                                                                /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_disabledForegroundColor( 
                                                                                                /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_disabledBackgroundColor( 
                                                                                                /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_disabledBackgroundColor( 
                                                                                                /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_fontSmoothing( 
                                                                                      /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_fontSmoothing( 
                                                                                      /* [in] */ VARIANT_BOOL pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_justification( 
                                                                                      /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_justification( 
                                                                                      /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_wordWrap( 
                                                                                 /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_wordWrap( 
                                                                                 /* [in] */ VARIANT_BOOL pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_cursor( 
                                                                               /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_cursor( 
                                                                               /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_scrolling( 
                                                                                  /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_scrolling( 
                                                                                  /* [in] */ VARIANT_BOOL pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_scrollingDirection( 
                                                                                           /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_scrollingDirection( 
                                                                                           /* [in] */ BSTR pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_scrollingDelay( 
                                                                                       /* [retval][out] */ int __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_scrollingDelay( 
                                                                                       /* [in] */ int pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_scrollingAmount( 
                                                                                        /* [retval][out] */ int __RPC_FAR *pVal) = 0;
        
  virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_scrollingAmount( 
                                                                                        /* [in] */ int pVal) = 0;
        
  virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_textWidth( 
                                                                                  /* [retval][out] */ int __RPC_FAR *pVal) = 0;
        
};
    
/////////////////////////////////////////////////////////////////////////////
// CSpyCtrl
class ATL_NO_VTABLE CSpyCtrl : 
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CSpyCtrl, &CLSID_SpyCtrl>,
  public ISupportErrorInfo,
  public IConnectionPointContainerImpl<CSpyCtrl>,
  public IDispatchImpl<ISpyCtrl, &IID_ISpyCtrl, &LIBID_WMPSPYLib>
{
public:
  CSpyCtrl()
  {
  }
  ~CSpyCtrl()
  {
  }

DECLARE_REGISTRY_RESOURCEID(IDR_SPYCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CSpyCtrl)
  COM_INTERFACE_ENTRY(ISpyCtrl)
  COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(ISupportErrorInfo)
  COM_INTERFACE_ENTRY(IConnectionPointContainer)
  COM_INTERFACE_ENTRY_AUTOAGGREGATE_BLIND(m_default.p,CLSID_TextControl)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CSpyCtrl)
END_CONNECTION_POINT_MAP()


// ISupportsErrorInfo
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ISpyCtrl
public:
  STDMETHOD(get_value)(/*[out, retval]*/ BSTR *pVal);
  STDMETHOD(put_value)(/*[in]*/ BSTR newVal);
  STDMETHOD(get_event)(/*[out, retval]*/ BSTR *pVal);
  STDMETHOD(put_event)(/*[in]*/ BSTR newVal);

  CComPtr<IUnknown> m_default;
  CComQIPtr<IWMPTextCtrl> m_text;
  CComBSTR m_event;
};

#endif //__SPYCTRL_H_

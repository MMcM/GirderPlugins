/////////////////////////////////////////////////////////////////////////////
//
// SpyPlugin.h : Declaration of the CSpyPlugin
// $Header$
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SPYPLUGIN_H_
#define __SPYPLUGIN_H_

#pragma once

// {0A90CDAD-FD32-400d-BFF0-49BB692940D4}
DEFINE_GUID(CLSID_SpyPlugin, 0x0A90CDAD, 0xFD32, 0x400D, 0xBF, 0xF0, 0x49, 0xBB, 0x69, 0x29, 0x40, 0xD4);

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin
class ATL_NO_VTABLE CSpyPlugin : 
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CSpyPlugin, &CLSID_SpyPlugin>,
  public IWMPEvents,
  public IWMPPluginUI
{
public:
  CSpyPlugin();
  ~CSpyPlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_SPYPLUGIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSpyPlugin)
  COM_INTERFACE_ENTRY(IWMPPluginUI)
  COM_INTERFACE_ENTRY(IWMPEvents)
END_COM_MAP()

  // CComCoClass methods
  HRESULT FinalConstruct();
  void    FinalRelease();

  // IWMPPluginUI methods
  STDMETHODIMP SetCore(IWMPCore *pCore);
  STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow) { return E_NOTIMPL; }
  STDMETHODIMP Destroy() { return E_NOTIMPL; }
  STDMETHODIMP TranslateAccelerator(LPMSG lpmsg) { return E_NOTIMPL; }
  STDMETHODIMP DisplayPropertyPage(HWND hwndParent) { return E_NOTIMPL; }
  STDMETHODIMP GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty);
  STDMETHODIMP SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty);

  // IWMPEvents methods
  void STDMETHODCALLTYPE OpenStateChange( long NewState );
  void STDMETHODCALLTYPE PlayStateChange( long NewState );
  void STDMETHODCALLTYPE AudioLanguageChange( long LangID );
  void STDMETHODCALLTYPE StatusChange();
  void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param );
  void STDMETHODCALLTYPE NewStream();
  void STDMETHODCALLTYPE Disconnect( long Result );
  void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start );
  void STDMETHODCALLTYPE Error();
  void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description );
  void STDMETHODCALLTYPE EndOfStream( long Result );
  void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition);
  void STDMETHODCALLTYPE MarkerHit( long MarkerNum );
  void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit );
  void STDMETHODCALLTYPE CdromMediaChange( long CdromNum );
  void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change );
  void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change );
  void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName );
  void STDMETHODCALLTYPE MediaChange( IDispatch * Item );
  void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName );
  void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
  void STDMETHODCALLTYPE MediaCollectionChange();
  void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal );
  void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal );
  void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal);
  void STDMETHODCALLTYPE PlaylistCollectionChange();
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
  void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted);
  void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue);
  void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject);
  void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem );
  void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
  void STDMETHODCALLTYPE SwitchedToPlayerApplication();
  void STDMETHODCALLTYPE SwitchedToControl();
  void STDMETHODCALLTYPE PlayerDockedStateChange();
  void STDMETHODCALLTYPE PlayerReconnect();
  void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState );
  void STDMETHODCALLTYPE KeyPress( short nKeyAscii );
  void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState );
  void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY );
  void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY );

  void NewElapsed(BOOL bForce);
private:
  void ReleaseCore();

  CComPtr<IWMPCore> m_spCore;
  CComPtr<IConnectionPoint> m_spConnectionPoint;
  DWORD m_dwAdviseCookie;
  UINT m_idTimer;
  CComBSTR m_bstrElapsed;
};

#endif //__SPYPLUGIN_H_

/////////////////////////////////////////////////////////////////////////////
//
// SpyPlugin.cpp : Implementation of CSpyPlugin
// $Header$
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SpyPlugin.h"
#include "DVDSpy.h"

CSpyPlugin *g_pTimerSpy = NULL;

void CALLBACK Timer(HWND, UINT, UINT, DWORD)
{
  if (NULL != g_pTimerSpy)
    g_pTimerSpy->NewElapsed(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::CSpyPlugin
// Constructor

CSpyPlugin::CSpyPlugin()
{
  m_dwAdviseCookie = 0;
  m_idTimer = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::~CSpyPlugin
// Destructor

CSpyPlugin::~CSpyPlugin()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT CSpyPlugin::FinalConstruct()
{
  return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CSpyPlugin::FinalRelease()
{
  ReleaseCore();
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::SetCore
// Set WMP core interface

HRESULT CSpyPlugin::SetCore(IWMPCore *pCore)
{
  HRESULT hr;

  // Release any existing WMP core interfaces.
  ReleaseCore();

  // If we get passed a NULL core, this means that the plugin is being
  // shutdown.

  if (pCore == NULL) {
    return S_OK;
  }

  m_spCore = pCore;

  // Connect up the event interface.
  CComPtr<IConnectionPointContainer> spConnectionContainer;

  hr = m_spCore->QueryInterface(&spConnectionContainer);

  if (SUCCEEDED(hr)) {
    hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents), 
                                                    &m_spConnectionPoint);
  }

  if (SUCCEEDED(hr)) {
    hr = m_spConnectionPoint->Advise(GetUnknown(), &m_dwAdviseCookie);

    if ((FAILED(hr)) || (0 == m_dwAdviseCookie)) {
      m_spConnectionPoint = NULL;
    }
  }

  m_idTimer = SetTimer(NULL, 0, 1000, Timer);
  g_pTimerSpy = this;

  return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::ReleaseCore
// Release WMP core interfaces

void CSpyPlugin::ReleaseCore()
{
  if (this == g_pTimerSpy) {
    KillTimer(NULL, m_idTimer);
    m_idTimer = 0;
    g_pTimerSpy = NULL;
  }
  if (m_spConnectionPoint) {
    if (0 != m_dwAdviseCookie) {
      m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
      m_dwAdviseCookie = 0;
    }
    m_spConnectionPoint = NULL;
  }

  if (m_spCore) {
    m_spCore = NULL;
    GirderEvent("WMP.Close", (LPCSTR)NULL);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::GetProperty
// Get plugin property

HRESULT CSpyPlugin::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
  if (NULL == pvarProperty) {
    return E_POINTER;
  }

  return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CSpyPlugin::SetProperty
// Set plugin property

HRESULT CSpyPlugin::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
  return E_NOTIMPL;
}

/** Events **/

void CSpyPlugin::OpenStateChange(long NewState)
{
  switch (NewState) {
  case wmposUndefined:
    break;
  case wmposPlaylistChanging:
    break;
  case wmposPlaylistLocating:
    break;
  case wmposPlaylistConnecting:
    break;
  case wmposPlaylistLoading:
    break;
  case wmposPlaylistOpening:
    break;
  case wmposPlaylistOpenNoMedia:
    break;
  case wmposPlaylistChanged:
    break;
  case wmposMediaChanging:
    break;
  case wmposMediaLocating:
    break;
  case wmposMediaConnecting:
    break;
  case wmposMediaLoading:
    break;
  case wmposMediaOpening:
    break;
  case wmposMediaOpen:
    break;
  case wmposBeginCodecAcquisition:
    break;
  case wmposEndCodecAcquisition:
    break;
  case wmposBeginLicenseAcquisition:
    break;
  case wmposEndLicenseAcquisition:
    break;
  case wmposBeginIndividualization:
    break;
  case wmposEndIndividualization:
    break;
  case wmposMediaWaiting:
    break;
  case wmposOpeningUnknownURL:
    break;
  default:
    break;
  }
}

void CSpyPlugin::PlayStateChange(long NewState)
{
  LPCSTR szPlayState = "Unknown";
  switch (NewState) {
  case wmppsStopped:
    szPlayState = "Stopped";
    break;
  case wmppsPaused:
    szPlayState = "Paused";
    break;
  case wmppsPlaying:
    szPlayState = "Playing";
    break;
  case wmppsScanForward:
    szPlayState = "FF";
    break;
  case wmppsScanReverse:
    szPlayState = "Rewind";
    break;
  case wmppsBuffering:
    szPlayState = "Buffering";
    break;
  case wmppsWaiting:
    szPlayState = "Waiting";
    break;
  case wmppsMediaEnded:
    break;
  case wmppsTransitioning:
    break;
  case wmppsReady:
    szPlayState = "Ready";
    break;
  case wmppsReconnecting:
    szPlayState = "Reconnecting";
    break;
  }
  GirderEvent("WMP.PlayState", szPlayState);
}

void CSpyPlugin::AudioLanguageChange(long LangID)
{
}

void CSpyPlugin::StatusChange()
{
  if (!m_spCore) return;
  CComBSTR bstrStatus;
  if (SUCCEEDED(m_spCore->get_status(&bstrStatus))) {
    GirderEvent("WMP.Status", bstrStatus);
  }
}

void CSpyPlugin::ScriptCommand(BSTR scType, BSTR Param)
{
}

void CSpyPlugin::NewStream()
{
}

void CSpyPlugin::Disconnect(long Result)
{
}

void CSpyPlugin::Buffering(VARIANT_BOOL Start)
{
}

void CSpyPlugin::Error()
{
  CComPtr<IWMPError>      spError;
  CComPtr<IWMPErrorItem>  spErrorItem;
  HRESULT                 dwError = S_OK;
  HRESULT                 hr = S_OK;

  if (m_spCore) {
    hr = m_spCore->get_error(&spError);

    if (SUCCEEDED(hr)) {
      hr = spError->get_item(0, &spErrorItem);
    }

    if (SUCCEEDED(hr)) {
      hr = spErrorItem->get_errorCode((long *) &dwError);
    }
  }
}

void CSpyPlugin::Warning(long WarningType, long Param, BSTR Description)
{
}

void CSpyPlugin::EndOfStream(long Result)
{
}

void CSpyPlugin::PositionChange(double oldPosition, double newPosition)
{
  NewElapsed(TRUE);
}

void CSpyPlugin::NewElapsed(BOOL bForce)
{
  if (!m_spCore) return;
  CComPtr<IWMPControls> spControls;
  if (FAILED(m_spCore->get_controls(&spControls))) return;
  CComBSTR bstrElapsed;
  if (SUCCEEDED(spControls->get_currentPositionString(&bstrElapsed))) {
    if (!bForce && (bstrElapsed == m_bstrElapsed)) 
      return;
    m_bstrElapsed.Empty();
    BSTR bstr = bstrElapsed.Detach();
    m_bstrElapsed.Attach(bstr);
    GirderEvent("WMP.Elapsed", bstr);
  }
}

void CSpyPlugin::MarkerHit(long MarkerNum)
{
}

void CSpyPlugin::DurationUnitChange(long NewDurationUnit)
{
}

void CSpyPlugin::CdromMediaChange(long CdromNum)
{
}

void CSpyPlugin::PlaylistChange(IDispatch * Playlist, 
                                WMPPlaylistChangeEventType change)
{
  switch (change) {
  case wmplcUnknown:
    break;
  case wmplcClear:
    break;
  case wmplcInfoChange:
    break;
  case wmplcMove:
    break;
  case wmplcDelete:
    break;
  case wmplcInsert:
    break;
  case wmplcAppend:
    break;
  case wmplcPrivate:
    break;
  case wmplcNameChange:
    break;
  case wmplcMorph:
    break;
  case wmplcSort:
    break;
  case wmplcLast:
    break;
  default:
    break;
  }
}

void CSpyPlugin::CurrentPlaylistChange(WMPPlaylistChangeEventType change)
{
  switch (change) {
  case wmplcUnknown:
    break;
  case wmplcClear:
    break;
  case wmplcInfoChange:
    break;
  case wmplcMove:
    break;
  case wmplcDelete:
    break;
  case wmplcInsert:
    break;
  case wmplcAppend:
    break;
  case wmplcPrivate:
    break;
  case wmplcNameChange:
    break;
  case wmplcMorph:
    break;
  case wmplcSort:
    break;
  case wmplcLast:
    break;
  default:
    break;
  }
}

void CSpyPlugin::CurrentPlaylistItemAvailable(BSTR bstrItemName)
{
}

void CSpyPlugin::MediaChange(IDispatch * Item)
{
  if (!m_spCore) return;
  CComPtr<IWMPControls> spControls;
  if (FAILED(m_spCore->get_controls(&spControls))) return;
  CComPtr<IWMPMedia> spCurrent;
  if (FAILED(spControls->get_CurrentItem(&spCurrent))) return;
  
  // Only concerned about updates to title and duration while playing.
  CComQIPtr<IWMPMedia> spMedia = Item;
  if (!spMedia) return;
  if (spMedia != spCurrent) return;
  
  CComBSTR bstrURL, bstrTitle, bstrDuration;
#if 0
  if (SUCCEEDED(spMedia->get_sourceURL(&bstrURL))) {
    GirderEvent("WMP.URL", bstrURL);
  }
#endif
  if (SUCCEEDED(spMedia->get_name(&bstrTitle))) {
    GirderEvent("WMP.Title", bstrTitle);
  }
  if (SUCCEEDED(spMedia->get_durationString(&bstrDuration))) {
    GirderEvent("WMP.Duration", bstrDuration);
  }
}

void CSpyPlugin::CurrentMediaItemAvailable(BSTR bstrItemName)
{
}

void CSpyPlugin::CurrentItemChange(IDispatch *pdispMedia)
{
  CComQIPtr<IWMPMedia> spMedia = pdispMedia;
  if (!spMedia) return;
  
  CComBSTR bstrURL, bstrTitle, bstrDuration;
  if (SUCCEEDED(spMedia->get_sourceURL(&bstrURL))) {
    GirderEvent("WMP.URL", bstrURL);
  }
  if (SUCCEEDED(spMedia->get_name(&bstrTitle))) {
    GirderEvent("WMP.Title", bstrTitle);
  }
  if (SUCCEEDED(spMedia->get_durationString(&bstrDuration))) {
    GirderEvent("WMP.Duration", bstrDuration);
  }
}

void CSpyPlugin::MediaCollectionChange()
{
}

void CSpyPlugin::MediaCollectionAttributeStringAdded(BSTR bstrAttribName,  
                                                     BSTR bstrAttribVal)
{
}

void CSpyPlugin::MediaCollectionAttributeStringRemoved(BSTR bstrAttribName,  
                                                       BSTR bstrAttribVal)
{
}

void CSpyPlugin::MediaCollectionAttributeStringChanged(BSTR bstrAttribName, 
                                                       BSTR bstrOldAttribVal, 
                                                       BSTR bstrNewAttribVal)
{
}

void CSpyPlugin::PlaylistCollectionChange()
{
}

void CSpyPlugin::PlaylistCollectionPlaylistAdded(BSTR bstrPlaylistName)
{
}

void CSpyPlugin::PlaylistCollectionPlaylistRemoved(BSTR bstrPlaylistName)
{
}

void CSpyPlugin::PlaylistCollectionPlaylistSetAsDeleted(BSTR bstrPlaylistName, 
                                                        VARIANT_BOOL varfIsDeleted)
{
}

void CSpyPlugin::ModeChange(BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CSpyPlugin::MediaError(IDispatch * pMediaObject)
{
}

void CSpyPlugin::OpenPlaylistSwitch(IDispatch *pItem)
{
}

void CSpyPlugin::DomainChange(BSTR strDomain)
{
}

void CSpyPlugin::SwitchedToPlayerApplication()
{
}

void CSpyPlugin::SwitchedToControl()
{
}

void CSpyPlugin::PlayerDockedStateChange()
{
}

void CSpyPlugin::PlayerReconnect()
{
}

void CSpyPlugin::Click(short nButton, short nShiftState, long fX, long fY)
{
}

void CSpyPlugin::DoubleClick(short nButton, short nShiftState, long fX, long fY)
{
}

void CSpyPlugin::KeyDown(short nKeyCode, short nShiftState)
{
}

void CSpyPlugin::KeyPress(short nKeyAscii)
{
}

void CSpyPlugin::KeyUp(short nKeyCode, short nShiftState)
{
}

void CSpyPlugin::MouseDown(short nButton, short nShiftState, long fX, long fY)
{
}

void CSpyPlugin::MouseMove(short nButton, short nShiftState, long fX, long fY)
{
}

void CSpyPlugin::MouseUp(short nButton, short nShiftState, long fX, long fY)
{
}

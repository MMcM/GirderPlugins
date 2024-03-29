// SpyCtrl.cpp : Implementation of CSpyCtrl
// $Header$
#include "stdafx.h"
#include "WMPSpy.h"
#include "SpyCtrl.h"
#include "DVDSpy.h"

/////////////////////////////////////////////////////////////////////////////
// CSpyCtrl

// This is the normal WMP Text control in WMPUI.DLL.  We let it handle
// everything except properties.  Mostly, it is just an in-place OLE
// control.  However, there are a couple of specific undocumented
// interfaces ({E4C6CC44-BF91-11D2-BE97-00C04F8EDCC4} and
// {AEA0A1A1-0423-11D3-BD3F-00C04F6EA5AE}) that require using it.
const IID IID_IWMPTextCtrl = {0x237DAC8E,0x0E32,0x11D3,{0xA2,0xE2,0x00,0xC0,0x4F,0x79,0xF8,0x8E}};
const CLSID CLSID_TextControl = {0xDDDA102E,0x0E17,0x11D3,{0xA2,0xE2,0x00,0xC0,0x4F,0x79,0xF8,0x8E}};

STDMETHODIMP CSpyCtrl::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID* arr[] = 
  {
    &IID_ISpyCtrl
  };
  for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
  {
    if (InlineIsEqualGUID(*arr[i],riid))
      return S_OK;
  }
  return S_FALSE;
}

STDMETHODIMP CSpyCtrl::get_event(BSTR *pVal)
{
  return m_event.CopyTo(pVal);
}

STDMETHODIMP CSpyCtrl::put_event(BSTR newVal)
{
  m_event = newVal;
  return S_OK;
}

// If anyone makes the control visible, it displays just like the
// regular text control, without things like font settings, of course.

STDMETHODIMP CSpyCtrl::get_value(BSTR *pVal)
{
  if (!m_text && !!m_default)
    m_text = m_default;
  if (!!m_text)
    return m_text->get_value(pVal);
  else
    return S_OK;
}

STDMETHODIMP CSpyCtrl::put_value(BSTR newVal)
{
  if (!m_text && !!m_default)
    m_text = m_default;
  if (!!m_text)
    m_text->put_value(newVal);

  GirderEvent(m_event, newVal);

  return S_OK;
}

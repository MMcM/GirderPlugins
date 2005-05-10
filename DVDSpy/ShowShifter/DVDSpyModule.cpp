// DVDSpyModule.cpp : Implementation of CDVDSpyModule
// $Header$

#include "stdafx.h"
#include "ShowShifterSpy.h"
#include "DVDSpyModule.h"
#include "..\Hook\DisplaySpyHook.h"

class ControlMonitor
{
public:
  ControlMonitor(LPTSTR szIdent) : m_sIdent(szIdent) {
    m_bMonitorVisible = FALSE;
  }
  ~ControlMonitor() {
  }
  
  void Reset() {
    m_sValue.Empty();
  }
  void Update(BSTR sValue, MonitorCallback *pDVDSpy);

  CComBSTR m_sIdent;
  CComBSTR m_sValue;
  BOOL m_bMonitorVisible;
};

class MonitorCallback
{
public:
  void ModuleChanged(LPCTSTR szName);
  void ControlChanged(LPCTSTR szIdent, LPCTSTR szLabel);
  void ControlGone(LPCTSTR szIdent);

  static MonitorCallback *Create();
  ~MonitorCallback() {
  }
  
protected:
  void SendEvent(LPCTSTR szEvent, LPCTSTR szSuffix, LPCTSTR szPayload);
  MonitorCallback(HWND hwnd) {
    m_hwnd = hwnd;
  }
  HWND m_hwnd;
};

/////////////////////////////////////////////////////////////////////////////
// CDVDSpyModule

STDMETHODIMP CDVDSpyModule::Init(IHMNVortress * pVortress)
{
  HRESULT hr;

  m_csMonitor.Lock();

  m_pDVDSpy = MonitorCallback::Create();
#if 0
  // Note that timer knows how to find it if need be.
  if (NULL == m_pDVDSpy) {
    TRACE("DVDSpy not running.");
    m_csMonitor.Unlock();
    return S_OK;
  }
#endif
  
  m_pVortress = pVortress;

  hr = m_pVortress->GetUI(&m_pUI);
  if (FAILED(hr)) {
    TRACE("Spy failed to get UI.");
    m_csMonitor.Unlock();
    return hr;
  }

  CRegKey settings;
  if (ERROR_SUCCESS != settings.Open(HKEY_LOCAL_MACHINE,
                                     DVDSPY_KEY "\\ShowShifter")) {
    TRACE("Spy missing registry settings.");
    m_csMonitor.Unlock();
    return E_FAIL;
  }
    
  CRegKey idents;
  if (ERROR_SUCCESS != idents.Open(settings, "ControlIdents")) {
    TRACE("Spy missing registry settings.");
    m_csMonitor.Unlock();
    return E_FAIL;
  }
  
  DWORD dwIndex = 0;
  while (TRUE) {
    char szName[128], szValue[128];
    DWORD dwType, dwNLen, dwVLen;
    dwNLen = sizeof(szName);
    dwVLen = sizeof(szValue);
    if (ERROR_SUCCESS != RegEnumValue(idents, dwIndex++, szName, &dwNLen,
                                      NULL, &dwType, (LPBYTE)szValue, &dwVLen))
      break;
    ControlMonitor *cm = new ControlMonitor(szName);
    if ((REG_SZ == dwType) && !_stricmp(szValue, "visible"))
      cm->m_bMonitorVisible = TRUE;
    m_monitors.push_back(cm);
    TRACE(_T("Spy monitoring %s\n"), szName);
  }

  DWORD dwInterval = 1000, dwSetting;
  if (ERROR_SUCCESS == settings.QueryValue(dwSetting, "Interval"))
    dwInterval = dwSetting;

  hr = m_pUI->CreateTimer(&m_pTimer);
  if (FAILED(hr)) {
    TRACE("Spy failed to create timer.");
    m_csMonitor.Unlock();
    return hr;
  }
  
  hr = m_pTimer->SetTimer(dwInterval, this, NULL);

  m_csMonitor.Unlock();

  return S_OK ;
}

STDMETHODIMP CDVDSpyModule::Term(IHMNVortress * pVortress)
{
  m_csMonitor.Lock();

  if (!!m_pTimer) {
    m_pTimer->RemoveTimer();
    m_pTimer.Release();
  }

  for (vector<ControlMonitor*>::iterator iter = m_monitors.begin(); 
       iter != m_monitors.end(); iter++)
    delete *iter;
  m_monitors.clear();

  m_pUI.Release();
  m_pVortress.Release();
  
  delete m_pDVDSpy;
  m_pDVDSpy = NULL;

  m_csMonitor.Unlock();

  return S_OK;
}

STDMETHODIMP CDVDSpyModule::OnTimeout(IHMNTimer * pTimer, DWORD dwUserData)
{
  HRESULT hr;

  m_csMonitor.Lock();

  if (!m_pVortress) {
    m_csMonitor.Unlock();
    return S_OK;
  }

  if (NULL == m_pDVDSpy) {
    m_pDVDSpy = MonitorCallback::Create();
    if (NULL == m_pDVDSpy) {
      m_csMonitor.Unlock();
      return S_OK;
    }
    // TODO: Also consider IsWindow check of any existing one.
  }

  CComPtr<IHMNModule> pModule;
  hr = m_pVortress->GetActiveModule(&pModule);
  if (FAILED(hr)) {
    TRACE("Spy failed to get module");
    m_csMonitor.Unlock();
    return hr;
  }

  if (!pModule) {
    if (!!m_sModule) {
      m_sModule.Empty();
      m_pDVDSpy->ModuleChanged(NULL);
    }
  }
  else {
    HMNModuleInfo moduleInfo;
    memset(&moduleInfo, 0, sizeof(moduleInfo));
    moduleInfo.m_dwSize = sizeof(moduleInfo);
    hr = pModule->GetModuleInfo(&moduleInfo);
    if (FAILED(hr)) {
      TRACE("Spy failed to get module info");
      m_csMonitor.Unlock();
      return hr;
    }

    if (!(m_sModule == moduleInfo.m_bstrDisplayName)) {
      USES_CONVERSION;
      m_sModule = moduleInfo.m_bstrDisplayName;
      m_pDVDSpy->ModuleChanged(OLE2T(m_sModule));
    }

    SysFreeString(moduleInfo.m_bstrDisplayName);
    SysFreeString(moduleInfo.m_bstrMachineName);
    SysFreeString(moduleInfo.m_bstrFileName);
  }

  CComPtr<IHMNPage> pPage;
  hr = m_pUI->GetActivePage(&pPage);
  if (FAILED(hr)) {
    TRACE("Spy failed to get active page");
    m_csMonitor.Unlock();
    return hr;
  }

  for (vector<ControlMonitor*>::iterator iter = m_monitors.begin();
       iter != m_monitors.end(); iter++) {
    ControlMonitor *pMonitor = *iter;
    CComBSTR sValue;
    if (!!pPage) {
      CComPtr<IHMNControl> pControl;
      hr = pPage->GetDescendant(pMonitor->m_sIdent, &pControl);
      if (SUCCEEDED(hr)) {
        if (pMonitor->m_bMonitorVisible) {
          hr = pControl->IsVisible();
          if (SUCCEEDED(hr)) {
            sValue = (S_FALSE == hr) ? "invisible" : "visible";
          }
        }
        else
          hr = pControl->GetLabel(&sValue);
      }
    }
    pMonitor->Update(sValue, m_pDVDSpy);
  }

  m_csMonitor.Unlock();

  return S_OK;
}

void ControlMonitor::Update(BSTR sValue, MonitorCallback *pDVDSpy)
{
  if (m_sValue == sValue) return;

  USES_CONVERSION;

  m_sValue = sValue;

  if (!m_sValue)
    pDVDSpy->ControlGone(OLE2T(m_sIdent));
  else
    pDVDSpy->ControlChanged(OLE2T(m_sIdent), OLE2T(m_sValue));
}

void MonitorCallback::ModuleChanged(LPCTSTR szName)
{
  SendEvent("ShowShifter.Module", NULL, szName);
}

void MonitorCallback::ControlChanged(LPCTSTR szIdent, LPCTSTR szLabel)
{
  SendEvent("ShowShifter.UI", szIdent, szLabel);
}

void MonitorCallback::ControlGone(LPCTSTR szIdent)
{
  SendEvent("ShowShifter.NoUI", szIdent, NULL);
}

static BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

MonitorCallback *MonitorCallback::Create()
{
  HWND hwnd = NULL;
  EnumWindows(FindMonitorWindow, (LPARAM)&hwnd);
  if (NULL == hwnd)
    return NULL;
  else
    return new MonitorCallback(hwnd);
}

void MonitorCallback::SendEvent(LPCTSTR szEvent, LPCTSTR szSuffix, LPCTSTR szPayload)
{
  TRACE("Spy event: %s.%s '%s'\n", szEvent, szSuffix, szPayload);
  char buf[1024], *pbuf;
  strncpy(buf, szEvent, sizeof(buf));
  pbuf = buf + strlen(buf);
  if (NULL != szSuffix) {
    *pbuf++ = '.';
    strncpy(pbuf, szSuffix, sizeof(buf) - (pbuf - buf));
    pbuf += strlen(pbuf);
  }
  *pbuf++ = '\0';
  if (NULL == szPayload)
    *pbuf++ = 0;
  else {
    *pbuf++ = 1;
    strncpy(pbuf, szPayload, sizeof(buf) - (pbuf - buf));
    pbuf += strlen(pbuf) + 1;
  }
  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = buf;
  cd.cbData = pbuf - buf;
  SendMessage(m_hwnd, WM_COPYDATA, 0, (LPARAM)&cd);
}

STDMETHODIMP CDVDSpyModule::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID* arr[] = 
  {
    &IID_IDVDSpyModule
  };
  for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
  {
    if (InlineIsEqualGUID(*arr[i],riid))
      return S_OK;
  }
  return S_FALSE;
}

/* -*-Mode:C++-*-
$Header$

This server intercepts events to the IVIPlayerX control.

Unfortunately, none of the interfaces in the public type libraries
seems to receive any interesting events.

*/

#include <objbase.h>

#ifdef _DEBUG
#define _TRACE
#endif

const IID IID_PLAYER = {0x009EB4E7,0xE31A,0x4A87,{0x86,0x9E,0xEA,0x9D,0x90,0xE4,0x9F,0xF6}};
const CLSID CLSID_PLAYER = {0x65233F47,0x144C,0x4F6E,{0x8E,0xFD,0xEC,0x06,0x4A,0x04,0xED,0xDC}};

HKEY GetServerKey(REFCLSID rclsid)
{
  OLECHAR wszBuf[64];
  int cWide = StringFromGUID2(rclsid, wszBuf, sizeof(wszBuf)/sizeof(OLECHAR));

  char szBuf[128];
  PTSTR ps = szBuf;
  strcpy(ps, "CLSID\\");
  ps += strlen(ps);
  WideCharToMultiByte(CP_ACP, 0, wszBuf, cWide, ps, sizeof(szBuf) - (ps - szBuf),
                      NULL, NULL);
  ps += strlen(ps);
  strcat(ps, "\\InProcServer32");

  HKEY hKey;
  if (ERROR_SUCCESS != RegOpenKey(HKEY_CLASSES_ROOT, szBuf, &hKey))
    return NULL;
  return hKey;
}

LPFNGETCLASSOBJECT g_pRealFunction = NULL;

HRESULT GetRealFunction() 
{
  if (NULL == g_pRealFunction) {
    HKEY hKey = GetServerKey(CLSID_PLAYER);
    if (NULL == hKey) return E_FAIL;

    char szBuf[MAX_PATH];
    DWORD dwType, dwLen;
    dwLen = sizeof(szBuf);
    if ((ERROR_SUCCESS != RegQueryValueEx(hKey, NULL,
                                          NULL, &dwType, (PBYTE)szBuf, &dwLen)) ||
        (REG_SZ != dwType)) {
      RegCloseKey(hKey);
      return E_FAIL;
    }
    OLECHAR wszBuf[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, szBuf, -1, wszBuf, sizeof(wszBuf)/sizeof(OLECHAR));

    RegCloseKey(hKey);

    HINSTANCE hInst = CoLoadLibrary(wszBuf, TRUE);
    if (NULL == hInst) return E_FAIL;

    g_pRealFunction = (LPFNGETCLASSOBJECT)
      GetProcAddress(hInst, "DllGetClassObject");
    if (NULL == g_pRealFunction) return E_FAIL;
  }
  return S_OK;
}

class CPlayerWrapper : public IDispatch
{
  friend class CClassFactory;

protected:
  DWORD m_cRef;
  IUnknown *m_pInner;
  IDispatch *m_pDispatch;

  CPlayerWrapper() {
    m_cRef = 0;
    m_pInner = NULL;
    m_pDispatch = NULL;
  }

  ~CPlayerWrapper() {
    m_cRef++;
    if (NULL != m_pDispatch)
      m_pDispatch->Release();
    if (NULL != m_pInner)
      m_pInner->Release();
  }

public:
  /** IUnknown methods **/

  STDMETHODIMP_(DWORD) AddRef() {
    return InterlockedIncrement((LONG*)&m_cRef);
  }

  STDMETHODIMP_(DWORD) Release() { 
    DWORD cRef = (DWORD)InterlockedDecrement((LONG*)&m_cRef);
    if (cRef != 0)
      return cRef; 
    delete this; 
    return 0; 
  }

  STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
    if ((riid == IID_IUnknown) ||
        (riid == IID_IDispatch) ||
        (riid == IID_PLAYER)) {
      if (NULL == ppv)
        return E_POINTER;
      *ppv = this;
      AddRef();
      return S_OK;
    }
    return m_pInner->QueryInterface(riid, ppv);
  }

  /** IDispatch methods **/

  HRESULT GetDispatch() {
    if (NULL == m_pDispatch)
      return m_pInner->QueryInterface(IID_IDispatch, (void**)&m_pDispatch);
    else
      return S_OK;
  }

  STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {
    HRESULT hr = GetDispatch();
    if (FAILED(hr)) return hr;
    return m_pDispatch->GetTypeInfoCount(pctinfo);
  }
        
  STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {
    HRESULT hr = GetDispatch();
    if (FAILED(hr)) return hr;
    return m_pDispatch->GetTypeInfo(iTInfo, lcid, ppTInfo);
  }
        
  STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
                             LCID lcid, DISPID *rgDispId) {
    HRESULT hr = GetDispatch();
    if (FAILED(hr)) return hr;
    return m_pDispatch->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
  }
        
  STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                      DISPPARAMS *pDispParams, VARIANT *pVarResult,
                      EXCEPINFO *pExcepInfo, UINT *puArgErr) {
    HRESULT hr = GetDispatch();
    if (FAILED(hr)) return hr;
    hr = m_pDispatch->Invoke(dispIdMember, riid, lcid, wFlags, 
                             pDispParams, pVarResult,
                             pExcepInfo, puArgErr);
    return hr;
  }
};

class CClassFactory : public IClassFactory
{
protected:
  DWORD m_cRef;
  IClassFactory *m_pRealCF;

public:
  CClassFactory() {
    m_cRef = 0;
    m_pRealCF = NULL;
  }

  ~CClassFactory() {
    if (NULL != m_pRealCF)
      m_pRealCF->Release();
  }

  STDMETHODIMP_(DWORD) AddRef() {
    return InterlockedIncrement((LONG*)&m_cRef);
  }

  STDMETHODIMP_(DWORD) Release() { 
    DWORD cRef = (DWORD)InterlockedDecrement((LONG*)&m_cRef);
    if (cRef != 0)
      return m_cRef; 
    delete this; 
    return 0; 
  }

  STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
    if ((riid == IID_IUnknown) ||
        (riid == IID_IClassFactory)) {
      if (NULL == ppv)
        return E_POINTER;
      *ppv = this;
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void** ppv);
  STDMETHOD(LockServer)(BOOL fLock);

protected:
  HRESULT GetRealCF();
  HRESULT RealCreate(CPlayerWrapper *pWrapper, IUnknown *pUnkOuter);
};

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void** ppv)
{
  HRESULT hr;
  if (NULL == m_pRealCF) {
    hr = GetRealCF();
    if (FAILED(hr)) return hr;
  }
  CPlayerWrapper *pWrapper = new CPlayerWrapper();
  hr = RealCreate(pWrapper, pUnkOuter);
  if (FAILED(hr)) return hr;
  hr = pWrapper->QueryInterface(riid, ppv);
  if (FAILED(hr)) delete pWrapper;
  return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock) 
{
  HRESULT hr;
  if (fLock) {
    if (NULL == m_pRealCF) {
      hr = GetRealCF();
      if (FAILED(hr)) return hr;
    }
  }
  else {
    if (NULL != m_pRealCF) {
      hr = m_pRealCF->LockServer(fLock);
      if (FAILED(hr)) return hr;
    }
  }
  hr = CoLockObjectExternal(this, fLock, TRUE);
  if (FAILED(hr)) return hr;
  if (fLock) {
    hr = m_pRealCF->LockServer(fLock);
    if (FAILED(hr)) return hr;
  }
  return S_OK;
}

HRESULT CClassFactory::GetRealCF()
{
  HRESULT hr = GetRealFunction();
  if (FAILED(hr)) return hr;
  return (g_pRealFunction)(CLSID_PLAYER, IID_IClassFactory, (void**)&m_pRealCF);
}

HRESULT CClassFactory::RealCreate(CPlayerWrapper *pWrapper, IUnknown *pUnkOuter)
{
  if (NULL == pUnkOuter)
    pUnkOuter = pWrapper;
  return m_pRealCF->CreateInstance(pUnkOuter, IID_IUnknown,
                                   (void**)&pWrapper->m_pInner);
}

/*** In-proc server entry functions ***/

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppv)
{
  if (rclsid != CLSID_PLAYER) {
    HRESULT hr = GetRealFunction();
    if (FAILED(hr)) return hr;
    return (g_pRealFunction)(rclsid, riid, ppv);
  }

  CClassFactory *pCF = new CClassFactory();
  HRESULT hr = pCF->QueryInterface(riid, ppv);
  if (FAILED(hr)) delete pCF;
  return hr;
}

#if 0
STDAPI DllRegisterServer()
{
  TCHAR szFile[MAX_PATH];
  DWORD dwFileLen = GetModuleFileName(GetModuleHandle("WINDVDSPY"),
                                      szFile, sizeof(szFile));

  HKEY hKey = GetServerKey(CLSID_PLAYER);
  if (NULL == hKey) return SELFREG_E_CLASS;

  char szBuf[MAX_PATH];
  DWORD dwType, dwLen;
  dwLen = sizeof(szBuf);
  if ((ERROR_SUCCESS != RegQueryValueEx(hKey, "RealServer32",
                                        NULL, &dwType, (PBYTE)szBuf, &dwLen)) ||
      (REG_SZ != dwType)) {
      
    // Get default value.
    dwLen = sizeof(szBuf);
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, NULL,
                                         NULL, &dwType, (PBYTE)szBuf, &dwLen))
      return SELFREG_E_CLASS;

    // Remember that.
    if (ERROR_SUCCESS != RegSetValueEx(hKey, "RealServer32", 
                                       0, dwType, (PBYTE)szBuf, dwLen))
      return SELFREG_E_CLASS;
  }

  if (ERROR_SUCCESS != RegSetValueEx(hKey, NULL, 0, REG_SZ, 
                                     (PBYTE)szFile, dwFileLen))
    return SELFREG_E_CLASS;

  RegCloseKey(hKey);
  return S_OK;
}

STDAPI DllUnregisterServer()
{
  HKEY hKey = GetServerKey(CLSID_PLAYER);
  if (NULL == hKey) return SELFREG_E_CLASS;

  char szBuf[MAX_PATH];
  DWORD dwType, dwLen;
  dwLen = sizeof(szBuf);
  if ((ERROR_SUCCESS != RegQueryValueEx(hKey, "RealServer32",
                                        NULL, &dwType, (PBYTE)szBuf, &dwLen)) ||
      (REG_SZ != dwType))
    continue;

  if (ERROR_SUCCESS != RegSetValueEx(hKey, NULL, 0, REG_SZ,
                                     (PBYTE)szBuf, dwLen))
    return SELFREG_E_CLASS;

  if (ERROR_SUCCESS != RegDeleteValue(hKey, "RealServer32"))
    return SELFREG_E_CLASS;

  RegCloseKey(hKey);
  return S_OK;
}
#endif

/*** Hook entry function ***/

STDAPI WinDVDSpySetInner(LPFNGETCLASSOBJECT pRealFunction)
{
  g_pRealFunction = pRealFunction;
  return S_OK;
}

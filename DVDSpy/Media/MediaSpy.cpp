/* -*-Mode:C++-*-

This server registers itself in place of several DirectX objects.  It
remembers the original server location and returns instances that wrap
the functional instances, using COM aggregation.

The wrappers remember whatever instances the user has created.  An
additional DLL entry lets a timer message hook retrieve those
instances.

Additionally, if enabled (see ROTEnable.reg), FilterGraph instances
will register themselves in the running object table.  This allows the
Connect command in GraphEdit to display any filter graph in any
application.

*/

#include <dshow.h>

class CMediaWrapper;

struct CInterceptedClass
{
  REFCLSID m_rclsid;
  CMediaWrapper *m_pCurrentWrapper;

  CInterceptedClass(REFCLSID rclsid) : m_rclsid(rclsid) {
    m_pCurrentWrapper = NULL;
  }
};

CInterceptedClass g_classes[] = {
  { CLSID_FilterGraph },
  { CLSID_DVDNavigator },
  { CLSID_NULL }
};

CInterceptedClass *FindClass(REFCLSID rclsid)
{
  CInterceptedClass *pClass = g_classes;
  while (TRUE) {
    if (pClass->m_rclsid == CLSID_NULL)
      return NULL;
    if (pClass->m_rclsid == rclsid)
      return pClass;
    pClass++;
  }
}

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

class CMediaWrapper : public IUnknown
{
  friend class CClassFactory;

protected:
  CInterceptedClass *m_pClass;
  DWORD m_cRef;
  IUnknown *m_pInner;
  DWORD m_dwROT;

  CMediaWrapper(CInterceptedClass *pClass) : m_pClass(pClass) {
    m_cRef = 0;
    m_pInner = NULL;
    m_dwROT = 0;
  }

  ~CMediaWrapper() {
    m_cRef++;
    if (0 != m_dwROT) {
      IRunningObjectTable *pROT;
      if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(m_dwROT);
        pROT->Release();
      }
    }
    if (NULL != m_pInner)
      m_pInner->Release();
    InterlockedCompareExchange((PVOID*)&m_pClass->m_pCurrentWrapper, NULL, this);
  }

public:
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
    if (riid == IID_IUnknown) {
      if (NULL == ppv)
        return E_POINTER;
      *ppv = this;
      AddRef();
      return S_OK;
    }
    return m_pInner->QueryInterface(riid, ppv);
  }
};

class CClassFactory : public IClassFactory
{
protected:
  CInterceptedClass *m_pClass;
  DWORD m_cRef;
  IClassFactory *m_pRealCF;
  char m_szROTPrefix[16];

public:
  CClassFactory(CInterceptedClass *pClass) : m_pClass(pClass) {
    m_cRef = 0;
    m_pRealCF = NULL;
    m_szROTPrefix[0] = '\0';
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
  HRESULT RealCreate(CMediaWrapper *pWrapper, IUnknown *pUnkOuter);
  HRESULT AddToROT(CMediaWrapper *pWrapper);
};

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void** ppv)
{
  HRESULT hr;
  if (NULL == m_pRealCF) {
    hr = GetRealCF();
    if (FAILED(hr)) return hr;
  }
  CMediaWrapper *pWrapper = new CMediaWrapper(m_pClass);
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
  REFCLSID rclsid = m_pClass->m_rclsid;
  HKEY hKey = GetServerKey(rclsid);
  if (NULL == hKey) return E_FAIL;

  char szBuf[MAX_PATH];
  DWORD dwType, dwLen;
  dwLen = sizeof(szBuf);
  if ((ERROR_SUCCESS != RegQueryValueEx(hKey, "RealServer32",
                                        NULL, &dwType, (PBYTE)szBuf, &dwLen)) ||
      (REG_SZ != dwType)) {
    RegCloseKey(hKey);
    return E_FAIL;
  }
  OLECHAR wszBuf[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, szBuf, -1, wszBuf, sizeof(wszBuf)/sizeof(OLECHAR));

  if ((ERROR_SUCCESS == RegQueryValueEx(hKey, "SpyROTPrefix",
                                        NULL, &dwType, (PBYTE)szBuf, &dwLen)) &&
      (REG_SZ == dwType)) {
    strncpy(m_szROTPrefix, szBuf, sizeof(m_szROTPrefix));
  }
  RegCloseKey(hKey);
  
  HINSTANCE hInst = CoLoadLibrary(wszBuf, TRUE);
  if (NULL == hInst) return E_FAIL;
  LPFNGETCLASSOBJECT pFunc = (LPFNGETCLASSOBJECT)
    GetProcAddress(hInst, "DllGetClassObject");
  if (NULL == pFunc) return E_FAIL;

  return (pFunc)(rclsid, IID_IClassFactory, (void**)&m_pRealCF);
}

HRESULT CClassFactory::RealCreate(CMediaWrapper *pWrapper, IUnknown *pUnkOuter)
{
  if (NULL == pUnkOuter)
    pUnkOuter = pWrapper;

  HRESULT hr = m_pRealCF->CreateInstance(pUnkOuter, IID_IUnknown,
                                         (void**)&pWrapper->m_pInner);
  if (FAILED(hr)) return hr;

  if ('\0' != m_szROTPrefix[0])
    hr = AddToROT(pWrapper);

  // Of course, we could put all of them into a list and let the
  // client enumerate them.
#if 1
  // This remembers the earliest one created.
  InterlockedCompareExchange((PVOID*)&m_pClass->m_pCurrentWrapper, pWrapper, NULL);
#else
  // This remembers the latest one created.
  InterlockedExchange((PLONG)&m_pClass->m_pCurrentWrapper, (LONG)pWrapper);
#endif

  return S_OK;
}

HRESULT CClassFactory::AddToROT(CMediaWrapper *pWrapper)
{
  IUnknown *pUnk = pWrapper;
  IRunningObjectTable *pROT;
  HRESULT hr = GetRunningObjectTable(0, &pROT);
  if (FAILED(hr)) return hr;

  char szBuf[256];
  wsprintf(szBuf, "%s %08p pid %08x", 
           m_szROTPrefix, (DWORD_PTR)pUnk, GetCurrentProcessId());

  OLECHAR wszBuf[256];
  MultiByteToWideChar(CP_ACP, 0, szBuf, -1, wszBuf, sizeof(wszBuf)/sizeof(OLECHAR));

  IMoniker *pMoniker;
  hr = CreateItemMoniker(L"!", wszBuf, &pMoniker);
  if (SUCCEEDED(hr)) {
    hr = pROT->Register(0, pUnk, pMoniker, &pWrapper->m_dwROT);
    pMoniker->Release();
  }
  pROT->Release();
  return hr;
}

/*** In-proc server entry functions ***/

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppv)
{
  CInterceptedClass *pClass = FindClass(rclsid);
  if (NULL == pClass) return CLASS_E_CLASSNOTAVAILABLE;
  CClassFactory *pCF = new CClassFactory(pClass);
  HRESULT hr = pCF->QueryInterface(riid, ppv);
  if (FAILED(hr)) delete pCF;
  return hr;
}

STDAPI DllRegisterServer()
{
  TCHAR szFile[MAX_PATH];
  DWORD dwFileLen = GetModuleFileName(GetModuleHandle("MEDIASPY"),
                                      szFile, sizeof(szFile));

  CInterceptedClass *pClass = g_classes;
  while (TRUE) {
    if (pClass->m_rclsid == CLSID_NULL) break;
    HKEY hKey = GetServerKey(pClass->m_rclsid);
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
    pClass++;
  }
  return S_OK;
}

STDAPI DllUnregisterServer()
{
  CInterceptedClass *pClass = g_classes;
  while (TRUE) {
    if (pClass->m_rclsid == CLSID_NULL) break;
    HKEY hKey = GetServerKey(pClass->m_rclsid);
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
    pClass++;
  }
  return S_OK;
}

/*** Hook entry function ***/

STDAPI MediaSpyGetCurrentObject(REFCLSID rclsid, REFIID riid, PVOID *ppv)
{
  CInterceptedClass *pClass = FindClass(rclsid);
  if (NULL == pClass) return CLASS_E_CLASSNOTAVAILABLE;
  CMediaWrapper *pCurrent = pClass->m_pCurrentWrapper;
  if (NULL == pCurrent) return S_FALSE;
  return pCurrent->QueryInterface(riid, ppv);
}

// This is here so that the hook itself does not need to link with OLE.
STDAPI MediaSpyConvertOle(LPOLESTR wszIn, LPSTR szOut, size_t nOut, BOOL bFree)
{
  WideCharToMultiByte(CP_ACP, 0, wszIn, -1, szOut, nOut, NULL, NULL);
  if (bFree) CoTaskMemFree(wszIn);
  return S_OK;
}

/** RealSpy -- RealOne Player plug-in for DVDSpy 
$Header$
**/

#include "stdafx.h"
#include "RealSpy.h"
#include "PlayerSpy.h"

static HWND g_hwndDVDSpy = NULL;

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

void DVDSpyEvent(LPCSTR szEvent, LPCSTR szPayload)
{
  char buf[1024], *pbuf;
  strncpy(buf, szEvent, sizeof(buf));
  pbuf = buf + strlen(buf) + 1;
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
  SendMessage(g_hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
}

 CRealSpy::CRealSpy() : m_refCount(0)
{
}

 CRealSpy::~CRealSpy()
{
}

STDMETHODIMP CRealSpy::QueryInterface(REFIID interfaceID, void** ppInterfaceObj)
{
  if (IsEqualIID(interfaceID, IID_IUnknown)) {
    AddRef();
    *ppInterfaceObj = (IUnknown*)(IRMAPlugin*)this;
    return PNR_OK;
  }
  else if (IsEqualIID(interfaceID, IID_IRMAPlugin)) {
    AddRef();
    *ppInterfaceObj = (IRMAPlugin*)this;
    return PNR_OK;
  }
  else if (IsEqualIID(interfaceID, IID_IRMAPlayerCreationSink)) {
    AddRef();
    *ppInterfaceObj = (IRMAPlayerCreationSink*)this;
    return PNR_OK;
  }
  *ppInterfaceObj = NULL;
  return PNR_NOINTERFACE;
}

STDMETHODIMP_(UINT32) CRealSpy::AddRef(void)
{
  return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(UINT32) CRealSpy::Release(void)
{
  if (InterlockedDecrement(&m_refCount) > 0) {
    return m_refCount;
  }

  m_refCount++;
  delete this;
  return 0;
}

STDMETHODIMP CRealSpy::GetPluginInfo(REF(BOOL) bLoadMultiple,
                                     REF(const char*) pDescription,
                                     REF(const char*) pCopyright,
                                     REF(const char*) pMoreInfoURL,
                                     REF(UINT32) versionNumber)
{
  bLoadMultiple = FALSE;
  pDescription  = "Girder DVDSpy plug-in";
  pCopyright = NULL;
  pMoreInfoURL = "http://www.girder.nl";
  versionNumber = 64;           // 1.64

  return PNR_OK;
}

STDMETHODIMP CRealSpy::InitPlugin(IUnknown* pRMACore)
{
  EnumWindows(FindMonitorWindow, (LPARAM)&g_hwndDVDSpy);
  // TODO: Consider checking more often.
  if (NULL == g_hwndDVDSpy)
    return PNR_OK;

  IRMAPlayerSinkControl* pControl = NULL;
  if (PNR_OK == pRMACore->QueryInterface(IID_IRMAPlayerSinkControl, (void**)&pControl)) {
    pControl->AddSink(this);
    pControl->Release();
  }
  return PNR_OK;
}

STDMETHODIMP CRealSpy::PlayerCreated(IRMAPlayer* pPlayer)
{
  (new CPlayerSpy())->Init(pPlayer);
  return PNR_OK;
}

STDMETHODIMP CRealSpy::PlayerClosed(IRMAPlayer* pPlayer)
{
  DVDSpyEvent("Real.Close");
  return PNR_OK;
}

STDAPI RMACreateInstance(IUnknown** ppPlugin)
{
  *ppPlugin = (IUnknown*)(IRMAPlugin*)new CRealSpy();
  if (NULL == *ppPlugin)
    return PNR_OUTOFMEMORY;

  (*ppPlugin)->AddRef();
  return PNR_OK;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH) {
#ifdef _DEBUG
    OutputDebugString("RealSpy loading\n");
#endif
  }
  else if (dwReason == DLL_PROCESS_DETACH) {
#ifdef _DEBUG
    OutputDebugString("RealSpy unloading\n");
#endif
  }
  return TRUE;
}

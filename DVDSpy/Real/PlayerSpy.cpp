// PlayerSpy.h -- implementation of advice sink object.
// $Header$

#include "stdafx.h"
#include "PlayerSpy.h"
#include "RealSpy.h"

 CPlayerSpy::CPlayerSpy() 
: m_refCount(0), m_pPlayer(NULL), m_pRegistry(NULL)
{
  m_ulPosition = m_ulLength = 0xFFFFFFFF;
  m_unBuffering = 0xFFFF;
  memset(m_nPropIDs, 0, sizeof(m_nPropIDs));
}

 CPlayerSpy::~CPlayerSpy()
{
  PN_RELEASE(m_pRegistry);
  PN_RELEASE(m_pPlayer);
}

const char *g_szPropNames[NPROPNAMES] = {
  "Title", "Author"
};

void CPlayerSpy::Init(IRMAPlayer* pPlayer)
{
  m_pPlayer = pPlayer;
  pPlayer->AddRef();
  pPlayer->AddAdviseSink(this);

  if (PNR_OK != pPlayer->QueryInterface(IID_IRMAPNRegistry, (void**)&m_pRegistry))
    return;

  char namebuf[1024], *pbuf;

  UINT32 nID = 0;
  IRMARegistryID* pRegistryID = NULL;
  if (PNR_OK == pPlayer->QueryInterface(IID_IRMARegistryID, (void**)&pRegistryID)) {
    pRegistryID->GetID(nID);
    pRegistryID->Release();
  }
  IRMABuffer* pBuffer = NULL;
  if (PNR_OK == m_pRegistry->GetPropName(nID, pBuffer)) {
    pBuffer->Release();
  }
  memcpy(namebuf, pBuffer->GetBuffer(), pBuffer->GetSize());
  pbuf = namebuf + pBuffer->GetSize() - 1;
  *pbuf++ = '.';

  for (size_t i = 0; i < NPROPNAMES; i++) {
    strcpy(pbuf, g_szPropNames[i]);
    m_nPropIDs[i] = m_pRegistry->GetId(namebuf);
  }
}

STDMETHODIMP CPlayerSpy::QueryInterface(REFIID interfaceID, void** ppInterfaceObj)
{
  if (IsEqualIID(interfaceID, IID_IUnknown)) {
    AddRef();
    *ppInterfaceObj = (IUnknown*)(IRMAClientAdviseSink*)this;
    return PNR_OK;
  }
  else if (IsEqualIID(interfaceID, IID_IRMAClientAdviseSink)) {
    AddRef();
    *ppInterfaceObj = (IRMAClientAdviseSink*)this;
    return PNR_OK;
  }
  *ppInterfaceObj = NULL;
  return PNR_NOINTERFACE;
}

STDMETHODIMP_(UINT32) CPlayerSpy::AddRef(void)
{
  return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(UINT32) CPlayerSpy::Release(void)
{
  if (InterlockedDecrement(&m_refCount) > 0) {
    return m_refCount;
  }

  m_refCount++;
  delete this;
  return 0;
}

STDMETHODIMP CPlayerSpy::OnPosLength(THIS_ UINT32 ulPosition, UINT32 ulLength)
{
  if (m_ulLength != ulLength) {
    m_ulLength = ulLength;
    TimeEvent("Real.Duration", ulLength);
  }
  // Only send events when pass a second threshold.
  if ((ulPosition / 1000) != (m_ulPosition / 1000))
    TimeEvent("Real.Elapsed", ulPosition);
  m_ulPosition = ulPosition;
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnPresentationOpened(THIS)
{
  {
    BOOL bFound = FALSE;
    UINT16 nSource = m_pPlayer->GetSourceCount();
    for (UINT16 i = 0; i < nSource; i++) {
      IUnknown *pUnk = NULL;
      if (PNR_OK == m_pPlayer->GetSource(i, pUnk)) {
        IRMAStreamSource* pSource = NULL;
        if (PNR_OK == pUnk->QueryInterface(IID_IRMAStreamSource, (void**)&pSource)) {
          DVDSpyEvent("Real.URL", pSource->GetURL());
          pSource->Release();
          bFound = TRUE;
        }
        pUnk->Release();
        if (bFound) break;
      }
    }
  }
  {
    for (size_t i = 0; i < NPROPNAMES; i++) {
      IRMABuffer* pValue = NULL;
      if (PNR_OK == m_pRegistry->GetStrById(m_nPropIDs[i], pValue)) {
        char szEvent[64];
        strcpy(szEvent, "Real.");
        strcat(szEvent, g_szPropNames[i]);
        const char *szPayload = (const char *)pValue->GetBuffer();
        DVDSpyEvent(szEvent, szPayload);
        pValue->Release();
      }
    }
  }

  m_unBuffering = 0xFFFF;
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnPresentationClosed(THIS)
{
  DVDSpyEvent("Real.URL");
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnStatisticsChanged(THIS)
{
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnPreSeek(THIS_ UINT32 ulOldTime, UINT32 ulNewTime)
{
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnPostSeek(THIS_ UINT32 ulOldTime, UINT32 ulNewTime)
{
  TimeEvent("Real.Elapsed", ulNewTime);
  m_ulPosition = ulNewTime;
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnStop(THIS)
{
  DVDSpyEvent("Real.State", "Stopped");
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnPause(THIS_ UINT32 ulTime)
{
  DVDSpyEvent("Real.State", "Paused");
  TimeEvent("Real.Elapsed", ulTime);
  m_ulPosition = ulTime;
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnBegin(THIS_ UINT32 ulTime)
{
  DVDSpyEvent("Real.State", "Playing");
  TimeEvent("Real.Elapsed", ulTime);
  m_ulPosition = ulTime;
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnBuffering(THIS_ UINT32 ulFlags, UINT16 unPercentComplete)
{
  if (m_unBuffering != unPercentComplete) {
    char buf[16];
    sprintf(buf, "%d", (int)unPercentComplete);
    DVDSpyEvent("Real.Buffering", buf);
    m_unBuffering = unPercentComplete;
  }
  return PNR_OK;
}

STDMETHODIMP CPlayerSpy::OnContacting(THIS_ const char* pHostName)
{
  // Actually, the payload is "Contacting host..."
  DVDSpyEvent("Real.Contacting", pHostName);
  return PNR_OK;
}

// Format numerators according to the length of denominators so that
// fixed length displays work nicely.

size_t SizeInt(int i)
{
  if (i < 0) 
    return (SizeInt(-i) + 1);

  int n = 1;
  unsigned int p = 1;
  while (TRUE) {
    p *= 10;
    if (p > (unsigned int)i) break;
    n++;
  }
  return n;
}

char *FormatInt(char *buf, int i, size_t size)
{
  if (i < 0) {
    *buf++ = '-';
    i = -i;
  }
  for (size_t n = 0; n < size; n++) {
    buf[size - n - 1] = '0' + (i % 10);
    i /= 10;
  }
  buf += size;
  *buf = '\0';
  return buf;
}

void DecodeTime(int time, int& thours, int& tmins, int& tsecs)
{
  // Time is in milliseconds; do desk calculator style rounding.
  tsecs = (time + 500) / 1000;
  tmins = tsecs / 60;
  tsecs -= tmins * 60;
  thours = tmins / 60;
  tmins -= thours * 60;
}

void FormatTime(char *buf, int time, int sizer)
{
  int thours, tmins, tsecs;
  int shours, smins, ssecs;
  DecodeTime(time, thours, tmins, tsecs);
  DecodeTime(sizer, shours, smins, ssecs);
  if (shours > 0) {
    buf = FormatInt(buf, thours, SizeInt(shours));
    *buf++ = ':';
    buf = FormatInt(buf, tmins, 2);
  }
  else {
    buf = FormatInt(buf, tmins, SizeInt(smins));
  }
  *buf++ = ':';
  buf = FormatInt(buf, tsecs, 2);
}

void CPlayerSpy::TimeEvent(LPCSTR szEvent, UINT32 ulTime)
{
  char buf[64];
  FormatTime(buf, (int)ulTime, (int)m_ulLength);
  DVDSpyEvent(szEvent, buf);
}

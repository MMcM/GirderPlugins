// DVDSpyMJEvent.h : Declaration of the CDVDSpyMJEvent

#ifndef __DVDSPYMJEVENT_H_
#define __DVDSPYMJEVENT_H_

enum CDVDSpyMJEventType {
  ET_STRING, ET_INT, ET_INT0, ET_SECONDS, 
  ET_PLAYBACK_STATE
};

class CDVDSpyMJEvent
{
public:
  CDVDSpyMJEvent(LPCSTR pszName, CDVDSpyMJEventType nType);
  CDVDSpyMJEvent(LPCSTR pszEvent, LPCSTR pszName, CDVDSpyMJEventType nType);
  ~CDVDSpyMJEvent();
  
  BOOL Check(LPDISPATCH pDisp);
  void Post(HWND hwndDVDSpy, LPCSTR szPrefix);

protected:
  LPCSTR m_pszName, m_pszEvent;
  CDVDSpyMJEventType m_nType;
  DISPID m_dispid;
  CComVariant m_value;
};

extern "C" void DVDSpyMJDoEvents(IMJAutomation *pMJ, HWND hwndDVDSpy);

#endif //__DVDSPYMJEVENT_H_

// DVDSpyMJEvent.cpp : Implementation of CDVDSpyMJEvent
// $Header$

#include "stdafx.h"
#include "DVDSpyMJEvent.h"

CDVDSpyMJEvent PlaylistEvents[] = {
  CDVDSpyMJEvent("Position", ET_INT0),
//CDVDSpyMJEvent("Shuffle", ET_BOOL),
//CDVDSpyMJEvent("Continuous", ET_BOOL),
  CDVDSpyMJEvent("Length", "GetNumberFiles", ET_INT),
};

CDVDSpyMJEvent FileEvents[] = {
  CDVDSpyMJEvent("Filename", ET_STRING),
  CDVDSpyMJEvent("Artist", ET_STRING),
  CDVDSpyMJEvent("Album", ET_STRING),
  CDVDSpyMJEvent("Name", ET_STRING),
//CDVDSpyMJEvent("FileSize", ET_INT),
//CDVDSpyMJEvent("FileSize", "GetFormattedFileSize", ET_STRING),
//CDVDSpyMJEvent("Duration", ET_SECONDS),
//CDVDSpyMJEvent("Duration", "GetFormattedDuration", ET_SECONDS),
//CDVDSpyMJEvent("Genre", ET_STRING),
//CDVDSpyMJEvent("Year", ET_INT),
//CDVDSpyMJEvent("Comment", ET_STRING),
//CDVDSpyMJEvent("BitRate", ET_INT),
//CDVDSpyMJEvent("TrackNumber", ET_INT),
//CDVDSpyMJEvent("FileDate", ET_INT),
//CDVDSpyMJEvent("FileDate", "GetFormattedFileDate", ET_STRING),
  CDVDSpyMJEvent("Custom1", ET_STRING),
  CDVDSpyMJEvent("Custom2", ET_STRING),
  CDVDSpyMJEvent("Custom3", ET_STRING),
//CDVDSpyMJEvent("PlayCounter", ET_INT),
//CDVDSpyMJEvent("LastPlayed", ET_INT),
//CDVDSpyMJEvent("LastPlayed", "GetFormattedLastPlayed", ET_INT),
//CDVDSpyMJEvent("Rating", ET_INT),
  CDVDSpyMJEvent("FileType", ET_STRING),
//CDVDSpyMJEvent("Lyrics", ET_STRING),
//CDVDSpyMJEvent("Notes", ET_STRING),
};

CDVDSpyMJEvent PlaybackEvents[] = {
  CDVDSpyMJEvent("State", ET_PLAYBACK_STATE),
//CDVDSpyMJEvent("BitRate", ET_INT),
  CDVDSpyMJEvent("Elapsed", "Position", ET_SECONDS),
  CDVDSpyMJEvent("Duration", ET_SECONDS),
//CDVDSpyMJEvent("Channels", ET_INT),
//CDVDSpyMJEvent("SampleRate", ET_INT),
//CDVDSpyMJEvent("BitsPerSample", ET_INT),
};

#define countof(x) (sizeof(x)/sizeof(x[0]))

CDVDSpyMJEvent::CDVDSpyMJEvent(LPCSTR pszName, CDVDSpyMJEventType nType)
  : m_pszName(pszName), m_pszEvent(pszName), m_nType(nType)
{
  m_dispid = DISPID_UNKNOWN;
}

CDVDSpyMJEvent::CDVDSpyMJEvent(LPCSTR pszEvent, LPCSTR pszName, CDVDSpyMJEventType nType)
  : m_pszName(pszName), m_pszEvent(pszEvent), m_nType(nType)
{
  m_dispid = DISPID_UNKNOWN;
}

CDVDSpyMJEvent::~CDVDSpyMJEvent()
{
}

BOOL CDVDSpyMJEvent::Check(LPDISPATCH pDisp)
{
  USES_CONVERSION;
  HRESULT hr;

  if (DISPID_UNKNOWN == m_dispid) {
    LPOLESTR wszName = T2OLE(m_pszName);
    hr = pDisp->GetIDsOfNames(IID_NULL, &wszName, 1, LOCALE_SYSTEM_DEFAULT, &m_dispid);
    if (FAILED(hr)) return FALSE;
  }

  CComVariant result;

  DISPPARAMS params = {NULL, NULL, 0, 0};
  hr = pDisp->Invoke(m_dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                     INVOKE_PROPERTYGET, &params, &result, 
                     NULL, NULL);
  if (FAILED(hr)) return FALSE;

  if (result == m_value)
    return FALSE;
  
  m_value = result;
  return TRUE;
}

void CDVDSpyMJEvent::Post(HWND hwndDVDSpy, LPCSTR szPrefix)
{
  char szEvent[1024], *pend = szEvent;
  
  strcpy(pend, "MediaJukebox.");
  pend += strlen(pend);
  strcpy(pend, szPrefix);
  pend += strlen(pend);
  *pend++ = '.';
  strcpy(pend, m_pszEvent);
  pend += strlen(pend);
  *pend++ = '\0';
  *pend++ = 1;

  CComVariant copy;
  switch (m_nType) {
  case ET_STRING:
    {
      if (FAILED(copy.ChangeType(VT_BSTR, &m_value)))
        return;
      pend += WideCharToMultiByte(CP_ACP, 0, copy.bstrVal, SysStringLen(copy.bstrVal),
                                  pend, sizeof(szEvent) - (pend - szEvent), 
                                  NULL, NULL);
    }
    break;

  case ET_INT:
    {
      if (FAILED(copy.ChangeType(VT_I4, &m_value)))
        return;
      sprintf(pend, "%ld", copy.lVal);
      pend += strlen(pend);
    }
    break;

  case ET_INT0:
    {
      if (FAILED(copy.ChangeType(VT_I4, &m_value)))
        return;
      sprintf(pend, "%ld", copy.lVal + 1);
      pend += strlen(pend);
    }
    break;

  case ET_SECONDS:
    {
      if (FAILED(copy.ChangeType(VT_I4, &m_value)))
        return;
      if (copy.lVal < 0) 
        break;
    
      int nSecs = copy.lVal;
      int nMins = nSecs / 60;
      nSecs -= nMins * 60;
      int nHrs = nMins / 60;
      nMins -= nHrs * 60;
      if (nHrs > 0)
        sprintf(pend, "%d:%02d:%02d", nHrs, nMins, nSecs);
      else
        sprintf(pend, "%d:%02d", nMins, nSecs);
      pend += strlen(pend);
    }
    break;

  case ET_PLAYBACK_STATE:
    {
      if (FAILED(copy.ChangeType(VT_I2, &m_value)))
        return;
      LPCSTR pszState;
      switch (copy.iVal) {
      case PLAYSTATE_STOPPED:
        pszState = "Stopped";
        break;
      case PLAYSTATE_PAUSED:
        pszState = "Paused";
        break;
      case PLAYSTATE_PLAYING:
        pszState = "Playing";
        break;
      case PLAYSTATE_WAITING:
        pszState = "Waiting";
        break;
      default:
        return;
      }
      strcpy(pend, pszState);
      pend += strlen(pend);
    }
    break;
  }
  *pend++ = '\0';

  COPYDATASTRUCT cd;
  cd.dwData = 0;
  cd.lpData = szEvent;
  cd.cbData = pend - szEvent;
  SendMessage(hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
}

void DoEvents(LPDISPATCH pDisp, HWND hwndDVDSpy, LPCSTR szPrefix,
              CDVDSpyMJEvent *pEvents, size_t nEvents)
{
  for (size_t i = 0; i < nEvents; i++) {
    if (pEvents[i].Check(pDisp))
      pEvents[i].Post(hwndDVDSpy, szPrefix);
  }
}

void DVDSpyMJDoEvents(IMJAutomation *pMJ, HWND hwndDVDSpy)
{
  IMJCurPlaylistAutomationPtr pPlaylist = pMJ->GetCurPlaylist();
  long nPosition = pPlaylist->Position;
  if (nPosition >= 0) {
    DoEvents(pPlaylist, hwndDVDSpy, 
             "Playlist", PlaylistEvents, countof(PlaylistEvents));
    
    IMJFileAutomationPtr pFile = pPlaylist->GetFile(nPosition);
    DoEvents(pFile, hwndDVDSpy, 
             "File", FileEvents, countof(FileEvents));
  }

  IMJPlaybackAutomationPtr pPlayback = pMJ->GetPlayback();
  DoEvents(pPlayback, hwndDVDSpy, 
           "Playback", PlaybackEvents, countof(PlaybackEvents));
}

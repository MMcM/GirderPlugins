
#include "stdafx.h"
#include "DisplaySpyHook.h"

#define ASSERT(x) _ASSERTE(x)
#define countof(x) sizeof(x)/sizeof(x[0])

struct MatchEntry
{
  LPCSTR szName;
  UINT nCode;
  LPCSTR szVal;
  DWORD dwVal;
};

#define ENTRY0(c) { NULL, c },
#define ENTRY(c,s,l) { NULL, c, s, l },
#define ENTRY_STR(c,s) { NULL, c, s },
#define ENTRY_NUM(c,l) { NULL, c, NULL, l },

#define NENTRY0(n,c) { #n, c },
#define NENTRY(n,c,s,l) { #n, c, s, l },
#define NENTRY_STR(n,c,s) { #n, c, s },
#define NENTRY_NUM(n,c,l) { #n, c, NULL, l },

const UINT ENTRY_MODULE = 1;
const UINT ENTRY_BEGIN = 2;
const UINT ENTRY_EXTRACT = 3;
const UINT ENTRY_END = 4;

#define BEGIN_MODULE(x) BEGIN_NMODULE(x,x)
#define BEGIN_NMODULE(n,x) NENTRY_STR(n,ENTRY_MODULE,#x)
#define BEGIN_NMODULE_FLAGS(n,x,f) NENTRY(n,ENTRY_MODULE,#x,f)
#define BEGIN_MATCH() ENTRY0(ENTRY_BEGIN)
#define BEGIN_NMATCH(n) NENTRY0(n,ENTRY_BEGIN)
#define BEGIN_EXTRACT() ENTRY0(ENTRY_EXTRACT)
#define END_MATCH() ENTRY0(ENTRY_END)

const UINT ENTRY_HWND_PARENT =      0x00010000;
const UINT ENTRY_HWND_GRANDPARENT = 0x00020000;
const UINT ENTRY_HWND_NOTIFY =      0x00100000;
const UINT ENTRY_NOT =              0x10000000;

#define BASE_CODE(n) (n&0xFFFF)

const UINT MATCH_MESSAGE = 1001;
const UINT MATCH_WPARAM = 1002;
const UINT MATCH_LPARAM = 1003;
const UINT MATCH_LPARAM_STR = 1004;
const UINT MATCH_GETTEXT = 1005;
const UINT MATCH_CLASS = 1006;
const UINT MATCH_CLASS_PREFIX = 1007;
const UINT MATCH_CONTROLID = 1008;
const UINT MATCH_NOTIFY_CODE = 1010;
const UINT MATCH_NOTIFY_FROM = 1011;
const UINT MATCH_PATCH = 1020;
const UINT MATCH_MEDIA_SPY = 1030;

const UINT EXTRACT_CONSTANT = 2001;
const UINT EXTRACT_GETTEXT = 2002;
const UINT EXTRACT_LPARAM_STR = 2003;
const UINT EXTRACT_LPARAM = 2004;
const UINT EXTRACT_WPARAM = 2005;
const UINT EXTRACT_LPARAM_POINT = 2006;
const UINT EXTRACT_WPARAM_POINT = 2007;
const UINT EXTRACT_CLASS = 2008;
const UINT EXTRACT_HWND = 2009;
const UINT EXTRACT_SB_GETTEXT = 2010;
const UINT EXTRACT_MEDIA_SPY = 2030;

const HWND HWND_PATCH = (HWND)0xDEADFACE;
const UINT PATCH_TEXTOUT = 0x80000001;

const UINT MS_FILTER_GRAPH = 1;
const UINT MS_DVD_NAVIGATOR = 2;
const UINT MS_FG_DURATION = 3;
const UINT MS_FG_POSITION = 4;
const UINT MS_FG_FILENAME = 5;
const UINT MS_DVD_DOMAIN = 6;
const UINT MS_DVD_TITLE = 7;
const UINT MS_DVD_CHAPTER = 8;
const UINT MS_DVD_TOTAL = 9;
const UINT MS_DVD_TIME = 10;

const DWORD MODULE_NO_WIN16_CWP = 1;

/*** Patterns that drive the hook to extract display information. ***/
static MatchEntry g_matches[] = {

  BEGIN_MODULE(WinDVD)

    // This is an update to the tracker bar in the status bar in the
    // display window.  When it happens, the status bar fields have been
    // changed as well (via MFC).
    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, TBM_SETPOS)
      ENTRY_NUM(MATCH_CONTROLID, 1)
      ENTRY_STR(MATCH_CLASS, TRACKBAR_CLASS)
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, STATUSCLASSNAME)
      ENTRY_STR(ENTRY_HWND_GRANDPARENT|MATCH_CLASS, "WinDVDClass")
     BEGIN_EXTRACT()
      NENTRY(Chapter,ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, "Chapter: ", 3)
      NENTRY(Elapsed,ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, "Time: ", 4)
      NENTRY_NUM(Video,ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 5)
      NENTRY_NUM(Field6,ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 6)
      NENTRY_NUM(Audio,ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 7)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "WinDVDClass")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(PowerDVD,POWERDVD)

    // This only has the time.  The title# and chapter# are displayed as
    // graphics in the "remote" window.
    BEGIN_NMATCH(Elapsed)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_STR(MATCH_CLASS, "Static")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "#32770")
      ENTRY_STR(ENTRY_HWND_GRANDPARENT|MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    // This only happens when playing a .VOB file: the window title is
    // changed to the file in question.  Better than nothing.
    BEGIN_NMATCH(Title)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_STR(MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(accessDTV)

    BEGIN_NMATCH(ChanText)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(51,71))
      ENTRY_STR(ENTRY_NOT|MATCH_LPARAM_STR, "XXXXXXXXXXXXXX")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(ChanNoText)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(51,71))
      ENTRY_STR(MATCH_LPARAM_STR, "XXXXXXXXXXXXXX")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

    BEGIN_NMATCH(Channel)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(50,43))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

#if 0
    // This worked in the first beta version.  It has understandably
    // since been replaced by something more graphical.
    BEGIN_NMATCH(Elapsed)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_NUM(MATCH_CONTROLID, 1114)
      ENTRY_STR(MATCH_CLASS, "Static")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "#32770")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_GETTEXT, "accessDTV")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Duration)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_NUM(MATCH_CONTROLID, 1115)
      ENTRY_STR(MATCH_CLASS, "Static")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "#32770")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_GETTEXT, "accessDTV")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()
#endif
  
    BEGIN_NMATCH(Elapsed)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(25,359))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_GETTEXT, "accessDTV")
      ENTRY_STR(MATCH_CLASS_PREFIX, "Afx:")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(ZoomPlayer,zplayer)

    BEGIN_NMATCH(DVD)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Media)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(OSD)
      ENTRY_NUM(MATCH_MESSAGE, WM_PAINT)
      ENTRY_STR(MATCH_CLASS, "TPanel")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "TOSDForm")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_GETTEXT)
    END_MATCH()

    BEGIN_NMATCH(Size)
      ENTRY_NUM(MATCH_MESSAGE, WM_SIZE)
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_POINT)
    END_MATCH()

    BEGIN_NMATCH(Init)
      ENTRY_NUM(MATCH_MESSAGE, WM_CREATE)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_HWND)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE_FLAGS(TheaterTek,TheaterTek DVD,MODULE_NO_WIN16_CWP)

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_GETTEXT, "Overlay")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(ATI,ATIMMC)

    BEGIN_NMATCH(DVD)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(MMC)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(ShowShifter)
  // Getting information off the screen is tricky because SSF has its
  // own window system of sorts.

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "ShowShifterWndClass")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(BSPlayer,bplay)
  // There are also some text controls, which display things like
  // Pause, that might be worth getting.  Their position comes from
  // the skin.ini file in the skin directory / archive.  They appear
  // to be drawn with DrawText to a zero-origin DC, such as a bitmap
  // that is then blted to the screen.

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "BSPlayer")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

#if 0
  // Does anyone still use this?
  BEGIN_NMODULE(SASAMI2k,sasami2000)

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()
#endif

#if 0
  // The problem here is that the the decoding is all internal and the
  // drawing via DirectDraw (not DirectShow).  Labels are drawn
  // manually because of skinning.
  BEGIN_NMODULE(DivXPlayer,DivX Player*)

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "QWidget")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()
#endif

  BEGIN_NMODULE(PowerDivX,PowerDivX*)

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
#if 0
      // Too many of these.
      ENTRY_STR(MATCH_CLASS, "ThunderRT6FormDC")
#else
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
#endif
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

#if 0
  // Uncomment this and edit to begin adding a new application.
  BEGIN_MODULE(XXX)

    // DVDNavigator.
    BEGIN_NMATCH(DVD)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    // Filter graph.
    BEGIN_NMATCH(Media)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    // Set window text.
    BEGIN_NMATCH(SetText)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
     BEGIN_EXTRACT()
      NENTRY0(Class,EXTRACT_CLASS)
      NENTRY0(Text,EXTRACT_LPARAM_STR)
    END_MATCH()

    // Calls to TextOut at some point.
    BEGIN_NMATCH(TextOut)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
     BEGIN_EXTRACT()
      NENTRY0(Name,EXTRACT_GETTEXT)
      NENTRY0(Class,EXTRACT_CLASS)
      NENTRY0(Point,EXTRACT_WPARAM_POINT)
      NENTRY0(String,EXTRACT_LPARAM_STR)
    END_MATCH()

    // Close a window.
    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
     BEGIN_EXTRACT()
      NENTRY0(Name,EXTRACT_GETTEXT)
      NENTRY0(Class,EXTRACT_CLASS)
    END_MATCH()

#endif

};

// We could use WM_COPYDATA and put the data in the actual message,
// but then we'd hang up the notifying process until that message was
// processed.  So, we use a circular queue instead.
struct DisplayBuf
{
  size_t nMatch;                // Match index
  size_t nIndex;                // Extract index
  char szValue[128];            // Extracted string
};

const size_t MAX_BUFS = 16;

struct MatchIndexEntry
{
  LPCSTR szModule;              // Name from module.
  LPCSTR szMatch;               // Name from match.
  size_t nMatches;              // Number of match predicates.
  MatchEntry *pMatches;         // Pointer to first match predicate.
  size_t nExtracts;             // Number of match extracts.
  MatchEntry *pExtracts;        // Pointer to first match extract.
};

/*** Shared global data ***/
#pragma data_seg(".SHARDATA")
static HHOOK gs_hWndHook = NULL, gs_hMsgHook = NULL;
static DWORD gs_dwThreadId = NULL;

static DisplayBuf gs_pDisplayBufs[MAX_BUFS] = { { 0 } };
static size_t gs_nRead = 0;
static size_t gs_nWrite = 0;
static BOOL gs_bEmpty = TRUE;
#pragma data_seg()

/*** Global data (per process) ***/
HINSTANCE g_hInst = NULL;
HANDLE g_hMutex = NULL;
size_t g_nMatchOffset = 0;
size_t g_nMatches = 0;
MatchIndexEntry *g_pMatches = NULL;
DWORD g_dwModuleFlags = 0;
BOOL g_bNoCallWndProc = FALSE;

BOOL MatchMediaSpy(UINT nClass);
void ExtractMediaSpy(UINT nField, LPSTR szBuf, size_t nSize);

BOOL DoMatch2(const MatchEntry *pEntry, 
              HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (pEntry->nCode & ENTRY_HWND_NOTIFY) {
    if (WM_NOTIFY != nMsg)
      return FALSE;
    LPNMHDR pnmh = (LPNMHDR)lParam;
    hWnd = pnmh->hwndFrom;
  }
  if (pEntry->nCode & (ENTRY_HWND_PARENT | ENTRY_HWND_GRANDPARENT)) {
    hWnd = GetParent(hWnd);
    if (NULL == hWnd)
      return FALSE;
    if (pEntry->nCode & ENTRY_HWND_GRANDPARENT) {
      hWnd = GetParent(hWnd);
      if (NULL == hWnd)
        return FALSE;
    }
  }
  switch (BASE_CODE(pEntry->nCode)) {
  case MATCH_MESSAGE:
    return (nMsg == (UINT)pEntry->dwVal);
  case MATCH_WPARAM:
    return (wParam == (WPARAM)pEntry->dwVal);
  case MATCH_LPARAM:
    return (lParam == (LPARAM)pEntry->dwVal);
  case MATCH_LPARAM_STR:
    return !strcmp((LPCSTR)lParam, pEntry->szVal);
  case MATCH_GETTEXT:
    {
      char szBuf[256];
      GetWindowText(hWnd, szBuf, sizeof(szBuf));
      return !strcmp(szBuf, pEntry->szVal);
    }
  case MATCH_CLASS:
    {
      char szCName[256];
      GetClassName(hWnd, szCName, sizeof(szCName));
      return !strcmp(szCName, pEntry->szVal);
    }
  case MATCH_CLASS_PREFIX:
    {
      char szCName[256];
      GetClassName(hWnd, szCName, sizeof(szCName));
      return !strncmp(szCName, pEntry->szVal, strlen(pEntry->szVal));
    }
  case MATCH_CONTROLID:
    {
      LONG lID = GetWindowLong(hWnd, GWL_ID);
      return (lID == (LONG)pEntry->dwVal);
    }
  case MATCH_NOTIFY_CODE:
    {
      if (WM_NOTIFY != nMsg)
        return FALSE;
      LPNMHDR pnmh = (LPNMHDR)lParam;
      return (pnmh->code == pEntry->dwVal);
    }
  case MATCH_NOTIFY_FROM:
    {
      if (WM_NOTIFY != nMsg)
        return FALSE;
      LPNMHDR pnmh = (LPNMHDR)lParam;
      return (pnmh->idFrom == pEntry->dwVal);
    }
  case MATCH_PATCH:
    {
      if (HWND_PATCH != hWnd)
        return FALSE;
      return (nMsg == (UINT)pEntry->dwVal);
    }
  case MATCH_MEDIA_SPY:
    return MatchMediaSpy((UINT)pEntry->dwVal);
  default:
    ASSERT(FALSE);
    return FALSE;
  }
}

BOOL DoMatch1(const MatchEntry *pEntry, 
              HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (pEntry->nCode & ENTRY_NOT)
    return !DoMatch2(pEntry, hWnd, nMsg, wParam, lParam);
  else
    return DoMatch2(pEntry, hWnd, nMsg, wParam, lParam);
}

BOOL DoMatch(const MatchIndexEntry *pEntry,
             HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  for (size_t i = 0; i < pEntry->nMatches; i++)
    if (!DoMatch1(pEntry->pMatches+i, hWnd, nMsg, wParam, lParam))
      return FALSE;
  return TRUE;
}

void DoExtract1(const MatchEntry *pEntry, LPSTR szBuf, size_t nSize,
                HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (pEntry->nCode & ENTRY_HWND_NOTIFY) {
    if (WM_NOTIFY != nMsg) {
      *szBuf = '\0';
      return;
    }
    LPNMHDR pnmh = (LPNMHDR)lParam;
    hWnd = pnmh->hwndFrom;
  }
  if (pEntry->nCode & (ENTRY_HWND_PARENT | ENTRY_HWND_GRANDPARENT)) {
    hWnd = GetParent(hWnd);
    if (NULL == hWnd) {
      *szBuf = '\0';
      return;
    }
    if (pEntry->nCode & ENTRY_HWND_GRANDPARENT) {
      hWnd = GetParent(hWnd);
      if (NULL == hWnd) {
        *szBuf = '\0';
        return;
      }
    }
  }
  switch (BASE_CODE(pEntry->nCode)) {
  case EXTRACT_CONSTANT:
    strncpy(szBuf, pEntry->szVal, nSize);
    break;
  case EXTRACT_GETTEXT:
    GetWindowText(hWnd, szBuf, nSize);
    break;
  case EXTRACT_LPARAM_STR:
    strncpy(szBuf, (LPCSTR)lParam, nSize);
    break;
  case EXTRACT_LPARAM:
    sprintf(szBuf, "%d", lParam);
    break;
  case EXTRACT_WPARAM:
    sprintf(szBuf, "%d", wParam);
    break;
  case EXTRACT_LPARAM_POINT:
    sprintf(szBuf, "%d,%d", LOWORD(lParam), HIWORD(lParam));
    break;
  case EXTRACT_WPARAM_POINT:
    sprintf(szBuf, "%d,%d", LOWORD(wParam), HIWORD(wParam));
    break;
  case EXTRACT_CLASS:
    {
      char szCName[256];
      GetClassName(hWnd, szCName, sizeof(szCName));
      strncpy(szBuf, szCName, nSize);
    }
    break;
  case EXTRACT_HWND:
    sprintf(szBuf, "%X", hWnd);
    break;
  case EXTRACT_SB_GETTEXT:
    SendMessage(hWnd, SB_GETTEXT, pEntry->dwVal, (LPARAM)szBuf);
    if (NULL != pEntry->szVal) {
      size_t prefixLen = strlen(pEntry->szVal);
      if (!strncmp(szBuf, pEntry->szVal, prefixLen))
        memmove(szBuf, szBuf + prefixLen, strlen(szBuf) - prefixLen + 1);
    }
    break;
  case EXTRACT_MEDIA_SPY:
    ExtractMediaSpy((UINT)pEntry->dwVal, szBuf, nSize);
    break;
  default:
    ASSERT(FALSE);
    szBuf[0] = '\0';
  }
}

BOOL DoExtract(size_t nMatch, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  BOOL notify = FALSE;
  const MatchIndexEntry *pEntry = g_pMatches+nMatch;
  DisplayBuf dbuf;
  dbuf.nMatch = nMatch + g_nMatchOffset;
  for (dbuf.nIndex = 0; dbuf.nIndex < pEntry->nExtracts; dbuf.nIndex++) {
    // Get new value.
    DoExtract1(pEntry->pExtracts+dbuf.nIndex, dbuf.szValue, sizeof(dbuf.szValue),
               hWnd, nMsg, wParam, lParam);
    // Search back in the queue for an entry for the same match and index.
    size_t nOld = gs_nWrite;
    BOOL bFound = FALSE, bPending = !gs_bEmpty;
    do {
      if (nOld == 0) nOld = MAX_BUFS;
      nOld--;
      if ((gs_pDisplayBufs[nOld].nMatch == dbuf.nMatch) && 
          (gs_pDisplayBufs[nOld].nIndex == dbuf.nIndex)) {
        bFound = TRUE;
        break;
      }
      if (nOld == gs_nRead)
        bPending = FALSE;       // Going into already read.
    } while (nOld != gs_nWrite);
    if (bFound) {
      if (!strcmp(gs_pDisplayBufs[nOld].szValue, dbuf.szValue)) {
        // Value has not changed since last entered (whether read or pending).
        continue;
      }
      if (bPending) {
        // Remove pending entry.
        while (TRUE) {
          size_t nNext = nOld + 1;
          if (nNext == MAX_BUFS) nNext = 0;
          if (nNext == gs_nWrite) break;
          gs_pDisplayBufs[nOld] = gs_pDisplayBufs[nNext];
          nOld = nNext;
        }
        gs_nWrite = nOld;
      }
    }
    BOOL full = (!gs_bEmpty && (gs_nRead == gs_nWrite));
    gs_pDisplayBufs[gs_nWrite++] = dbuf;
    if (gs_nWrite == MAX_BUFS) gs_nWrite = 0;
    // If the queue was full, we overwrote the oldest entry, so
    // advance the read pointer as well, to keep the remaining ones in
    // order.
    if (full)
      gs_nRead = gs_nWrite;
    else
      notify = TRUE;
    gs_bEmpty = FALSE;
  }
  return notify;
}

BOOL GetMutex(DWORD timeout)
{
  if (NULL == g_hMutex) {
    g_hMutex = CreateMutex(NULL, TRUE, "DisplaySpyHookMutex");
    return TRUE;
  }
  return (WAIT_TIMEOUT != WaitForSingleObject(g_hMutex, timeout));
}

// C++ macros are not powerful enough to make the structure we want at
// compile time, so we make something flatter and index it when first
// needed.  To keep overhead to a minimum, we disable pattern matching
// entirely except in applications that are known to send the
// interesting messages.
void IndexMatches(BOOL bAll)
{
  char szModBuf[MAX_PATH];
  LPCSTR szModule;
  DWORD dwFlags = 0;
  if (!bAll) {
    GetModuleFileName(NULL, szModBuf, sizeof(szModBuf));
    LPSTR pend = strrchr(szModBuf, '.');
    if (NULL != pend)
      *pend = '\0';             // Remove extension.
    szModule = strrchr(szModBuf, '\\');
    if (NULL != szModule)
      szModule++;               // Remove directory.
    else
      szModule = szModBuf;
  }
  size_t nOffset = 1, nMatches = 0, nBegin = 0;
  for (size_t i = 0; i < countof(g_matches); i++) {
    if (ENTRY_MODULE == g_matches[i].nCode) {
      if (!bAll) {
        if (nBegin)
          break;                // End of found module.
        LPCSTR szKey = g_matches[i].szVal;
        BOOL bMatch = FALSE;
        size_t nKey = strlen(szKey);
        if ('*' == szKey[nKey-1]) // Allow trailing wildcard for prefix match.
          bMatch = !_strnicmp(szKey, szModule, nKey-1);
        else
          bMatch = !_stricmp(szKey, szModule);
        if (bMatch) {
          szModule = g_matches[i].szName; // Our name for it.
          dwFlags = g_matches[i].dwVal;
          nBegin = i + 1;       // First module entry.
        }
      }
    }
    else if (ENTRY_BEGIN == g_matches[i].nCode) {
      if (bAll || nBegin)
        nMatches++;             // Count matches in module (or all).
      else
        nOffset++;              // Count matches before module.
    }
  }

  MatchIndexEntry *pMatches = NULL;
  if (nMatches > 0)
    pMatches = new MatchIndexEntry[nMatches];

  i = nBegin;
  size_t j = 0;
  while ((i < countof(g_matches)) && (j < nMatches)) {
    if (ENTRY_MODULE == g_matches[i].nCode) {
      if (!bAll) 
        break;
      szModule = g_matches[i].szName;
      i++;
      continue;
    }
    ASSERT(ENTRY_BEGIN == g_matches[i].nCode);
    pMatches[j].szModule = szModule;
    pMatches[j].szMatch = g_matches[i].szName;
    i++;
    pMatches[j].nMatches = 0;
    pMatches[j].pMatches = g_matches+i;
    while ((i < countof(g_matches)) && (ENTRY_EXTRACT != g_matches[i++].nCode)) {
      ASSERT((1000 < BASE_CODE(g_matches[i-1].nCode)) && 
             (2000 > BASE_CODE(g_matches[i-1].nCode)));
      pMatches[j].nMatches++;
    }
    pMatches[j].nExtracts = 0;
    pMatches[j].pExtracts = g_matches+i;
    while ((i < countof(g_matches)) && (ENTRY_END != g_matches[i++].nCode)) {
      ASSERT((2000 < BASE_CODE(g_matches[i-1].nCode)) && 
             (3000 > BASE_CODE(g_matches[i-1].nCode)));
      pMatches[j].nExtracts++;
    }
    j++;
  }
  
  g_pMatches = pMatches;
  g_nMatchOffset = nOffset;
  g_nMatches = nMatches;

  g_dwModuleFlags = dwFlags;
  if (dwFlags & MODULE_NO_WIN16_CWP) {
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    if (GetVersionEx(&osinfo) &&
        (VER_PLATFORM_WIN32_NT != osinfo.dwPlatformId))
      // Don't try to decode CallWndProc messages from this application under Win9x/Me.
      // Just matching causes some kind of damage which makes Windows unable to reboot.
      // I think it may have something to do with the stack depth and thunking.
      g_bNoCallWndProc = TRUE;
  }
}

// Process a single message
void DoMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (NULL == g_pMatches)
    return;
  BOOL bNotify = FALSE;
  for (size_t i = 0; i < g_nMatches; i++) {
    if (DoMatch(g_pMatches+i, hWnd, nMsg, wParam, lParam)) {
      if (GetMutex(1000)) {     // Don't hang up long here if another process has it.
        if (DoExtract(i, hWnd, nMsg, wParam, lParam))
          bNotify = TRUE;
        ReleaseMutex(g_hMutex);
      }
      // This statement makes only the first match apply.  That seems best for now.
      break;
    }
  }
  if (bNotify)
    PostThreadMessage(gs_dwThreadId, WM_NEWDISPLAY, 0, 0);
}

// Windows message hook callback
LRESULT CALLBACK CallWndHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if ((nCode >= 0) && !g_bNoCallWndProc) {
    PCWPSTRUCT pParams = (PCWPSTRUCT)lParam;
    DoMessage(pParams->hwnd, pParams->message, pParams->wParam, pParams->lParam);
  }
  return CallNextHookEx(gs_hWndHook, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode >= 0) {
    PMSG pMsg = (PMSG)lParam;
    DoMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
  }
  return CallNextHookEx(gs_hMsgHook, nCode, wParam, lParam);
}

/*** Patching of Windows functions via Import Address Table ***/

PROC *TextOutIAT = NULL;
BOOL (WINAPI *OrigTextOut)(HDC, int, int, LPCSTR, int) = NULL;
BOOL WINAPI PatchTextOut(HDC hdc, int x, int y, LPSTR str, int nb)
{
  BOOL rc = (*OrigTextOut)(hdc, x, y, str, nb);
  char ch = str[nb];
  if ('\0' != ch)
    str[nb] = '\0';
  DoMessage(HWND_PATCH, PATCH_TEXTOUT, (WPARAM)MAKELONG(x,y), (LPARAM)str);
  if ('\0' != ch)
    str[nb] = ch;
  return rc;
}

void PatchFunction(LPCSTR szModule, LPCSTR szFunction, 
                   PROC pnfunc, PROC **ppiat, PROC *porig)
{
  HMODULE hfmod = GetModuleHandle(szModule);
  if (NULL == hfmod) return;
  PROC pfunc = GetProcAddress(hfmod, szFunction);
  if (NULL == pfunc) return;

  HMODULE hmod = GetModuleHandle(NULL);
  PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)hmod;
  if (pdos->e_magic != IMAGE_DOS_SIGNATURE) return;
  PIMAGE_NT_HEADERS pnt = (PIMAGE_NT_HEADERS)((DWORD)hmod + pdos->e_lfanew);
  if (pnt->Signature != IMAGE_NT_SIGNATURE) return;
  PIMAGE_DATA_DIRECTORY pimp = pnt->OptionalHeader.DataDirectory 
    + IMAGE_DIRECTORY_ENTRY_IMPORT;
  PIMAGE_DATA_DIRECTORY piat= pnt->OptionalHeader.DataDirectory 
    + IMAGE_DIRECTORY_ENTRY_IAT;
  
  PIMAGE_IMPORT_DESCRIPTOR pid = (PIMAGE_IMPORT_DESCRIPTOR)
    ((DWORD)hmod + pimp->VirtualAddress);
  while (TRUE) {
    if (NULL == pid->Characteristics) return;
    LPCSTR pmod = (LPCSTR)((DWORD)hmod + pid->Name);
    if (!_stricmp(pmod, szModule)) break;
    pid++;
  }
  PIMAGE_THUNK_DATA pdata = (PIMAGE_THUNK_DATA)((DWORD)hmod + pid->OriginalFirstThunk);
  PROC *paddr = (PROC *)((DWORD)hmod + pid->FirstThunk);
  while (TRUE) {
    if (NULL == pdata->u1.Function) return;
    if ((pdata->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0) {
      PIMAGE_IMPORT_BY_NAME pnam = (PIMAGE_IMPORT_BY_NAME)
        ((DWORD)hmod + (DWORD)pdata->u1.AddressOfData);
      if (!strcmp((LPCSTR)pnam->Name, szFunction)) break;
      pdata++;
      paddr++;
    }
  }
  if (pfunc != *paddr) return;
  *ppiat = paddr;
  *porig = *paddr;
  DWORD dwOldProt;
  if (VirtualProtect(paddr, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
    *paddr = pnfunc;
    VirtualProtect(paddr, sizeof(PROC), dwOldProt, &dwOldProt);
  }
}

void InstallPatches ()
{
  if (NULL == g_pMatches)
    return;
  for (size_t i = 0; i < g_nMatches; i++) {
    if (MATCH_PATCH == g_pMatches[i].pMatches[0].nCode) {
      switch (g_pMatches[i].pMatches[0].dwVal) {
      case PATCH_TEXTOUT:
        PatchFunction("GDI32.DLL", "TextOutA", 
                      (PROC)PatchTextOut, &TextOutIAT, (PROC*)&OrigTextOut);
        break;
      }
    }
  }
}

// Remove any patches.
void RemovePatches()
{
  if (NULL != TextOutIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(TextOutIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *TextOutIAT = (PROC)*OrigTextOut;
      VirtualProtect(TextOutIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
}

/*** Wrapped COM object interface ***/

// We do not link to mediaspy.dll, since that would clutter up every process.
typedef HRESULT (STDAPICALLTYPE * LPFNGETOBJECT)(REFCLSID, UINT, REFIID, PVOID*);
LPFNGETOBJECT MediaSpyGetCurrentObject = NULL;
typedef HRESULT (STDAPICALLTYPE * LPFNCONVERTOLE)(LPOLESTR, LPSTR, size_t,BOOL);
LPFNCONVERTOLE MediaSpyConvertOle = NULL;
static REFTIME fgDuration, fgPosition;
static char szfgFileName[MAX_PATH];
static DVD_DOMAIN dvdDomain;
static DVD_PLAYBACK_LOCATION dvdLocation;
static ULONG ulTotalTime;

BOOL MatchMediaSpy(UINT nClass)
{
  if (NULL == MediaSpyGetCurrentObject) {
    HINSTANCE hInst = GetModuleHandle("MEDIASPY.DLL");
    if (NULL == hInst) return FALSE;
    // Once it has come in, we need to keep it in so that we can keep
    // the function pointer around.
    hInst = LoadLibrary("MEDIASPY.DLL");
    if (NULL == hInst) return FALSE;
    MediaSpyGetCurrentObject = (LPFNGETOBJECT)
      GetProcAddress(hInst, "MediaSpyGetCurrentObject");
    if (NULL == MediaSpyGetCurrentObject) return FALSE;
    MediaSpyConvertOle = (LPFNCONVERTOLE)
      GetProcAddress(hInst, "MediaSpyConvertOle");
    if (NULL == MediaSpyConvertOle) return FALSE;
  }
  switch (nClass) {
  case MS_FILTER_GRAPH:
    {
      // There may be multiple graphs lying around; use the eldest one
      // that seems complete.
      UINT n = 0;
      while (TRUE) {
        IFilterGraph *pFG = NULL;
        HRESULT hr = MediaSpyGetCurrentObject(CLSID_FilterGraph, n,
                                              IID_IFilterGraph, (void**)&pFG);
        if (S_OK != hr) break;  // Including S_FALSE for none.
        IMediaPosition *pMP = NULL;
        hr = pFG->QueryInterface(IID_IMediaPosition, (void**)&pMP);
        if (SUCCEEDED(hr)) {
          hr = pMP->get_Duration(&fgDuration);
          if (SUCCEEDED(hr))
            hr = pMP->get_CurrentPosition(&fgPosition);
          pMP->Release();
        }
        IEnumFilters *pEF = NULL;
        BOOL bFound = FALSE;
        hr = pFG->EnumFilters(&pEF);
        if (SUCCEEDED(hr)) {
          while (!bFound) {
            IBaseFilter *pBF = NULL;
            hr = pEF->Next(1, &pBF, NULL);
            if (S_OK != hr) break;
            IFileSourceFilter *pFSF = NULL;
            hr = pBF->QueryInterface(IID_IFileSourceFilter, (void**)&pFSF);
            if (SUCCEEDED(hr)) {
              LPOLESTR wszFileName = NULL;
              AM_MEDIA_TYPE mtype;
              hr = pFSF->GetCurFile(&wszFileName, &mtype);
              pFSF->Release();
              if (SUCCEEDED(hr)) {
                if (NULL != wszFileName)
                  hr = MediaSpyConvertOle(wszFileName, 
                                          szfgFileName, sizeof(szfgFileName),
                                          TRUE);
                else {
                  // The filter may not support getting the filename,
                  // but the graph builder's render method will have
                  // given it the file's name.
                  FILTER_INFO finfo;
                  hr = pBF->QueryFilterInfo(&finfo);
                  if (SUCCEEDED(hr)) {
                    hr = MediaSpyConvertOle(finfo.achName, 
                                            szfgFileName, sizeof(szfgFileName),
                                            FALSE);
                    if (NULL != finfo.pGraph)
                      finfo.pGraph->Release();
                  }
                }
                bFound = TRUE;
              }
            }
            // Could look for a file sink as well to indicate that we are capturing.
            pBF->Release();
          }
          pEF->Release();
        }
        pFG->Release();
        if (bFound) return TRUE;
        n++;
      }
      return FALSE;
    }
  case MS_DVD_NAVIGATOR:
    {
      IDvdInfo *pDVD = NULL;
      HRESULT hr = MediaSpyGetCurrentObject(CLSID_DVDNavigator, 0,
                                            IID_IDvdInfo, (void**)&pDVD);
      if (S_OK != hr) return FALSE; // Including S_FALSE for none.
      hr = pDVD->GetCurrentDomain(&dvdDomain);
      if (SUCCEEDED(hr)) {
        if (DVD_DOMAIN_Title == dvdDomain) {
          hr = pDVD->GetCurrentLocation(&dvdLocation);
          if (SUCCEEDED(hr)) {
            hr = pDVD->GetTotalTitleTime(&ulTotalTime);
            if (VFW_S_DVD_NON_ONE_SEQUENTIAL == hr) {
              ulTotalTime = 0xFFFFFFFF;
              hr = S_OK;
            }
          }
        }
        else {
          dvdLocation.TitleNum = 0xFFFFFFFF;
          dvdLocation.ChapterNum = 0xFFFFFFFF;
          dvdLocation.TimeCode = 0xFFFFFFFF;
          ulTotalTime = 0xFFFFFFFF;
        }
      }
      pDVD->Release();
      return SUCCEEDED(hr);
    }

  // TODO: It should be possible to get the filename of a media file
  // from the graph by looking for the stream filter and asking it.

  default:
    ASSERT(FALSE);
    return FALSE;
  }
}

void FormatREFTIME(REFTIME time, LPSTR szBuf, size_t nSize)
{
  int nHours = (int)(time / 3600.0);
  time -= nHours * 3600.0;
  int nMinutes = (int)(time / 60.0);
  time -= nMinutes * 60.0;
  int nSeconds = (int)time;
  // Using this doesn't seem to bloat the DLL as much as I'd feared.
  sprintf(szBuf, "%02d:%02d:%02d", nHours, nMinutes, nSeconds);
}

void FormatTIMECODE(const DVD_TIMECODE& tc, LPSTR szBuf, size_t nSize)
{
  // Easy enough this way since it's already BCD.
  szBuf[0] = '0' + (char)tc.Hours10;
  szBuf[1] = '0' + (char)tc.Hours1;
  szBuf[2] = ':';
  szBuf[3] = '0' + (char)tc.Minutes10;
  szBuf[4] = '0' + (char)tc.Minutes1;
  szBuf[5] = ':';
  szBuf[6] = '0' + (char)tc.Seconds10;
  szBuf[7] = '0' + (char)tc.Seconds1;
  szBuf[8] = '\0';
}

void ExtractMediaSpy(UINT nField, LPSTR szBuf, size_t nSize)
{
  switch (nField) {
  case MS_FG_DURATION:
    FormatREFTIME(fgDuration, szBuf, nSize);
    break;
  case MS_FG_POSITION:
    FormatREFTIME(fgPosition, szBuf, nSize);
    break;
  case MS_FG_FILENAME:
    strncpy(szBuf, szfgFileName, nSize);
    break;
  case MS_DVD_DOMAIN:
    switch (dvdDomain) {
    case DVD_DOMAIN_FirstPlay:
      strncpy(szBuf, "PLAY", nSize);
      break;
    case DVD_DOMAIN_VideoManagerMenu:
    case DVD_DOMAIN_VideoTitleSetMenu:
      strncpy(szBuf, "MENU", nSize);
      break;
    case DVD_DOMAIN_Title:
      strncpy(szBuf, "DVD", nSize);
      break;
    case DVD_DOMAIN_Stop:
      strncpy(szBuf, "STOP", nSize);
      break;
    }
    break;
  case MS_DVD_TITLE:
    if (dvdLocation.TitleNum == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      sprintf(szBuf, "%02d", dvdLocation.TitleNum);
    break;
  case MS_DVD_CHAPTER:
    if (dvdLocation.ChapterNum == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      sprintf(szBuf, "%02d", dvdLocation.ChapterNum);
    break;
  case MS_DVD_TOTAL:
    if (ulTotalTime == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      FormatTIMECODE(*(DVD_TIMECODE*)&ulTotalTime, szBuf, nSize);
    break;
  case MS_DVD_TIME:
    if (dvdLocation.TimeCode == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      FormatTIMECODE(*(DVD_TIMECODE*)&dvdLocation.TimeCode, szBuf, nSize);
    break;
  }
}

/*** Functions called from the plug-in itself, rather than a hooked process. ***/
// Enable windows message hook globally.
void DISPLAYSPYHOOK_API DS_StartHook(DWORD dwThreadId)
{
  GetMutex(INFINITE);
  if (NULL == gs_hWndHook)
    gs_hWndHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndHookProc, g_hInst, 0);
  if (NULL == gs_hMsgHook)
    gs_hMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgHookProc, g_hInst, 0);
  gs_dwThreadId = dwThreadId;
  ReleaseMutex(g_hMutex);
}

// Disable windows message hook.
void DISPLAYSPYHOOK_API DS_EndHook(DWORD dwThreadId)
{
  GetMutex(INFINITE);
  if (dwThreadId == gs_dwThreadId) {
    if (NULL != gs_hWndHook) {
      UnhookWindowsHookEx(gs_hWndHook);
      gs_hWndHook = NULL;
    }
    if (NULL != gs_hMsgHook) {
      UnhookWindowsHookEx(gs_hMsgHook);
      gs_hMsgHook = NULL;
    }
  }
  ReleaseMutex(g_hMutex);
}

// Get number of possible matches (for populating menu).
size_t DISPLAYSPYHOOK_API DS_GetMatchCount()
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  return g_nMatches;
}

// Get number of extracted strings for match (for populating menu).
size_t DISPLAYSPYHOOK_API DS_GetMatchIndexCount(size_t nCurrent)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  if (nCurrent >= g_nMatches)
    return 0;
  else
    return g_pMatches[nCurrent].nExtracts;
}

// Get name of event.
void DISPLAYSPYHOOK_API DS_GetName(size_t nMatch, size_t nIndex,
                                   char *szBuf, size_t nSize)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  *szBuf = '\0';
  if ((nMatch >= g_nMatches) ||
      (nIndex >= g_pMatches[nMatch].nExtracts))
    return;
  BOOL bFirst = TRUE;
  LPCSTR szName = g_pMatches[nMatch].szModule;
  if (NULL != szName) {
    bFirst = FALSE;
    strncpy(szBuf, szName, nSize);
    size_t nLength = strlen(szBuf);
    szBuf += nLength;
    nSize -= nLength;
    if (nSize <= 1) return;
  }
  szName = g_pMatches[nMatch].szMatch;
  if (NULL != szName) {
    if (bFirst)
      bFirst = FALSE;
    else {
      *szBuf++ = '.';
      nSize--;
    }
    strncpy(szBuf, szName, nSize);
    size_t nLength = strlen(szBuf);
    szBuf += nLength;
    nSize -= nLength;
    if (nSize <= 1) return;
  }
  szName = g_pMatches[nMatch].pExtracts[nIndex].szName;
  if (NULL != szName) {
    if (bFirst)
      bFirst = FALSE;
    else {
      *szBuf++ = '.';
      nSize--;
    }
    strncpy(szBuf, szName, nSize);
  }
}

// Get the next extracted string from the ring buffer.
BOOL DISPLAYSPYHOOK_API DS_GetNext(size_t *nMatch, size_t *nIndex,
                                   char *szBuf, size_t nSize)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  GetMutex(INFINITE);
  if (gs_bEmpty) {
    ReleaseMutex(g_hMutex);
    return FALSE;
  }
  DisplayBuf *pBuf = gs_pDisplayBufs + gs_nRead++;
  *nMatch = pBuf->nMatch - g_nMatchOffset;
  *nIndex = pBuf->nIndex;
  strncpy(szBuf, pBuf->szValue, nSize);
  if (gs_nRead == MAX_BUFS) gs_nRead = 0;
  gs_bEmpty = (gs_nRead == gs_nWrite);
  ReleaseMutex(g_hMutex);
  return TRUE;
}

// Reset the ring buffer.
void DISPLAYSPYHOOK_API DS_Reset()
{
  GetMutex(INFINITE);
  gs_nRead = gs_nWrite;
  gs_bEmpty = TRUE;
  ReleaseMutex(g_hMutex);
}

/*** DLL entry function ***/
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
    g_hInst = hInstance;
    IndexMatches(FALSE);
    InstallPatches();
    break;
  case DLL_PROCESS_DETACH:
    RemovePatches();
    if (NULL != g_pMatches)
      delete [] g_pMatches;
    if (NULL != g_hMutex)
      CloseHandle(g_hMutex);
    break;
  }
  return TRUE;
}


#include "stdafx.h"
#include "DisplaySpyHook.h"

#define ASSERT(x) _ASSERTE(x)
#define countof(x) sizeof(x)/sizeof(x[0])

struct MatchEntry
{
  UINT nCode;
  LPCSTR szVal;
  DWORD dwVal;
};

#define ENTRY0(n) { n },
#define ENTRY(n,s,l) { n, s, l },
#define ENTRY_STR(n,s) { n, s },
#define ENTRY_NUM(n,l) { n, NULL, l },

const UINT ENTRY_MODULE = 1;
const UINT ENTRY_BEGIN = 2;
const UINT ENTRY_EXTRACT = 3;
const UINT ENTRY_END = 4;

#define BEGIN_MODULE(x) ENTRY_STR(ENTRY_MODULE,#x)
#define BEGIN_MATCH(x,r) ENTRY(ENTRY_BEGIN,#x,r)
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
const UINT MATCH_CONTROLID = 1007;
const UINT MATCH_NOTIFY_CODE = 1010;
const UINT MATCH_NOTIFY_FROM = 1011;
const UINT MATCH_PATCH = 1020;
const UINT MATCH_MEDIA_SPY = 1030;

const UINT EXTRACT_CONSTANT = 2001;
const UINT EXTRACT_GETTEXT = 2002;
const UINT EXTRACT_LPARAM_STR = 2003;
const UINT EXTRACT_SB_GETTEXT = 2010;
const UINT EXTRACT_MEDIA_SPY = 2030;

const HWND HWND_PATCH = (HWND)0xDEADFACE;
const UINT PATCH_TEXTOUT = 0x80000001;

const UINT MS_FILTER_GRAPH = 1;
const UINT MS_DVD_NAVIGATOR = 2;
const UINT MS_FG_DURATION = 3;
const UINT MS_FG_POSITION = 4;
const UINT MS_DVD_DOMAIN = 5;
const UINT MS_DVD_TITLE = 6;
const UINT MS_DVD_CHAPTER = 7;
const UINT MS_DVD_TOTAL = 8;
const UINT MS_DVD_TIME = 9;

/*** Patterns that drive the hook to extract display information. ***/
static MatchEntry g_matches[] = {

  BEGIN_MODULE(WinDVD)

    // This is an update to the tracker bar in the status bar in the
    // display window.  When it happens, the status bar fields have been
    // changed as well (via MFC).
    BEGIN_MATCH(WinDVD,1)
      ENTRY_NUM(MATCH_MESSAGE, TBM_SETPOS)
      ENTRY_NUM(MATCH_CONTROLID, 1)
      ENTRY_STR(MATCH_CLASS, TRACKBAR_CLASS)
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, STATUSCLASSNAME)
      ENTRY_STR(ENTRY_HWND_GRANDPARENT|MATCH_CLASS, "WinDVDClass")
     BEGIN_EXTRACT()
      ENTRY(ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, "Chapter: ", 3) // Chapter
      ENTRY(ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, "Time: ", 4) // Time
      ENTRY_NUM(ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 5) // Video mode
      ENTRY_NUM(ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 6) // Not sure
      ENTRY_NUM(ENTRY_HWND_PARENT|EXTRACT_SB_GETTEXT, 7) // Audio mode
    END_MATCH()

    BEGIN_MATCH(WinDVD.Close,0)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "WinDVDClass")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(POWERDVD)

    // This only has the time.  The title# and chapter# are displayed as
    // graphics in the "remote" window.
    BEGIN_MATCH(PowerDVD.Time,5)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_STR(MATCH_CLASS, "Static")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "#32770")
      ENTRY_STR(ENTRY_HWND_GRANDPARENT|MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()
    // This only happens when playing a .VOB file: the window title is
    // changed to the file in question.  Better than nothing.
    BEGIN_MATCH(PowerDVD.Title,6)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
      ENTRY_STR(MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

  BEGIN_MODULE(accessDTV)

    BEGIN_MATCH(accessDTV.ChanText,1)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(51,71))
      ENTRY_STR(ENTRY_NOT|MATCH_LPARAM_STR, "XXXXXXXXXXXXXX")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_MATCH(accessDTV.ChanNoText,0)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(51,71))
      ENTRY_STR(MATCH_LPARAM_STR, "XXXXXXXXXXXXXX")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

    BEGIN_MATCH(accessDTV.Channel,2)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTOUT)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(50,43))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_MATCH(accessDTV.Close,0)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_GETTEXT, "accessDTV")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(zplayer)

    BEGIN_MATCH(ZoomPlayer.DVD,1)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_MATCH(ZoomPlayer.Media,4)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_FG_POSITION)
    END_MATCH()

    BEGIN_MATCH(ZoomPlayer.OSD,6)
      ENTRY_NUM(MATCH_MESSAGE, WM_PAINT)
      ENTRY_STR(MATCH_CLASS, "TPanel")
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "TOSDForm")
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_GETTEXT)
    END_MATCH()

    BEGIN_MATCH(ZoomPlayer.Close,0)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(TheaterTek DVD)

    BEGIN_MATCH(TheaterTek,1)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_MATCH(TheaterTek.Close,0)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_GETTEXT, "Overlay")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(ATIMMC)

    BEGIN_MATCH(ATI.DVD,1)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_MATCH(ATI.MMC,4)
      ENTRY_NUM(MATCH_MESSAGE, WM_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      ENTRY_NUM(EXTRACT_MEDIA_SPY, MS_FG_POSITION)
    END_MATCH()

    BEGIN_MATCH(ATI.Close,0)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
     BEGIN_EXTRACT()
      ENTRY_STR(EXTRACT_CONSTANT, "")
    END_MATCH()

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
  LPCSTR szName;                // Name of this match event.
  int nRegister;                // First text register.
  size_t nMatches;              // Number of match predicates.
  MatchEntry *pMatches;         // Pointer to first match predicate.
  size_t nExtracts;             // Number of match extracts.
  MatchEntry *pExtracts;        // Pointer to first match extract.
};

/*** Shared global data ***/
#pragma data_seg(".SHARDATA")
static HHOOK gs_hWndHook = NULL, gs_hMsgHook = NULL;
static DWORD gs_dwThreadId = NULL;
static DWORD gs_dwEnabled = 0xFFFFFFFF;

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
  char szModBuf[MAX_PATH], *szModule = NULL;
  if (!bAll) {
    GetModuleFileName(NULL, szModBuf, sizeof(szModBuf));
    szModule = strrchr(szModBuf, '.');
    if (NULL != szModule)
      *szModule = '\0';         // Remove extension.
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
        else if (!_stricmp(g_matches[i].szVal, szModule))
          nBegin = i + 1;       // First module entry.
      }
    }
    else if (ENTRY_BEGIN == g_matches[i].nCode) {
      if (bAll || nBegin)
        nMatches++;
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
      i++;
      continue;
    }
    ASSERT(ENTRY_BEGIN == g_matches[i].nCode);
    pMatches[j].szName = g_matches[i].szVal;
    pMatches[j].nRegister = g_matches[i].dwVal;
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
}

// Process a single message
void DoMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (NULL == g_pMatches)
    return;
  BOOL bNotify = FALSE;
  for (size_t i = 0; i < g_nMatches; i++) {
    if (0 == (gs_dwEnabled & (1 << i))) continue;
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
  if (nCode >= 0) {
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
LPFNGETCLASSOBJECT MediaSpyGetCurrentObject = NULL;
static REFTIME fgDuration, fgPosition;
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
    MediaSpyGetCurrentObject = (LPFNGETCLASSOBJECT)
      GetProcAddress(hInst, "MediaSpyGetCurrentObject");
    if (NULL == MediaSpyGetCurrentObject) return FALSE;
  }
  switch (nClass) {
  case MS_FILTER_GRAPH:
    {
      IMediaPosition *pMP = NULL;
      HRESULT hr = MediaSpyGetCurrentObject(CLSID_FilterGraph, IID_IMediaPosition, 
                                            (void**)&pMP);
      if (S_OK != hr) return FALSE; // Including S_FALSE for none.
      hr = pMP->get_Duration(&fgDuration);
      if (SUCCEEDED(hr))
        hr = pMP->get_CurrentPosition(&fgPosition);
      pMP->Release();
      return SUCCEEDED(hr);
    }
  case MS_DVD_NAVIGATOR:
    {
      IDvdInfo *pDVD = NULL;
      HRESULT hr = MediaSpyGetCurrentObject(CLSID_DVDNavigator, IID_IDvdInfo, 
                                            (void**)&pDVD);
      if (S_OK != hr) return FALSE; // Including S_FALSE for none.
      hr = pDVD->GetCurrentDomain(&dvdDomain);
      if (SUCCEEDED(hr)) {
        hr = pDVD->GetCurrentLocation(&dvdLocation);
        if (VFW_E_DVD_INVALIDDOMAIN == hr) {
          dvdLocation.TitleNum = 0xFFFFFFFF;
          dvdLocation.ChapterNum = 0xFFFFFFFF;
          dvdLocation.TimeCode = 0xFFFFFFFF;
          hr = S_OK;
        }
      }
      if (SUCCEEDED(hr)) {
        hr = pDVD->GetTotalTitleTime(&ulTotalTime);
        if ((VFW_E_DVD_INVALIDDOMAIN == hr) ||
            (VFW_S_DVD_NON_ONE_SEQUENTIAL == hr)) {
          ulTotalTime = 0xFFFFFFFF;
          hr = S_OK;
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

// Get name of match.
void DISPLAYSPYHOOK_API DS_GetMatchName(size_t nCurrent, char *szBuf, size_t nSize)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  if (nCurrent >= g_nMatches)
    *szBuf = '\0';
  else
    strncpy(szBuf, g_pMatches[nCurrent].szName, nSize);
}

// Get index of match, given name.
size_t DISPLAYSPYHOOK_API DS_GetMatchIndex(LPCSTR szName)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  for (size_t i = 0; i < g_nMatches; i++)
    if (!strcmp(szName, g_pMatches[i].szName))
      return i;
  return (size_t)-1;
}

// Get text register for match.
int DISPLAYSPYHOOK_API DS_GetMatchRegister(size_t nCurrent)
{
  if (NULL == g_pMatches)
    IndexMatches(TRUE);
  if (nCurrent >= g_nMatches)
    return 0;
  else
    return g_pMatches[nCurrent].nRegister;
}

// Enable a particular match.
void DISPLAYSPYHOOK_API DS_EnableMatch(size_t nIndex, BOOL bEnabled)
{
  if (bEnabled)
    gs_dwEnabled |= (1 << nIndex);
  else
    gs_dwEnabled &= ~(1 << nIndex);
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

/* Windows message hook running in player address space 
$Header$
*/

#include "stdafx.h"
#include "DisplaySpyHook.h"

#ifdef _DEBUG
#define _TRACE
#endif

#define ASSERT(x) _ASSERTE(x)
#define countof(x) sizeof(x)/sizeof(x[0])

LPCSTR _stristr(LPCSTR str, LPCSTR key);

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
const UINT ENTRY_EVENT =            0x20000000;

#define BASE_CODE(n) (n&0xFFFF)

const UINT MATCH_MESSAGE = 1001;
const UINT MATCH_REGISTERED_MESSAGE = 1002;
const UINT MATCH_WPARAM = 1003;
const UINT MATCH_LPARAM = 1004;
const UINT MATCH_LPARAM_STR = 1005;
const UINT MATCH_GETTEXT = 1006;
const UINT MATCH_CLASS = 1007;
const UINT MATCH_CLASS_PREFIX = 1008;
const UINT MATCH_CONTROLID = 1009;
const UINT MATCH_NOTIFY_CODE = 1010;
const UINT MATCH_NOTIFY_FROM = 1011;
const UINT MATCH_ONCE = 1018;
const UINT MATCH_SECOND_TIMER = 1019;
const UINT MATCH_PATCH = 1020;
const UINT MATCH_MEDIA_SPY = 1030;
const UINT MATCH_WINDVD_SETIMAGE = 1031;
const UINT MATCH_MODULE_PATHNAME = 1501;
const UINT MATCH_MODULE_VERSION = 1502;

const UINT EXTRACT_CONSTANT = 2001;
const UINT EXTRACT_GETTEXT = 2002;
const UINT EXTRACT_LPARAM_STR = 2003;
const UINT EXTRACT_LPARAM = 2004;
const UINT EXTRACT_WPARAM = 2005;
const UINT EXTRACT_LPARAM_POINT = 2006;
const UINT EXTRACT_WPARAM_POINT = 2007;
const UINT EXTRACT_HWND = 2008;
const UINT EXTRACT_CLASS = 2009;
const UINT EXTRACT_CONTROLID = 2010;
const UINT EXTRACT_SB_GETTEXT = 2015;
const UINT EXTRACT_LPARAM_SUBSTR = 2020;
const UINT EXTRACT_LPARAM_STR_BEFORE = 2021;
const UINT EXTRACT_LPARAM_STR_AFTER = 2022;
const UINT EXTRACT_LPARAM_TIME_BCD = 2025;
const UINT EXTRACT_MEDIA_SPY = 2030;
const UINT EXTRACT_WINDVD_SETIMAGE = 2031;

const UINT PATCH_TEXTOUT = 1;
const UINT PATCH_DRAWTEXT = 2;
const UINT PATCH_TEXTIMAGE = 3;
const UINT PATCH_POWERDVD_STRETCHDIBITS = 4;
const UINT PATCH_WINDVD_GETIMAGE = 5;

const UINT MS_FILTER_GRAPH = 1;
const UINT MS_DVD_NAVIGATOR = 2;
const UINT MS_TV_TUNER = 3;
const UINT MS_FG_DURATION = 3;
const UINT MS_FG_POSITION = 4;
const UINT MS_FG_FILENAME = 5;
const UINT MS_FG_STATE = 6;
const UINT MS_FG_VIDEO_POSITION = 7;
const UINT MS_DVD_DOMAIN = 8;
const UINT MS_DVD_TITLE = 9;
const UINT MS_DVD_CHAPTER = 10;
const UINT MS_DVD_TOTAL = 11;
const UINT MS_DVD_TIME = 12;
const UINT MS_TV_CHANNEL = 13;

const UINT SKINDVD_STATUS = 1;
const UINT SKINDVD_TIME = 2;
const UINT SKINDVD_TITLE = 3;
const UINT SKINDVD_CHAPTER = 4;

const DWORD MODULE_NO_WIN16_CWP = 1;

enum MatchContext { CONTEXT_WNDPROC = 1, CONTEXT_GETMSG, CONTEXT_PATCH };

/*** Patterns that drive the hook to extract display information. ***/
static MatchEntry g_matches[] = {

  BEGIN_MODULE(WinDVD)

    BEGIN_NMATCH(Status)
      ENTRY_NUM(MATCH_MESSAGE, STM_SETIMAGE)
      ENTRY_NUM(MATCH_WPARAM, IMAGE_BITMAP)
      ENTRY_NUM(MATCH_WINDVD_SETIMAGE, SKINDVD_STATUS)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_WINDVD_SETIMAGE, SKINDVD_STATUS)
    END_MATCH()

    BEGIN_NMATCH(Elapsed)
      ENTRY_NUM(MATCH_MESSAGE, STM_SETIMAGE)
      ENTRY_NUM(MATCH_WPARAM, IMAGE_BITMAP)
      ENTRY_NUM(MATCH_WINDVD_SETIMAGE, SKINDVD_TIME)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_WINDVD_SETIMAGE, SKINDVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Chapter)
      ENTRY_NUM(MATCH_MESSAGE, STM_SETIMAGE)
      ENTRY_NUM(MATCH_WPARAM, IMAGE_BITMAP)
      ENTRY_NUM(MATCH_WINDVD_SETIMAGE, SKINDVD_CHAPTER)
     BEGIN_EXTRACT()
      ENTRY_NUM(EXTRACT_WINDVD_SETIMAGE, SKINDVD_CHAPTER)
    END_MATCH()

    BEGIN_MATCH()
      ENTRY_NUM(MATCH_PATCH, PATCH_WINDVD_GETIMAGE)
     BEGIN_EXTRACT()
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "WinDVDClass")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(PowerDVD)

    BEGIN_NMATCH(Status)
      ENTRY_NUM(MATCH_PATCH, PATCH_POWERDVD_STRETCHDIBITS)
      ENTRY_NUM(MATCH_WPARAM, SKINDVD_STATUS)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Elapsed)
      ENTRY_NUM(MATCH_PATCH, PATCH_POWERDVD_STRETCHDIBITS)
      ENTRY_NUM(MATCH_WPARAM, SKINDVD_TIME)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(TitleNo)
      ENTRY_NUM(MATCH_PATCH, PATCH_POWERDVD_STRETCHDIBITS)
      ENTRY_NUM(MATCH_WPARAM, SKINDVD_TITLE)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Chapter)
      ENTRY_NUM(MATCH_PATCH, PATCH_POWERDVD_STRETCHDIBITS)
      ENTRY_NUM(MATCH_WPARAM, SKINDVD_CHAPTER)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "CyberLink Video Window Class")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
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
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(ZoomPlayer,zplayer)

    BEGIN_NMATCH(DVD)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(VideoPosition, EXTRACT_MEDIA_SPY, MS_FG_VIDEO_POSITION)
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Media)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(VideoPosition, EXTRACT_MEDIA_SPY, MS_FG_VIDEO_POSITION)
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

    BEGIN_NMATCH(Init)
      ENTRY_NUM(MATCH_MESSAGE, WM_SHOWWINDOW)
      ENTRY_NUM(MATCH_WPARAM, TRUE)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
      ENTRY0(MATCH_ONCE)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_HWND)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE_FLAGS(TheaterTek,TheaterTek DVD,MODULE_NO_WIN16_CWP)

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(VideoPosition, EXTRACT_MEDIA_SPY, MS_FG_VIDEO_POSITION)
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
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(CinePlayer)

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(VideoPosition, EXTRACT_MEDIA_SPY, MS_FG_VIDEO_POSITION)
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(ATI,ATIMMC)

    BEGIN_NMATCH(DVD)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(TV)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_TV_TUNER)
     BEGIN_EXTRACT()
      NENTRY_NUM(Channel, EXTRACT_MEDIA_SPY, MS_TV_CHANNEL)
    END_MATCH()

    BEGIN_NMATCH(MMC)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

#if 0
    // Calls to TextOut / DrawText at some point perhaps through bitmap.
    BEGIN_NMATCH(TextImage)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
     BEGIN_EXTRACT()
      NENTRY0(Window,ENTRY_EVENT|EXTRACT_HWND)
      NENTRY0(Caption,ENTRY_EVENT|EXTRACT_GETTEXT)
      NENTRY0(Class,ENTRY_EVENT|EXTRACT_CLASS)
      NENTRY0(ControlID,ENTRY_EVENT|EXTRACT_CONTROLID)
      NENTRY0(Position,ENTRY_EVENT|EXTRACT_WPARAM_POINT)
      NENTRY0(Text,ENTRY_EVENT|EXTRACT_LPARAM_STR)
    END_MATCH()
#endif

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "VideoRenderer")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(ShowShifter)
  // Getting information off the screen uses a separate module built
  // with the program's SDK.

    BEGIN_NMATCH(DVD)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    BEGIN_NMATCH(Media)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "ShowShifterWndClass")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(BSPlayer,bplay)
  // There are also some text controls, which display things like
  // Pause, that might be worth getting.  Their position comes from
  // the skin.ini file in the skin directory / archive.  They appear
  // to be drawn with DrawText to a zero-origin DC, such as a bitmap
  // that is then blted to the screen.

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "BSPlayer")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

#if 0
  // Does anyone still use this?
  BEGIN_NMODULE(SASAMI2k,sasami2000)

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TMainForm")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
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
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()
#endif

  BEGIN_NMODULE(PowerDivX,PowerDivX*)

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
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
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(RadLight)
  // There is mention of a RadLight OpenAPI SDK, which allows writing
  // "modules", which might be able to get more information.  I cannot
  // find it, though.

    BEGIN_MATCH()
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TApplication")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  // Digital CD Player 
  BEGIN_MODULE(cdplayer)

    BEGIN_NMATCH(Time) 
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT) 
      ENTRY_STR(MATCH_CLASS, "TApplication") 
    BEGIN_EXTRACT() 
      ENTRY0(EXTRACT_LPARAM_STR) 
    END_MATCH() 

#if 0
    BEGIN_NMATCH(Album) 
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT) 
      ENTRY_STR(MATCH_CLASS, "TPanel") 
    BEGIN_EXTRACT() 
      ENTRY0(EXTRACT_LPARAM_STR) 
    END_MATCH() 
#endif

    BEGIN_NMATCH(Track) 
      ENTRY_NUM(MATCH_MESSAGE, CB_GETLBTEXT) 
      ENTRY_STR(MATCH_CLASS, "TComboBox") 
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "TMyanDigitalCDPlayerForm") 
    BEGIN_EXTRACT() 
      ENTRY0(EXTRACT_GETTEXT) 
    END_MATCH() 

#if 0
    BEGIN_NMATCH(Status)
      ENTRY_NUM(MATCH_MESSAGE, WM_USER+1) 
      ENTRY_STR(MATCH_CLASS, "TStatusBar") 
      ENTRY_STR(ENTRY_HWND_PARENT|MATCH_CLASS, "TMyanDigitalCDPlayerForm") 
    BEGIN_EXTRACT() 
  // How to extract anything outside a Delphi program?
    END_MATCH() 
#endif

    BEGIN_NMATCH(Close) 
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY) 
      ENTRY_STR(MATCH_CLASS, "TMyanDigitalCDPlayerForm") 
    BEGIN_EXTRACT() 
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH() 

  BEGIN_NMODULE(Eugenes,station)
      ENTRY_STR(MATCH_MODULE_PATHNAME, "Eugene")
    // The main window's class is TFMain, but it does not reliably get these messages.

    BEGIN_NMATCH(Init)
      ENTRY_NUM(MATCH_MESSAGE, WM_SHOWWINDOW)
      ENTRY_NUM(MATCH_WPARAM, TRUE)
      ENTRY_STR(MATCH_CLASS, "TfDisplayForm")
      ENTRY0(MATCH_ONCE)
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_HWND)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "TThreadWindow")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_NMODULE(DVDStation,station)

    BEGIN_NMATCH(Title.1)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
      ENTRY_STR(MATCH_CLASS, "Title")
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(5,4))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Title.2)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
      ENTRY_STR(MATCH_CLASS, "Title")
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(51,2))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_CLASS, "DVDST")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

  BEGIN_MODULE(MyHD)

#if 0
    BEGIN_NMATCH(TextImage)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
     BEGIN_EXTRACT()
      NENTRY0(Window,ENTRY_EVENT|EXTRACT_HWND)
      NENTRY0(Caption,ENTRY_EVENT|EXTRACT_GETTEXT)
      NENTRY0(Class,ENTRY_EVENT|EXTRACT_CLASS)
      NENTRY0(ControlID,ENTRY_EVENT|EXTRACT_CONTROLID)
      NENTRY0(Position,ENTRY_EVENT|EXTRACT_WPARAM_POINT)
      NENTRY0(Text,ENTRY_EVENT|EXTRACT_LPARAM_STR)
    END_MATCH()
#else
    // There are two windows displaying what looks to be channel info.
    // More experimentation is needed to determine which is best and
    // whether either has additional PSIP info.
    BEGIN_NMATCH(Channel)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
      ENTRY_NUM(MATCH_WPARAM, MAKELONG(25,60))
     BEGIN_EXTRACT()
      ENTRY0(EXTRACT_LPARAM_STR)
    END_MATCH()
#endif

    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
      ENTRY_STR(MATCH_GETTEXT, "Remote Controller")
     BEGIN_EXTRACT()
      ENTRY_STR(ENTRY_EVENT|EXTRACT_CONSTANT, "")
    END_MATCH()

#if 0
  // Uncomment this and edit to begin adding a new application.
  BEGIN_MODULE(XXX)

    // DVDNavigator.
    BEGIN_NMATCH(DVD)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_DVD_NAVIGATOR)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Domain, EXTRACT_MEDIA_SPY, MS_DVD_DOMAIN)
      NENTRY_NUM(TitleNo, EXTRACT_MEDIA_SPY, MS_DVD_TITLE)
      NENTRY_NUM(Chapter, EXTRACT_MEDIA_SPY, MS_DVD_CHAPTER)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_DVD_TOTAL)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_DVD_TIME)
    END_MATCH()

    // Filter graph.
    BEGIN_NMATCH(Media)
      ENTRY0(MATCH_SECOND_TIMER)
      ENTRY_NUM(MATCH_MEDIA_SPY, MS_FILTER_GRAPH)
     BEGIN_EXTRACT()
      NENTRY_NUM(State, EXTRACT_MEDIA_SPY, MS_FG_STATE)
      NENTRY_NUM(Duration, EXTRACT_MEDIA_SPY, MS_FG_DURATION)
      NENTRY_NUM(Elapsed, EXTRACT_MEDIA_SPY, MS_FG_POSITION)
      NENTRY_NUM(File, EXTRACT_MEDIA_SPY, MS_FG_FILENAME)
    END_MATCH()

    // Set window text.
    BEGIN_NMATCH(SetText)
      ENTRY_NUM(MATCH_MESSAGE, WM_SETTEXT)
     BEGIN_EXTRACT()
      NENTRY0(Window,ENTRY_EVENT|EXTRACT_HWND)
      NENTRY0(Class,ENTRY_EVENT|EXTRACT_CLASS)
      NENTRY0(Text,ENTRY_EVENT|EXTRACT_LPARAM_STR)
    END_MATCH()

    // Calls to TextOut / DrawText at some point perhaps through bitmap.
    BEGIN_NMATCH(TextImage)
      ENTRY_NUM(MATCH_PATCH, PATCH_TEXTIMAGE)
     BEGIN_EXTRACT()
      NENTRY0(Window,ENTRY_EVENT|EXTRACT_HWND)
      NENTRY0(Caption,ENTRY_EVENT|EXTRACT_GETTEXT)
      NENTRY0(Class,ENTRY_EVENT|EXTRACT_CLASS)
      NENTRY0(ControlID,ENTRY_EVENT|EXTRACT_CONTROLID)
      NENTRY0(Position,ENTRY_EVENT|EXTRACT_WPARAM_POINT)
      NENTRY0(Text,ENTRY_EVENT|EXTRACT_LPARAM_STR)
    END_MATCH()

    // Close a window.
    BEGIN_NMATCH(Close)
      ENTRY_NUM(MATCH_MESSAGE, WM_DESTROY)
     BEGIN_EXTRACT()
      NENTRY0(Caption,EXTRACT_GETTEXT)
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
  char szValue[MAX_PATH];       // Extracted string
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
BOOL MatchWinDVDSetImage(HWND hWnd, UINT nType, HBITMAP hBitmap);
void ExtractWinDVDSetImage(UINT nType, LPSTR szBuf, size_t nSize);

BOOL DoMatch2(const MatchEntry *pEntry, MatchContext nCtx,
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
  case MATCH_REGISTERED_MESSAGE:
    if (0 == pEntry->dwVal)
      ((MatchEntry*)pEntry)->dwVal = RegisterWindowMessage(pEntry->szVal);
    /* falls through */
  case MATCH_MESSAGE:
    if ((CONTEXT_WNDPROC != nCtx) && (CONTEXT_GETMSG != nCtx))
      return FALSE;
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
  case MATCH_ONCE:
    return (((MatchEntry*)pEntry)->dwVal++ == 0);
  case MATCH_SECOND_TIMER:
    {
      if (WM_TIMER != nMsg)
        return FALSE;
      ULONG ulNow = GetTickCount();
      if ((ulNow - pEntry->dwVal) < 900)
        return FALSE;           // Less than a second (more or less) since last time.
      char szCName[256];
      GetClassName(hWnd, szCName, sizeof(szCName));
      if (!strcmp(szCName, "VideoRenderer"))
        return FALSE;           // Try not to interfere with playback.
      ((MatchEntry*)pEntry)->dwVal = ulNow;
      return TRUE;
    }
  case MATCH_PATCH:
    if (CONTEXT_PATCH != nCtx)
      return FALSE;
    return (nMsg == (UINT)pEntry->dwVal);
  case MATCH_MEDIA_SPY:
    return MatchMediaSpy((UINT)pEntry->dwVal);
  case MATCH_WINDVD_SETIMAGE:
    return MatchWinDVDSetImage(hWnd, (UINT)pEntry->dwVal, (HBITMAP)lParam);
  default:
    ASSERT(FALSE);
    return FALSE;
  }
}

BOOL DoMatch1(const MatchEntry *pEntry, MatchContext nCtx,
              HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (pEntry->nCode & ENTRY_NOT)
    return !DoMatch2(pEntry, nCtx, hWnd, nMsg, wParam, lParam);
  else
    return DoMatch2(pEntry, nCtx, hWnd, nMsg, wParam, lParam);
}

BOOL DoMatch(const MatchIndexEntry *pEntry, MatchContext nCtx,
             HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  for (size_t i = 0; i < pEntry->nMatches; i++)
    if (!DoMatch1(pEntry->pMatches+i, nCtx, hWnd, nMsg, wParam, lParam))
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
  case EXTRACT_HWND:
    sprintf(szBuf, "%X", hWnd);
    break;
  case EXTRACT_CLASS:
    GetClassName(hWnd, szBuf, nSize);
    break;
  case EXTRACT_CONTROLID:
    sprintf(szBuf, "%lX", GetWindowLong(hWnd, GWL_ID));
    break;
  case EXTRACT_SB_GETTEXT:
    SendMessage(hWnd, SB_GETTEXT, pEntry->dwVal, (LPARAM)szBuf);
    if (NULL != pEntry->szVal) {
      size_t prefixLen = strlen(pEntry->szVal);
      if (!strncmp(szBuf, pEntry->szVal, prefixLen))
        memmove(szBuf, szBuf + prefixLen, strlen(szBuf) - prefixLen + 1);
    }
    break;
  case EXTRACT_LPARAM_SUBSTR:
    {
      LPCSTR szData = (LPCSTR)lParam;
      size_t nOff = LOWORD(pEntry->dwVal);
      size_t nLen = HIWORD(pEntry->dwVal);
      size_t nDLen = strlen(szData);
      if (nDLen <= nOff) {
        *szBuf = '\0';
        break;
      }
      nDLen -= nOff;
      if (nLen > nDLen)
        nLen = nDLen;
      if (nLen >= nSize)
        nLen = nSize - 1;
      memcpy(szBuf, szData + nOff, nLen);
      szBuf[nLen] = '\0';
    }
    break;
  case EXTRACT_LPARAM_STR_BEFORE:
    {
      LPCSTR szData = (LPCSTR)lParam;
      LPCSTR szMatch = strstr(szData, pEntry->szVal);
      if (NULL == szMatch) {
        *szBuf = '\0';
        break;
      }
      size_t nLen = szMatch - szData;
      if (nLen >= nSize)
        nLen = nSize - 1;
      memcpy(szBuf, szData, nLen);
      szBuf[nLen] = '\0';
    }
    break;
  case EXTRACT_LPARAM_STR_AFTER:
    {
      LPCSTR szData = (LPCSTR)lParam;
      LPCSTR szMatch = strstr(szData, pEntry->szVal);
      if (NULL == szMatch) {
        *szBuf = '\0';
        break;
      }
      szMatch += strlen(pEntry->szVal);
      strncpy(szBuf, szMatch, nSize);
    }
    break;
  case EXTRACT_LPARAM_TIME_BCD:
    if (lParam & 0xFF000000)
      sprintf(szBuf, "%d:%02d:%02d", ((lParam >> 24) & 0xFF), 
              ((lParam >> 16) & 0xFF), ((lParam >> 8) & 0xFF));
    else
      sprintf(szBuf, "%d:%02d", 
              ((lParam >> 16) & 0xFF), ((lParam >> 8) & 0xFF));
    break;
  case EXTRACT_MEDIA_SPY:
    ExtractMediaSpy((UINT)pEntry->dwVal, szBuf, nSize);
    break;
  case EXTRACT_WINDVD_SETIMAGE:
    ExtractWinDVDSetImage((UINT)pEntry->dwVal, szBuf, nSize);
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
    if (!(pEntry->pExtracts[dbuf.nIndex].nCode & ENTRY_EVENT)) {
      // Search back in the queue for an entry with the same match and index.
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
          if (gs_nRead == gs_nWrite) gs_bEmpty = TRUE;
        }
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
  char szModulePathName[MAX_PATH], szModuleFileName[64];
  LPCSTR szModuleName = szModuleFileName;
  DWORD dwModuleFlags = 0;
  if (!bAll) {
    GetModuleFileName(NULL, szModulePathName, sizeof(szModulePathName));
    LPSTR psz = strrchr(szModulePathName, '\\');
    if (NULL != psz)
      psz++;                    // Remove directory.
    else
      psz = szModulePathName;
    strncpy(szModuleFileName, psz, sizeof(szModuleFileName));
    psz = strrchr(szModuleFileName, '.');
    if (NULL != psz)
      *psz = '\0';              // Remove extension.
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
          bMatch = !_strnicmp(szKey, szModuleFileName, nKey-1);
        else
          bMatch = !_stricmp(szKey, szModuleFileName);
        if (bMatch) {
          szModuleName = g_matches[i].szName; // Our name for it.
          dwModuleFlags = g_matches[i].dwVal;
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
    else if (!bAll && nBegin && !nMatches) {
      // A module match predicate.
      BOOL bMatch = TRUE;
      switch (BASE_CODE(g_matches[i].nCode)) {
      case MATCH_MODULE_PATHNAME:
        bMatch = (NULL != _stristr(szModulePathName, g_matches[i].szVal));
        break;
      case MATCH_MODULE_VERSION:
        // TODO: Something involving GetFileVersionInfo.  Maybe load
        // dynamically to avoid pulling VERSION.DLL into every
        // process.
        break;
      }
      if (g_matches[i].nCode & ENTRY_NOT)
        bMatch = !bMatch;
      if (bMatch)
        nBegin = i + 1;
      else
        nBegin = 0;
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
      szModuleName = g_matches[i].szName;
      while ((++i < countof(g_matches)) && (ENTRY_BEGIN != g_matches[i].nCode)) {
        ASSERT((1500 < BASE_CODE(g_matches[i].nCode)) && 
               (2000 > BASE_CODE(g_matches[i].nCode)));
      }
      continue;
    }
    pMatches[j].szModule = szModuleName;
    pMatches[j].szMatch = g_matches[i].szName;
    i++;
    pMatches[j].nMatches = 0;
    pMatches[j].pMatches = g_matches+i;
    while ((i < countof(g_matches)) && (ENTRY_EXTRACT != g_matches[i++].nCode)) {
      ASSERT((1000 < BASE_CODE(g_matches[i-1].nCode)) && 
             (1500 > BASE_CODE(g_matches[i-1].nCode)));
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

  g_dwModuleFlags = dwModuleFlags;
  if (dwModuleFlags & MODULE_NO_WIN16_CWP) {
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    if (GetVersionEx(&osinfo) &&
        (VER_PLATFORM_WIN32_NT != osinfo.dwPlatformId))
      // Don't try to decode CallWndProc messages from this application under Win9x/Me.
      // Just matching causes some kind of damage which makes Windows unable to reboot.
      // I think it may have something to do with the stack depth and thunking.
      g_bNoCallWndProc = TRUE;
  }

  if (!bAll && (nMatches > 0)) {
#ifdef _TRACE
  {
    char szBuf[2048];
    sprintf(szBuf, "%s spy hooks chosen for %s\n", szModuleName, szModulePathName);
    OutputDebugString(szBuf);
  }
#endif
    // Clear any buffers from a previous run of this same program.
    size_t nLimit = nOffset + nMatches;
    if (GetMutex(100)) {
      if (gs_bEmpty) {
        for (size_t i = 0; i < MAX_BUFS; i++) {
          if ((gs_pDisplayBufs[i].nMatch >= nOffset) &&
              (gs_pDisplayBufs[i].nMatch < nLimit)) {
            gs_pDisplayBufs[i].nIndex = 0xDEADDEAD;
          }
        }
      }
      else {
        size_t i = gs_nWrite;
        while (i != gs_nRead) {
          if ((gs_pDisplayBufs[i].nMatch >= nOffset) &&
              (gs_pDisplayBufs[i].nMatch < nLimit)) {
            gs_pDisplayBufs[i].nIndex = 0xDEADDEAD;
          }
          i++;
          if (i == MAX_BUFS) i = 0;
        }
      }
      ReleaseMutex(g_hMutex);
    }
  }
}

// Process a single message
void DoMessage(MatchContext nCtx, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  if (NULL == g_pMatches)
    return;
  // Some of the COM stuff we do may allow events to run again; do not
  // attempt more processing recursively.
  static DWORD g_dwActive = 0;
  DWORD dwCurrent = GetCurrentThreadId();
  if (0 != InterlockedCompareExchange((PVOID*)&g_dwActive, (PVOID)dwCurrent, 0))
    return;
  BOOL bNotify = FALSE;
  for (size_t i = 0; i < g_nMatches; i++) {
    if (DoMatch(g_pMatches+i, nCtx, hWnd, nMsg, wParam, lParam)) {
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
  InterlockedCompareExchange((PVOID*)&g_dwActive, 0, (PVOID)dwCurrent);
}

// Windows message hook callback
LRESULT CALLBACK CallWndHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if ((nCode >= 0) && !g_bNoCallWndProc) {
    PCWPSTRUCT pParams = (PCWPSTRUCT)lParam;
    DoMessage(CONTEXT_WNDPROC, pParams->hwnd, 
              pParams->message, pParams->wParam, pParams->lParam);
  }
  return CallNextHookEx(gs_hWndHook, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode >= 0) {
    PMSG pMsg = (PMSG)lParam;
    DoMessage(CONTEXT_GETMSG, pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
  }
  return CallNextHookEx(gs_hMsgHook, nCode, wParam, lParam);
}

/*** Patching of Windows functions via Import Address Table ***/

BOOL PatchFunction(LPCSTR szModule, LPCSTR szFunction, LPCSTR szCallModule,
                   PROC pnfunc, PROC **ppiat, PROC *porig)
{
  HMODULE hfmod = GetModuleHandle(szModule);
  if (NULL == hfmod) return FALSE;
  PROC pfunc = GetProcAddress(hfmod, szFunction);
  if (NULL == pfunc) return FALSE;

  HMODULE hmod;
  if (NULL == szCallModule)
    hmod = GetModuleHandle(NULL);
  else {
    hmod = GetModuleHandle(szCallModule);
    if (NULL == hmod) {
      if (NULL == strchr(szCallModule, '.')) // Full name to allow loading.
        return FALSE;
      hmod = LoadLibrary(szCallModule);
      if (NULL == hmod) return FALSE;
    }
  }
  PIMAGE_DOS_HEADER pdos = (PIMAGE_DOS_HEADER)hmod;
  if (pdos->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;
  PIMAGE_NT_HEADERS pnt = (PIMAGE_NT_HEADERS)((DWORD)hmod + pdos->e_lfanew);
  if (pnt->Signature != IMAGE_NT_SIGNATURE) return FALSE;
  PIMAGE_DATA_DIRECTORY pimp = pnt->OptionalHeader.DataDirectory 
    + IMAGE_DIRECTORY_ENTRY_IMPORT;
  PIMAGE_DATA_DIRECTORY piat= pnt->OptionalHeader.DataDirectory 
    + IMAGE_DIRECTORY_ENTRY_IAT;
  
  PIMAGE_IMPORT_DESCRIPTOR pid = (PIMAGE_IMPORT_DESCRIPTOR)
    ((DWORD)hmod + pimp->VirtualAddress);
  while (TRUE) {
    if (NULL == pid->Characteristics) return FALSE;
    LPCSTR pmod = (LPCSTR)((DWORD)hmod + pid->Name);
    if (!_stricmp(pmod, szModule)) break;
    pid++;
  }
  PIMAGE_THUNK_DATA pdata = (PIMAGE_THUNK_DATA)((DWORD)hmod + pid->OriginalFirstThunk);
  PROC *paddr = (PROC *)((DWORD)hmod + pid->FirstThunk);
  while (TRUE) {
    if (NULL == pdata->u1.Function) return FALSE;
    if ((pdata->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0) {
      PIMAGE_IMPORT_BY_NAME pnam = (PIMAGE_IMPORT_BY_NAME)
        ((DWORD)hmod + (DWORD)pdata->u1.AddressOfData);
      if (!strcmp((LPCSTR)pnam->Name, szFunction)) break;
      pdata++;
      paddr++;
    }
  }
  if (pfunc != *paddr) return FALSE;
  *ppiat = paddr;
  *porig = *paddr;
  DWORD dwOldProt;
  if (VirtualProtect(paddr, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
    *paddr = pnfunc;
    VirtualProtect(paddr, sizeof(PROC), dwOldProt, &dwOldProt);
  }
  return TRUE;
}

PROC *TextOutIAT = NULL;
BOOL (WINAPI *OrigTextOut)(HDC, int, int, LPCSTR, int) = NULL;
BOOL WINAPI PatchTextOut(HDC hDC, int nX, int nY, LPSTR lpString, int nCount)
{
  BOOL bResult = (*OrigTextOut)(hDC, nX, nY, lpString, nCount);
  char ch = lpString[nCount];
  if ('\0' != ch)
    lpString[nCount] = '\0';
  DoMessage(CONTEXT_PATCH, WindowFromDC(hDC),
            PATCH_TEXTOUT, (WPARAM)MAKELONG(nX,nY), (LPARAM)lpString);
  if ('\0' != ch)
    lpString[nCount] = ch;
  return bResult;
}

PROC *DrawTextIAT = NULL;
int (WINAPI *OrigDrawText)(HDC, LPCSTR, int, LPRECT, UINT) = NULL;
int WINAPI PatchDrawText(HDC hDC, LPSTR lpString, int nCount, 
                         LPRECT lpRect, UINT uFormat)
{
  int nResult = (*OrigDrawText)(hDC, lpString, nCount, lpRect, uFormat);
  char ch = lpString[nCount];
  if ('\0' != ch)
    lpString[nCount] = '\0';
  DoMessage(CONTEXT_PATCH, WindowFromDC(hDC),
            PATCH_DRAWTEXT, (WPARAM)MAKELONG(lpRect->left,lpRect->top), (LPARAM)lpString);
  if ('\0' != ch)
    lpString[nCount] = ch;
  return nResult;
}

PROC *ExtTextOutIAT = NULL;
BOOL (WINAPI *OrigExtTextOut)(HDC, int, int, UINT, CONST RECT *,LPCSTR, UINT, CONST INT *) = NULL;

PROC *TabbedTextOutIAT = NULL;
BOOL (WINAPI *OrigTabbedTextOut)(HDC, int, int, LPCSTR, int, int, LPINT, int) = NULL;

PROC *BitBltIAT = NULL;
BOOL (WINAPI *OrigBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD) = NULL;

PROC *StretchBltIAT = NULL;
BOOL (WINAPI *OrigStretchBlt)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD) = NULL;

HDC g_hdcTextImage = NULL;
LPSTR g_szTextImage = NULL;
size_t g_nTextImage = 0;

void PatchTIText(HDC hDC, int nX, int nY, LPCSTR lpString, int nCount)
{
  char ch = lpString[nCount];
  if ('\0' != ch)
    ((LPSTR)lpString)[nCount] = '\0';
  HWND hWnd = WindowFromDC(hDC);
  if (NULL != hWnd)
    DoMessage(CONTEXT_PATCH, hWnd,
              PATCH_TEXTIMAGE, (WPARAM)MAKELONG(nX,nY), (LPARAM)lpString);
  else {
    // Remember text drawn to off screen bitmap.  Then later see if
    // that bitmap is drawn to the screen.  If so, send event as
    // though text were directly.
    g_hdcTextImage = hDC;
    if (g_nTextImage < (size_t)nCount + 1) {
      g_nTextImage = nCount + 1;
      g_szTextImage = (LPSTR)realloc(g_szTextImage, g_nTextImage);
    }
    memcpy(g_szTextImage, lpString, nCount + 1);
  }
  if ('\0' != ch)
    ((LPSTR)lpString)[nCount] = ch;
}

BOOL WINAPI PatchTITextOut(HDC hDC, int nX, int nY, LPCSTR lpString, int nCount)
{
  BOOL bResult = (*OrigTextOut)(hDC, nX, nY, lpString, nCount);
  PatchTIText(hDC, nX, nY, lpString, nCount);
  return bResult;
}

BOOL WINAPI PatchTIExtTextOut(HDC hDC, int nX, int nY, UINT fuOptions,
                              CONST RECT *lpRect, LPCSTR lpString, UINT nCount,
                              CONST INT *lpDx)
{
  BOOL bResult = (*OrigExtTextOut)(hDC, nX, nY, fuOptions, lpRect, 
                                   lpString, nCount, lpDx);
  PatchTIText(hDC, nX, nY, lpString, nCount);
  return bResult;
}

BOOL WINAPI PatchTITabbedTextOut(HDC hDC, int nX, int nY, LPCSTR lpString, int nCount,
                                 int nTabPositions, LPINT lpnTabStopPositions,
                                 int nTabOrigin)
{
  BOOL bResult = (*OrigTabbedTextOut)(hDC, nX, nY, lpString, nCount, 
                                      nTabPositions, lpnTabStopPositions, nTabOrigin);
  PatchTIText(hDC, nX, nY, lpString, nCount);
  return bResult;
}

int WINAPI PatchTIDrawText(HDC hDC, LPCSTR lpString, int nCount, 
                           LPRECT lpRect, UINT uFormat)
{
  int nResult = (*OrigDrawText)(hDC, lpString, nCount, lpRect, uFormat);
  PatchTIText(hDC, lpRect->left, lpRect->top, lpString, nCount);
  return nResult;
}

BOOL WINAPI PatchTIBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
                          HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
  BOOL bResult = OrigBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, 
                            hdcSrc, nXSrc, nYSrc, dwRop);
  if (hdcSrc == g_hdcTextImage) {
    HWND hWnd = WindowFromDC(hdcDest);
    if (NULL != hWnd) {
      DoMessage(CONTEXT_PATCH, hWnd,
                PATCH_TEXTIMAGE, (WPARAM)MAKELONG(nXDest,nYDest), (LPARAM)g_szTextImage);
      g_hdcTextImage = NULL;
    }
  }
  return bResult;
}

BOOL WINAPI PatchTIStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidthDest, int nHeightDest, 
                              HDC hdcSrc, int nXSrc, int nYSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop)
{
  BOOL bResult = OrigStretchBlt(hdcDest, nXDest, nYDest, nWidthDest, nHeightDest, 
                                hdcSrc, nXSrc, nYSrc, nWidthSrc, nHeightSrc, dwRop);
  if (hdcSrc == g_hdcTextImage) {
    HWND hWnd = WindowFromDC(hdcDest);
    if (NULL != hWnd) {
      DoMessage(CONTEXT_PATCH, hWnd,
                PATCH_TEXTIMAGE, (WPARAM)MAKELONG(nXDest,nYDest), (LPARAM)g_szTextImage);
      g_hdcTextImage = NULL;
    }
  }
  return bResult;
}

PROC *StretchDIBitsIAT = NULL;
int (WINAPI *OrigStretchDIBits)(HDC, int, int, int, int, int, int, int, int, 
                                CONST VOID *, CONST BITMAPINFO *, UINT, DWORD) = NULL;
int WINAPI PatchPowerDVDStretchDIBits(HDC, int, int, int, int, int, int, int, int, 
                                      CONST VOID *, CONST BITMAPINFO *, UINT, DWORD);

PROC *LoadImageIAT = NULL;
HANDLE (WINAPI *OrigLoadImage)(HINSTANCE, LPCSTR, UINT, int, int, UINT) = NULL;
HANDLE WINAPI PatchWinDVDLoadImage(HINSTANCE, LPCSTR, UINT, int, int, UINT);

void InstallPatches()
{
  if (NULL == g_pMatches)
    return;
  LPCSTR szDefMod = NULL;
  for (size_t i = 0; i < g_nMatches; i++) {
    if (MATCH_PATCH == g_pMatches[i].pMatches[0].nCode) {
      switch (g_pMatches[i].pMatches[0].dwVal) {
      case PATCH_TEXTOUT:
        if (NULL == TextOutIAT) {
          if (!PatchFunction("GDI32.DLL", "TextOutA", szDefMod,
                             (PROC)PatchTextOut, &TextOutIAT, (PROC*)&OrigTextOut))
            PatchFunction("GDI32.DLL", "TextOutA", "MFC42",
                          (PROC)PatchTextOut, &TextOutIAT, (PROC*)&OrigTextOut);
        }
        break;
      case PATCH_DRAWTEXT:
        if (NULL == DrawTextIAT) {
          if (!PatchFunction("GDI32.DLL", "DrawTextA", szDefMod,
                             (PROC)PatchDrawText, &DrawTextIAT, (PROC*)&OrigDrawText))
            PatchFunction("GDI32.DLL", "DrawTextA", "MFC42",
                          (PROC)PatchDrawText, &DrawTextIAT, (PROC*)&OrigDrawText);
        }
        break;
      case PATCH_TEXTIMAGE:
        if (!_stricmp(g_pMatches->szModule, "ATI"))
          szDefMod = "ATICORE.DLL";
        if (NULL == TextOutIAT) {
          if (!PatchFunction("GDI32.DLL", "TextOutA", szDefMod,
                             (PROC)PatchTITextOut, &TextOutIAT, (PROC*)&OrigTextOut))
            PatchFunction("GDI32.DLL", "TextOutA", "MFC42",
                          (PROC)PatchTITextOut, &TextOutIAT, (PROC*)&OrigTextOut);
        }
        if (NULL == ExtTextOutIAT) {
          PatchFunction("GDI32.DLL", "ExtTextOutA", szDefMod,
                        (PROC)PatchTIExtTextOut, &ExtTextOutIAT, (PROC*)&OrigExtTextOut);
        }
        if (NULL == TabbedTextOutIAT) {
          PatchFunction("GDI32.DLL", "TabbedTextOutA", szDefMod,
                        (PROC)PatchTITabbedTextOut, &TabbedTextOutIAT, (PROC*)&OrigTabbedTextOut);
        }
        if (NULL == DrawTextIAT) {
          if (!PatchFunction("GDI32.DLL", "DrawTextA", szDefMod,
                             (PROC)PatchTIDrawText, &DrawTextIAT, (PROC*)&OrigDrawText))
            PatchFunction("GDI32.DLL", "DrawTextA", "MFC42",
                          (PROC)PatchTIDrawText, &DrawTextIAT, (PROC*)&OrigDrawText);
        }
        if (NULL == BitBltIAT) {
          PatchFunction("GDI32.DLL", "BitBlt", szDefMod,
                        (PROC)PatchTIBitBlt, &BitBltIAT, (PROC*)&OrigBitBlt);
        }
        if (NULL == StretchBltIAT) {
          PatchFunction("GDI32.DLL", "StretchBlt", szDefMod,
                        (PROC)PatchTIStretchBlt, &StretchBltIAT, (PROC*)&OrigStretchBlt);
        }
        break;
      case PATCH_POWERDVD_STRETCHDIBITS:
        if (NULL == StretchDIBitsIAT) {
          PatchFunction("GDI32.DLL", "StretchDIBits", szDefMod,
                        (PROC)PatchPowerDVDStretchDIBits, &StretchDIBitsIAT, (PROC*)&OrigStretchDIBits);
        }
        break;
      case PATCH_WINDVD_GETIMAGE:
        if (NULL == LoadImageIAT) {
          for (int i = 1; i <= 3; i++) {
            LPCSTR szMod;
            switch (i) {
            case 1:
              szMod = szDefMod; // Version 3 (WinDVDi)
              break;
            case 2:
              szMod = "IVIPlayerX.ocx"; // Version 4 (WinDVD4)
              break;
            case 3:
              szMod = "pbPlyr.ocx"; // Platinum (WinDVD4PR)
              break;
            }
            if (PatchFunction("USER32.DLL", "LoadImageA", szMod,
                              (PROC)PatchWinDVDLoadImage, &LoadImageIAT, (PROC*)&OrigLoadImage))
              break;
          }
        }
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
  if (NULL != DrawTextIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(DrawTextIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *DrawTextIAT = (PROC)*OrigDrawText;
      VirtualProtect(DrawTextIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
  if (NULL != BitBltIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(BitBltIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *BitBltIAT = (PROC)*OrigBitBlt;
      VirtualProtect(BitBltIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
  if (NULL != StretchBltIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(StretchBltIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *StretchBltIAT = (PROC)*OrigStretchBlt;
      VirtualProtect(StretchBltIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
  if (NULL != StretchDIBitsIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(StretchDIBitsIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *StretchDIBitsIAT = (PROC)*OrigStretchDIBits;
      VirtualProtect(StretchDIBitsIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
  if (NULL != LoadImageIAT) {
    DWORD dwOldProt;
    if (VirtualProtect(LoadImageIAT, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt)) {
      *LoadImageIAT = (PROC)*OrigLoadImage;
      VirtualProtect(LoadImageIAT, sizeof(PROC), dwOldProt, &dwOldProt);
    }
  }
}

/*** Wrapped COM object interface ***/

// We do not link to mediaspy.dll, since that would clutter up every process.
typedef HRESULT (STDAPICALLTYPE * LPFNGETOBJECT)(REFCLSID, UINT, REFIID, PVOID*);
LPFNGETOBJECT MediaSpyGetCurrentObject = NULL;
typedef HRESULT (STDAPICALLTYPE * LPFNCONVERTOLE)(LPOLESTR, LPSTR, size_t,BOOL);
LPFNCONVERTOLE MediaSpyConvertOle = NULL;
static REFTIME g_fgDuration, g_fgPosition;
static char g_szfgFileName[MAX_PATH];
static DVD_DOMAIN g_dvdDomain;
static DVD_PLAYBACK_LOCATION2 g_dvdLocation;
static DVD_HMSF_TIMECODE g_dvdTotalTime;
static FILTER_STATE g_sourceFilterState;
static long g_lVideoWindowLeft, g_lVideoWindowTop, 
  g_lVideoWindowWidth, g_lVideoWindowHeight;
static long g_lVideoDestinationLeft, g_lVideoDestinationTop, 
  g_lVideoDestinationWidth, g_lVideoDestinationHeight;
static long g_lVideoSourceLeft, g_lVideoSourceTop, 
  g_lVideoSourceWidth, g_lVideoSourceHeight;
static long g_lTVChannel;

HRESULT GetVideoPosition(IFilterGraph *pFG)
{
  HRESULT hr;
  IVideoWindow *pVW = NULL;
  hr = pFG->QueryInterface(IID_IVideoWindow, (void**)&pVW);
  if (SUCCEEDED(hr)) {
    hr = pVW->GetWindowPosition(&g_lVideoWindowLeft, &g_lVideoWindowTop,
                                &g_lVideoWindowWidth, &g_lVideoWindowHeight);
    pVW->Release();
  }
  if (FAILED(hr)) {
    g_lVideoWindowLeft = g_lVideoWindowTop = 
      g_lVideoWindowWidth = g_lVideoWindowHeight = -1;
  }
  IBasicVideo *pBV = NULL;
  hr = pFG->QueryInterface(IID_IBasicVideo, (void**)&pBV);
  if (SUCCEEDED(hr)) {
    hr = pBV->GetDestinationPosition(&g_lVideoDestinationLeft, &g_lVideoDestinationTop,
                                     &g_lVideoDestinationWidth, &g_lVideoDestinationHeight);
    if (SUCCEEDED(hr))
      hr = pBV->GetSourcePosition(&g_lVideoSourceLeft, &g_lVideoSourceTop,
                                  &g_lVideoSourceWidth, &g_lVideoSourceHeight);
    pBV->Release();
  }  
  if (FAILED(hr)) {
    g_lVideoDestinationLeft = g_lVideoDestinationTop = 
      g_lVideoDestinationWidth = g_lVideoDestinationHeight = -1;
    g_lVideoSourceLeft = g_lVideoSourceTop = 
      g_lVideoSourceWidth = g_lVideoSourceHeight = -1;
  }
  return hr;
}

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
      // that seems complete (has a file source).
      UINT n = 0;
      while (TRUE) {
        IFilterGraph *pFG = NULL;
        HRESULT hr = MediaSpyGetCurrentObject(CLSID_FilterGraph, n,
                                              IID_IFilterGraph, (void**)&pFG);
        if (S_OK != hr) break;  // Including S_FALSE for none.
        IMediaPosition *pMP = NULL;
        hr = pFG->QueryInterface(IID_IMediaPosition, (void**)&pMP);
        if (SUCCEEDED(hr)) {
          hr = pMP->get_Duration(&g_fgDuration);
          if (SUCCEEDED(hr))
            hr = pMP->get_CurrentPosition(&g_fgPosition);
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
                                          g_szfgFileName, sizeof(g_szfgFileName),
                                          TRUE);
                else {
                  // The filter may not support getting the filename,
                  // but the graph builder's render method will have
                  // given it the file's name.
                  FILTER_INFO finfo;
                  hr = pBF->QueryFilterInfo(&finfo);
                  if (SUCCEEDED(hr)) {
                    hr = MediaSpyConvertOle(finfo.achName, 
                                            g_szfgFileName, sizeof(g_szfgFileName),
                                            FALSE);
                    if (NULL != finfo.pGraph)
                      finfo.pGraph->Release();
                  }
                }
                bFound = TRUE;
                hr = pBF->GetState(0, &g_sourceFilterState);
              }
            }
            // Could look for a file sink as well to indicate that we are capturing.
            pBF->Release();
          }
          pEF->Release();
        }
        if (bFound)
          hr = GetVideoPosition(pFG);
        pFG->Release();
        if (bFound) return TRUE;
        n++;
      }
      return FALSE;
    }
  case MS_DVD_NAVIGATOR:
    {
      IDvdInfo2 *pDVD = NULL;
      HRESULT hr = MediaSpyGetCurrentObject(CLSID_DVDNavigator, 0,
                                            IID_IDvdInfo2, (void**)&pDVD);
      if (S_OK != hr) return FALSE; // Including S_FALSE for none.
      hr = pDVD->GetCurrentDomain(&g_dvdDomain);
      if (SUCCEEDED(hr)) {
        if (DVD_DOMAIN_Title == g_dvdDomain) {
          hr = pDVD->GetCurrentLocation(&g_dvdLocation);
          if (SUCCEEDED(hr)) {
            hr = pDVD->GetTotalTitleTime(&g_dvdTotalTime, NULL);
            if (VFW_S_DVD_NON_ONE_SEQUENTIAL == hr) {
              (ULONG&)g_dvdTotalTime = 0xFFFFFFFF;
              hr = S_OK;
            }
          }
        }
        else {
          g_dvdLocation.TitleNum = 0xFFFFFFFF;
          g_dvdLocation.ChapterNum = 0xFFFFFFFF;
          (ULONG&)g_dvdLocation.TimeCode = 0xFFFFFFFF;
          (ULONG&)g_dvdTotalTime = 0xFFFFFFFF;
        }
      }
      IBaseFilter *pBF = NULL;
      hr = pDVD->QueryInterface(IID_IBaseFilter, (void**)&pBF);
      pDVD->Release();
      if (SUCCEEDED(hr)) {
        hr = pBF->GetState(0, &g_sourceFilterState); // OK if in transition or no cueing.
        FILTER_INFO finfo;
        hr = pBF->QueryFilterInfo(&finfo);
        pBF->Release();
        if (SUCCEEDED(hr) && (NULL != finfo.pGraph)) {
          hr = GetVideoPosition(finfo.pGraph);
          finfo.pGraph->Release();
        }
      }
      return SUCCEEDED(hr);
    }
  case MS_TV_TUNER:
    {
      // Try to find a TV tuner in running graphs.
      UINT n = 0;
      while (TRUE) {
        IFilterGraph *pFG = NULL;
        HRESULT hr = MediaSpyGetCurrentObject(CLSID_FilterGraph, n,
                                              IID_IFilterGraph, (void**)&pFG);
        if (S_OK != hr) break;  // Including S_FALSE for none.
        IEnumFilters *pEF = NULL;
        BOOL bFound = FALSE;
        hr = pFG->EnumFilters(&pEF);
        if (SUCCEEDED(hr)) {
          while (!bFound) {
            IBaseFilter *pBF = NULL;
            hr = pEF->Next(1, &pBF, NULL);
            if (S_OK != hr) break;
            IAMTVTuner *pTV = NULL;
            hr = pBF->QueryInterface(IID_IAMTVTuner, (void**)&pTV);
            if (SUCCEEDED(hr)) {
              long lJunk;
              hr = pTV->get_Channel(&g_lTVChannel, &lJunk, &lJunk);
              if (SUCCEEDED(hr))
                bFound = TRUE;
              pTV->Release();
            }
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

void FormatTIMECODE(DVD_HMSF_TIMECODE tc, LPSTR szBuf, size_t nSize)
{
  sprintf(szBuf, "%02d:%02d:%02d", tc.bHours, tc.bMinutes, tc.bSeconds);
}

void ExtractMediaSpy(UINT nField, LPSTR szBuf, size_t nSize)
{
  switch (nField) {
  case MS_FG_DURATION:
    FormatREFTIME(g_fgDuration, szBuf, nSize);
    break;
  case MS_FG_POSITION:
    FormatREFTIME(g_fgPosition, szBuf, nSize);
    break;
  case MS_FG_FILENAME:
    strncpy(szBuf, g_szfgFileName, nSize);
    break;
  case MS_FG_STATE:
    switch (g_sourceFilterState) {
    case State_Stopped:
      strncpy(szBuf, "Stopped", nSize);
      break;
    case State_Paused:
      strncpy(szBuf, "Paused", nSize);
      break;
    case State_Running:
      strncpy(szBuf, "Playing", nSize);
      break;
    }
    break;
  case MS_FG_VIDEO_POSITION:
    sprintf(szBuf, "%d,%d,%d,%d;%d,%d,%d,%d;%d,%d,%d,%d",
            g_lVideoWindowWidth, g_lVideoWindowHeight, 
            g_lVideoWindowLeft, g_lVideoWindowTop,
            g_lVideoDestinationWidth, g_lVideoDestinationHeight, 
            g_lVideoDestinationLeft, g_lVideoDestinationTop,
            g_lVideoSourceWidth, g_lVideoSourceHeight, 
            g_lVideoSourceLeft, g_lVideoSourceTop);
    break;
  case MS_DVD_DOMAIN:
    switch (g_dvdDomain) {
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
    if (g_dvdLocation.TitleNum == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      sprintf(szBuf, "%02d", g_dvdLocation.TitleNum);
    break;
  case MS_DVD_CHAPTER:
    if (g_dvdLocation.ChapterNum == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      sprintf(szBuf, "%02d", g_dvdLocation.ChapterNum);
    break;
  case MS_DVD_TOTAL:
    if ((ULONG&)g_dvdTotalTime == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      FormatTIMECODE(g_dvdTotalTime, szBuf, nSize);
    break;
  case MS_DVD_TIME:
    if ((ULONG&)g_dvdLocation.TimeCode == 0xFFFFFFFF)
      szBuf[0] = '\0';
    else
      FormatTIMECODE(g_dvdLocation.TimeCode, szBuf, nSize);
    break;
  case MS_TV_CHANNEL:
    sprintf(szBuf, "%d", g_lTVChannel);
    break;
  }
}

/*** Skin based decoding ***/

inline int operator==(const POINT& pt1, const POINT& pt2)
{
  return ((pt1.x == pt2.x) && (pt1.y == pt2.y));
}

BOOL g_skinDVDRegistryLoaded = FALSE;
POINT g_skinDVDStatus,
  g_skinDVDHour10, g_skinDVDHour1,
  g_skinDVDMinute10, g_skinDVDMinute1, 
  g_skinDVDSecond10, g_skinDVDSecond1, 
  g_skinDVDTitle100, g_skinDVDTitle10, g_skinDVDTitle1, 
  g_skinDVDChapter100, g_skinDVDChapter10, g_skinDVDChapter1;

char g_skinDVDTime[9] = {' ', ' ', ':', ' ', ' ', ':', ' ', ' ', '\0'};
char g_skinDVDTitle[4] = {' ', ' ', ' ', '\0'};
char g_skinDVDChapter[4] = {' ', ' ', ' ', '\0'};

POINT SkinDVDRegistryPoint(HKEY hkey, LPCSTR szName)
{
  POINT ptResult = { -1, -1 };

  char szBuf[32];
  DWORD dwType, nLen;
  nLen = sizeof(szBuf);
  if ((ERROR_SUCCESS != RegQueryValueEx(hkey, szName, NULL, 
                                        &dwType, (LPBYTE)szBuf, &nLen)) ||
      (REG_SZ != dwType))
    return ptResult;

  LPSTR psz = strchr(szBuf, ',');
  if (NULL == psz) return ptResult;
  *psz++ = '\0';
  ptResult.x = atol(szBuf);
  ptResult.y = atol(psz);

  return ptResult;
}

/*** PowerDVD ***/

POINT g_PDVDStatusSize, g_PDVDTimeDigitSize, g_PDVDLocationDigitSize;

const char *g_PDVDStatusNames[] = {
  "", "Playing", "Paused", "Still", "Stopped", "Eject",
  "FF1/8x", "FF1/4x", "FF1/2x", "FF1.5x", "FF2x", "FF2.5x", "FF4x", "FF8x", "FF", 
  "REW1/8x", "REW1/4x", "REW1/2x", "REW1.5x", "REW2x", "REW2.5x", "REW4x", "REW8x", "REW"
};
const char *g_PDVDDigits = " 0123456789";

void PDVDLoadRegistry()
{
  g_skinDVDRegistryLoaded = TRUE;
  
  // Hopefully it is okay to load ADVAPI32 into all these processes.
  HKEY hkey;
  if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, 
                                  "Software\\Girder3\\HardPlugins\\DVDSpy\\PowerDVD", 
                                  &hkey))
    return;
  
  g_PDVDStatusSize = SkinDVDRegistryPoint(hkey, "Status_SIZE");
  g_PDVDTimeDigitSize = SkinDVDRegistryPoint(hkey, "TimeDigit_SIZE");
  g_PDVDLocationDigitSize = SkinDVDRegistryPoint(hkey, "LocationDigit_SIZE");
  g_skinDVDStatus = SkinDVDRegistryPoint(hkey, "PlayStatus_POSITION");
  g_skinDVDHour10 = SkinDVDRegistryPoint(hkey, "Hour_M_POSITION");
  g_skinDVDHour1 = SkinDVDRegistryPoint(hkey, "Hour_L_POSITION");
  g_skinDVDMinute10 = SkinDVDRegistryPoint(hkey, "Minute_M_POSITION");
  g_skinDVDMinute1 = SkinDVDRegistryPoint(hkey, "Minute_L_POSITION");
  g_skinDVDSecond10 = SkinDVDRegistryPoint(hkey, "Second_M_POSITION");
  g_skinDVDSecond1 = SkinDVDRegistryPoint(hkey, "Second_L_POSITION");
  g_skinDVDTitle100 = SkinDVDRegistryPoint(hkey, "Title_H_POSITION");
  g_skinDVDTitle10 = SkinDVDRegistryPoint(hkey, "Title_M_POSITION");
  g_skinDVDTitle1 = SkinDVDRegistryPoint(hkey, "Title_L_POSITION");
  g_skinDVDChapter100 = SkinDVDRegistryPoint(hkey, "Chapter_H_POSITION");
  g_skinDVDChapter10 = SkinDVDRegistryPoint(hkey, "Chapter_M_POSITION");
  g_skinDVDChapter1 = SkinDVDRegistryPoint(hkey, "Chapter_L_POSITION");

  RegCloseKey(hkey);
}

int WINAPI PatchPowerDVDStretchDIBits
(HDC hDC, int XDest, int YDest, int nDestWidth, int nDestHeight, 
 int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, 
 CONST VOID * lpbits, CONST BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop)
{
  int nResult = (*OrigStretchDIBits)(hDC, XDest, YDest, nDestWidth, nDestHeight,
                                     XSrc, YSrc, nSrcWidth, nSrcHeight,
                                     lpbits, lpBitsInfo, iUsage, dwRop);
  if (GDI_ERROR == nResult) return nResult;

  if (!((nSrcWidth > 0) &&
        (nDestWidth == nSrcWidth) &&
        (nDestHeight == nSrcHeight) &&
        (lpBitsInfo->bmiHeader.biSize == sizeof(BITMAPINFOHEADER)) &&
        ((lpBitsInfo->bmiHeader.biWidth % nSrcWidth) == 0) &&
        ((XSrc % nSrcWidth) == 0) &&
        (YSrc == (lpBitsInfo->bmiHeader.biHeight - nSrcHeight)) &&
        (iUsage == 0) &&
        (dwRop == SRCCOPY))) {
#ifdef _TRACE
    {
      char szBuf[1024];
      sprintf(szBuf, "PowerDVD: StretchDIBits %X(%X) %d,%d %dx%d <- %d,%d %dx%d %X [%d] %dx%d %d %X\n",
              hDC, WindowFromDC(hDC), 
              XDest, YDest, nDestWidth, nDestHeight,
              XSrc, YSrc, nSrcWidth, nSrcHeight,
              lpbits, lpBitsInfo->bmiHeader.biSize, lpBitsInfo->bmiHeader.biWidth, lpBitsInfo->bmiHeader.biHeight, 
              iUsage, dwRop);
      OutputDebugString(szBuf);
    }
#endif
    return nResult;
  }

  if (!g_skinDVDRegistryLoaded)
    PDVDLoadRegistry();

  HWND hWnd = WindowFromDC(hDC);
  POINT ptSize = { nDestWidth, nDestHeight }, ptPos = { XDest, YDest };
  size_t nOff = (XSrc / nSrcWidth), nLen = (lpBitsInfo->bmiHeader.biWidth / nSrcWidth);

#ifdef _TRACE
  {
    char szBuf[1024];
    sprintf(szBuf, "PowerDVD: StretchDIBits %X, %d/%d %dx%d %d,%d\n", 
            hWnd, nOff, nLen, ptSize.x, ptSize.y, ptPos.x, ptPos.y);
    OutputDebugString(szBuf);
  }
#endif

  if ((ptSize == g_PDVDStatusSize) &&
      (nLen >= countof(g_PDVDStatusNames)) &&
      (ptPos == g_skinDVDStatus)) {
    DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
              (WPARAM)SKINDVD_STATUS, (LPARAM)g_PDVDStatusNames[nOff]);
  }
  else if ((ptSize == g_PDVDTimeDigitSize) &&
           (nLen >= strlen(g_PDVDDigits))) {
    char cDigit = g_PDVDDigits[nOff];
    if (ptPos == g_skinDVDHour10) {
      g_skinDVDTime[0] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
    else if (ptPos == g_skinDVDHour1) {
      g_skinDVDTime[1] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
    else if (ptPos == g_skinDVDMinute10) {
      g_skinDVDTime[3] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
    else if (ptPos == g_skinDVDMinute1) {
      g_skinDVDTime[4] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
    else if (ptPos == g_skinDVDSecond10) {
      g_skinDVDTime[6] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
    else if (ptPos == g_skinDVDSecond1) {
      g_skinDVDTime[7] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TIME, (LPARAM)g_skinDVDTime);
    }
  }
  else if ((ptSize == g_PDVDLocationDigitSize) &&
           (nLen >= strlen(g_PDVDDigits))) {
    char cDigit = g_PDVDDigits[nOff];
    if (ptPos == g_skinDVDTitle100) {
      g_skinDVDTitle[0] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TITLE, (LPARAM)g_skinDVDTitle);
    }
    else if (ptPos == g_skinDVDTitle10) {
      g_skinDVDTitle[1] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TITLE, (LPARAM)g_skinDVDTitle);
    }
    else if (ptPos == g_skinDVDTitle1) {
      g_skinDVDTitle[2] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_TITLE, (LPARAM)g_skinDVDTitle);
    }
    else if (ptPos == g_skinDVDChapter100) {
      g_skinDVDChapter[0] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_CHAPTER, (LPARAM)g_skinDVDChapter);
    }
    else if (ptPos == g_skinDVDChapter10) {
      g_skinDVDChapter[1] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_CHAPTER, (LPARAM)g_skinDVDChapter);
    }
    else if (ptPos == g_skinDVDChapter1) {
      g_skinDVDChapter[2] = cDigit;
      DoMessage(CONTEXT_PATCH, hWnd, PATCH_POWERDVD_STRETCHDIBITS, 
                (WPARAM)SKINDVD_CHAPTER, (LPARAM)g_skinDVDChapter);
    }
  }

  return nResult;
}

/*** WinDVD ***/

struct WinDVDImage
{
  const char *szName;
  const char *szValue;
  HBITMAP hBitmap;
} g_WinDVDImages[] = {
  { NULL, " ", },
  { "Number_0", "0" },
  { "Number_1", "1" },
  { "Number_2", "2" },
  { "Number_3", "3" },
  { "Number_4", "4" },
  { "Number_5", "5" },
  { "Number_6", "6" },
  { "Number_7", "7" },
  { "Number_8", "8" },
  { "Number_9", "9" },
  { "Playback_Status_Fast_Backward", "Rewind" },
  { "Playback_Status_Fast_Forward", "FF" },
  { "Playback_Status_Pause", "Paused" },
  { "Playback_Status_Play", "Playing" },
  { "Playback_Status_Stop", "Stopped" },
};

WinDVDImage *g_pWinDVDStatus = NULL;

HANDLE WINAPI PatchWinDVDLoadImage(HINSTANCE hinst, LPCSTR lpszName, UINT uType,
                                   int cxDesired, int cyDesired, UINT fuLoad)
{
  HANDLE hResult = (*OrigLoadImage)(hinst, lpszName, uType, 
                                    cxDesired, cyDesired, fuLoad);
  if (NULL == hResult) return hResult;
                   
  if (!((IMAGE_BITMAP == uType) &&
        ((LR_CREATEDIBSECTION|LR_LOADFROMFILE) == 
         (fuLoad & (LR_CREATEDIBSECTION|LR_LOADFROMFILE)))))
    return hResult;

  char szName[MAX_PATH];
  LPSTR psz = strrchr(lpszName, '\\');
  if (NULL != psz)
    lpszName = psz+1;
  strcpy(szName, lpszName);
  psz = strrchr(szName, '.');
  if (NULL != psz)
    *psz = '\0';
  
#ifdef _TRACE
  {
    char szBuf[1024];
    sprintf(szBuf, "WinDVD: LoadImage %s => %X\n", szName, hResult);
    OutputDebugString(szBuf);
  }
#endif

  for (size_t i = 1; i < countof(g_WinDVDImages); i++) {
    if (!_stricmp(g_WinDVDImages[i].szName, szName)) {
      g_WinDVDImages[i].hBitmap = (HBITMAP)hResult;
      break;
    }
  }

  return hResult;
}

void WDVDLoadRegistry()
{
  g_skinDVDRegistryLoaded = TRUE;
  
  HKEY hkey;
  if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, 
                                  "Software\\Girder3\\HardPlugins\\DVDSpy\\WinDVD", 
                                  &hkey))
    return;
  
  g_skinDVDStatus = SkinDVDRegistryPoint(hkey, "Playback_Status_Display_Location");
  g_skinDVDHour10 = SkinDVDRegistryPoint(hkey, "Hour_10_Display_Location");
  g_skinDVDHour1 = SkinDVDRegistryPoint(hkey, "Hour_0_Display_Location");
  g_skinDVDMinute10 = SkinDVDRegistryPoint(hkey, "Minute_10_Display_Location");
  g_skinDVDMinute1 = SkinDVDRegistryPoint(hkey, "Minute_0_Display_Location");
  g_skinDVDSecond10 = SkinDVDRegistryPoint(hkey, "Second_10_Display_Location");
  g_skinDVDSecond1 = SkinDVDRegistryPoint(hkey, "Second_0_Display_Location");
  g_skinDVDChapter100 = SkinDVDRegistryPoint(hkey, "Chapter_100_Display_Location");
  g_skinDVDChapter10 = SkinDVDRegistryPoint(hkey, "Chapter_10_Display_Location");
  g_skinDVDChapter1 = SkinDVDRegistryPoint(hkey, "Chapter_0_Display_Location");

  RegCloseKey(hkey);
}

BOOL MatchWinDVDSetImage(HWND hWnd, UINT nType, HBITMAP hBitmap)
{
  RECT rcWnd, rcParent;
  GetWindowRect(hWnd, &rcWnd);
  GetWindowRect(GetParent(hWnd), &rcParent);
  POINT ptPos = { rcWnd.left - rcParent.left, rcWnd.top - rcParent.top };

  WinDVDImage *pImage = NULL;
  for (size_t i = 0; i < countof(g_WinDVDImages); i++) {
    if (g_WinDVDImages[i].hBitmap == hBitmap) {
      pImage = g_WinDVDImages + i;
      break;
    }
  }

#ifdef _TRACE
  {
    char szBuf[1024];
    sprintf(szBuf, "WinDVD: SetImage %X(%d,%d) %X(%s)\n", 
            hWnd, ptPos.x, ptPos.y, hBitmap, 
            (NULL != pImage) ? pImage->szName : "???");
    OutputDebugString(szBuf);
  }
#endif

  if (NULL == pImage)
    return FALSE;

  if (!g_skinDVDRegistryLoaded)
    WDVDLoadRegistry();

  switch (nType) {
  case SKINDVD_STATUS:
    if (ptPos == g_skinDVDStatus) {
      g_pWinDVDStatus = pImage;
      return TRUE;
    }
    break;
  case SKINDVD_TIME:
    if (ptPos == g_skinDVDHour10) {
      g_skinDVDTime[0] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDHour1) {
      g_skinDVDTime[1] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDMinute10) {
      g_skinDVDTime[3] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDMinute1) {
      g_skinDVDTime[4] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDSecond10) {
      g_skinDVDTime[6] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDSecond1) {
      g_skinDVDTime[7] = *pImage->szValue;
      return TRUE;
    }
    break;
  case SKINDVD_CHAPTER:
    if (ptPos == g_skinDVDChapter100) {
      g_skinDVDChapter[0] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDChapter10) {
      g_skinDVDChapter[1] = *pImage->szValue;
      return TRUE;
    }
    else if (ptPos == g_skinDVDChapter1) {
      g_skinDVDChapter[2] = *pImage->szValue;
      return TRUE;
    }
    break;
  }
  return FALSE;
}

void ExtractWinDVDSetImage(UINT nType, LPSTR szBuf, size_t nSize)
{
  switch (nType) {
  case SKINDVD_STATUS:
    if (NULL == g_pWinDVDStatus)
      *szBuf = '\0';
    else
      strncpy(szBuf, g_pWinDVDStatus->szValue, nSize);
    break;
  case SKINDVD_TIME:
    strncpy(szBuf, g_skinDVDTime, nSize);
    break;
  case SKINDVD_CHAPTER:
    strncpy(szBuf, g_skinDVDChapter, nSize);
    break;
  }
}

LPCSTR _stristr(LPCSTR str, LPCSTR key)
{
  char chrs[3];
  chrs[0] = chrs[1] = key[0];
  chrs[2] = '\0';
  _strupr(chrs);
  _strlwr(chrs+1);
  size_t keyl = strlen(key);
  while (TRUE) {
    LPCSTR nstr = strpbrk(str, chrs);
    if (NULL == nstr) return NULL;
    if (!_strnicmp(nstr, key, keyl)) return nstr;
    str = nstr + 1;
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
#ifdef _TRACE
    OutputDebugString("DisplaySpyHook.DLL loading.\n");
#endif
    g_hInst = hInstance;
    IndexMatches(FALSE);
    InstallPatches();
    break;
  case DLL_PROCESS_DETACH:
#ifdef _TRACE
    OutputDebugString("DisplaySpyHook.DLL unloading.\n");
#endif
    RemovePatches();
    if (NULL != g_pMatches)
      delete [] g_pMatches;
    if (NULL != g_hMutex)
      CloseHandle(g_hMutex);
    break;
  }
  return TRUE;
}

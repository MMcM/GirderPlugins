/*

  Nullsoft WASABI Source File License

  Copyright 1999-2001 Nullsoft, Inc.

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


  Brennan Underwood
  brennan@nullsoft.com

$Header$
*/

// Background window that receives changes from Winamp and forwards them to Girder.
// Also receives requests from Girder and executes them in Winamp.

// This could more straightforwardly be just a thread listening to a
// semaphore or a mailslot, but I am not altogether confident of the
// thread safety of the core and pledit interfaces.

#include <bfc/std.h>

#include "girder.h"
#include "ipc.h"
#include "girderwnd.h"
#include "resource.h"
#include "girderlogwnd.h"

#include <pledit/svc_pldir.h>
#include <pledit/playlist.h>
#include <pledit/editor.h>
#include <common/metatags.h>

#if 0
static void dumpCfgItem(CfgItem *cfg, int depth)
{
  if (NULL == cfg) return;

  char guidstr[nsGUID::GUID_STRLEN];
  char buf[1024];
  sprintf(buf, "%d %s %s\n", depth, 
          nsGUID::toChar(cfg->getGuid(), guidstr), cfg->getName());
  OutputDebugString(buf);
  int na = cfg->getNumAttributes();
  for (int i = 0; i < na; i++) {
    sprintf(buf, "  %s\n", cfg->enumAttribute(i));
    OutputDebugString(buf);
  }
  int nc = cfg->getNumChildren();
  for (i = 0; i < nc; i++) {
    dumpCfgItem(cfg->enumChild(i), depth+1);
  }
}
#endif

static inline CfgItem *getCfgItemForName(const char *guidstr, const char* name)
{
  GUID guid = nsGUID::fromChar(guidstr);
  return api->config_getCfgItemByGuid(guid);
}

GirderWnd::GirderWnd() : pldir(NULL) { 
  setStartHidden(TRUE);
  setName("Girder Internal");
}

GirderWnd::~GirderWnd() { 
  SvcEnum::release(pldir);

  IPCMessage(IPC_E_GOODBYE).send();
  IPCMessage::Term();

  core.delCallback(this);
}

const int MY_TIMER = 0x1234;

int GirderWnd::onInit() {
  int retval = GIRDERWND_PARENT::onInit();

  core.addCallback(this);

  setTimer(MY_TIMER, WACNAME::intervalAttrib);
  
  connect();

  return retval;
}

int GirderWnd::corecb_onStarted() {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;

  int pos = core.getCurPlaybackNumber();
  int len = core.getNumTracks();
  
  IPCMessage msg(IPC_E_START);
  msg << pos << len;
  BOOL ok = msg.send();

  if (GirderLogWnd::logging()) {
    char buf[64];
    sprintf(buf, "%d/%d", pos, len);
    GirderLogWnd::log((ok) ? "sent" : "lost", "start", buf);
  }
  return 0;
}

int GirderWnd::corecb_onStopped() {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;

  IPCMessage msg(IPC_E_STOP);
  BOOL ok = msg.send();

  if (GirderLogWnd::logging())
    GirderLogWnd::log((ok) ? "sent" : "lost", "stop", NULL);
  return 0;
}

int GirderWnd::corecb_onPaused() {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;

  sendPos(IPC_E_PAUSE, "pause");
  return 0;
}

int GirderWnd::corecb_onUnpaused() {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;

  sendPos(IPC_E_RESUME, "resume");
  return 0;
}

int GirderWnd::corecb_onSeeked(int pos) {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;

  int len = core.getLength();

  sendPos(IPC_E_SEEK, "seek", pos, len);
  return 0;
}

struct MetaInfo {
  const char *tag;
  int dt;
} MetaInfos[] = {
#if 0
  // Already handled.
  { MT_TITLE, },
#endif
  { MT_ALBUM, },
  { MT_ARTIST, },
  { MT_GENRE, },
  { MT_YEAR, },
  { MT_COMMENT, },
  { MT_INFO, },
#if 0
  // These ID3 tags with types other than string seem to not work,
  // either returning nothing or characters anyway.
  { MT_BITRATE, },
  { MT_BITSPERSAMPLE, },
  { MT_NUMCHANNELS, },
  { MT_SAMPLERATE, },
#endif
  { MT_TRACK, },
  { MT_NAME, },
#if 0
  // These are redundant or seem not useful.
  { MT_ADDEDON, },
  { MT_DISABLED, },
  { MT_LASTPLAYED, },
  { MT_LENGTH, },
  { MT_MODIFIED, },
  { MT_NAME2, },
  { MT_PLAYSTRING, },
  { MT_SIZE, },
#endif
};

#define countof(x) sizeof(x)/sizeof(x[0])

int GirderWnd::corecb_onTitleChange(const char *title) {
  if (!WACNAME::eventAttrib || !connect()) 
    return 0;
  
  const char *playstr = core.getCurrent();

  IPCMessage msg(IPC_E_PLAYSTRING);
  msg << playstr << "Title" << title;

  if (GirderLogWnd::logging()) {
    GirderLogWnd::log("send", "change", playstr);
    GirderLogWnd::log("", "Title", title);
  }

  for (size_t i = 0; i < countof(MetaInfos); i++) {
    const char *tag = MetaInfos[i].tag;
    int dt = MetaInfos[i].dt;
    if (MDT_NONE == dt)
      MetaInfos[i].dt = dt = api->metadb_getMetaDataType(tag);
    char buf[1024];
    int len = api->metadb_getMetaData(playstr, tag, buf, sizeof(buf), dt);
    if (len > 0) {
      char buf2[1024];
      const char *value = buf;
      if (MDT_STRINGZ != dt) {
        if (!api->metadb_convertToText(buf, dt, buf2, sizeof(buf2)))
          continue;
        value = buf2;
      }
      msg.addString(tag);
      msg.addString(value);
      if (GirderLogWnd::logging())
        GirderLogWnd::log("", tag, buf);
    }
  }

  BOOL ok = msg.send();

  return 0;
}

void GirderWnd::timerCallback(int id) {
  if (MY_TIMER != id) {
    GIRDERWND_PARENT::timerCallback(id);
    return;
  }
    
  if (isDestroying() || !WACNAME::eventAttrib || !connect())
    return;

  if (1 != core.getStatus()) return;

  sendPos(IPC_E_PLAYING, "playing", core.getPosition(), core.getLength());
}

int GirderWnd::onUserMessage(int umsg, int w, int l, int *r) {
  if (WM_IPC != umsg)
    return GIRDERWND_PARENT::onUserMessage(umsg, w, l, r);

  IPCMessage msg((LPARAM)l);

  *r = 0;                       // If nothing else.

  const char *logreq = NULL;
  char lreqbuf[1024], lrepbuf[1024];
  const char *lreqdata = NULL, *lrepdata = NULL;
  
  switch (w) {
  case IPC_C_START:
    logreq = "start";
    core.play();
    break;
  case IPC_C_STOP:
    logreq = "stop";
    core.stop();
    break;
  case ICP_C_PAUSE:
    logreq = "pause";
    core.pause();
    break;
  case IPC_C_RESUME:
    logreq = "resume";
    core.play();
    break;
  case IPC_C_PAUSE_TOGGLE:
    logreq = "pause/resume";
    if (1 == core.getStatus())
      core.pause();
    else
      core.play();
    break;
  case IPC_C_SEEK_ABSOLUTE:
    {
      int pos = msg.nextInt();
      if (GirderLogWnd::logging()) {
        logreq = "seek abs";
        sprintf(lreqbuf, "%d", pos);
        lreqdata = lreqbuf;
      }
      core.setPosition(pos);
    }
    break;
  case IPC_C_SEEK_RELATIVE:
    {
      int delta = msg.nextInt();
      if (GirderLogWnd::logging()) {
        logreq = "seek rel";
        sprintf(lreqbuf, "%d", delta);
        lreqdata = lreqbuf;
      }
      core.setPosition(core.getPosition() + delta);
    }
    break;
  case IPC_C_NEXT:
    logreq = "next";
    core.next();
    break;
  case IPC_C_PREV:
    logreq = "prev";
    core.prev();
    break;
  case IPC_C_SET_NEXT:
    {
      const char *playstr = msg.nextString();
      if (GirderLogWnd::logging()) {
        logreq = "set next";
        strncpy(lreqbuf, playstr, sizeof(lreqbuf));
        lreqdata = lreqbuf;
      }
      core.setNextFile(playstr);
    }
    break;
  case IPC_C_TRACK:
    {
      int pos = msg.nextInt();
      if (GirderLogWnd::logging()) {
        logreq = "set pos";
        sprintf(lreqbuf, "%d", pos);
        lreqdata = lreqbuf;
      }
      Playlist *playlist = getPlaylist();
      if (NULL != playlist)
        playlist->setCurrent(pos);
    }
    break;
  case IPC_C_NEXT_PLAYLIST:
    logreq = "next playlist";
    nextPlaylist(+1);
    break;
  case IPC_C_PREV_PLAYLIST:
    logreq = "prev playlist";
    nextPlaylist(-1);
    break;
  case IPC_C_START_PLAYLIST:
    {
      logreq = "start playlist";
      Playlist *playlist = getPlaylist();
      if (NULL != playlist)
        playlist->startPlayback(playlist->getCurrent());
    }
    break;
  case IPC_C_SET_VOLUME:
    {
      int vol = msg.nextInt();
      if (GirderLogWnd::logging()) {
        logreq = "set volume";
        sprintf(lreqbuf, "%d", vol);
        lreqdata = lreqbuf;
      }
      core.setVolume(vol);
    }
    break;
  case IPC_C_SET_ATTRIB:
    {
      const char *guidstr = msg.nextString();
      const char *name = msg.nextString();
      const char *value = msg.nextString();
      if (GirderLogWnd::logging()) {
        logreq = "set attrib";
        sprintf(lreqbuf, "%s %s %s", guidstr, name, value);
        lreqdata = lreqbuf;
      }
      CfgItem *cfg = getCfgItemForName(guidstr, name);
      if (NULL != cfg)
        cfg->setData(name, value);
    }
    break;
  case IPC_C_TOGGLE_ATTRIB:
    {
      const char *guidstr = msg.nextString();
      const char *name = msg.nextString();
      if (GirderLogWnd::logging()) {
        logreq = "toggle attrib";
        sprintf(lreqbuf, "%s %s", guidstr, name);
        lreqdata = lreqbuf;
      }
      CfgItem *cfg = getCfgItemForName(guidstr, name);
      if (NULL != cfg)
        cfg->setDataAsInt(name, !cfg->getDataAsInt(name));
    }
    break;

  case IPC_R_STATE:
    {
      msg.reuse();
      int status = core.getStatus();
      msg.addInt(status);
      if (GirderLogWnd::logging()) {
        logreq = "get state";
        if (status > 0)
          lrepdata = "playing";
        else if (status < 0)
          lrepdata = "paused";
        else
          lrepdata = "stopped";
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_PLAYSTRING:
    {
      msg.reuse();
      const char *playstr = core.getCurrent();
      if (NULL != playstr)
        msg.addString(playstr);
      if (GirderLogWnd::logging()) {
        logreq = "get playstring";
        if (NULL != playstr)
          strncpy(lrepbuf, playstr, sizeof(lrepbuf));
        else
          lrepbuf[0] = '\0';
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_METADATA:
    {
      const char *tag = msg.nextString();
      const char *playstr = msg.nextString();
      if (NULL == playstr)
        playstr = core.getCurrent();
      int dt = api->metadb_getMetaDataType(tag);
      char buf[1024], buf2[1024];
      const char *value = NULL;
      int len = api->metadb_getMetaData(playstr, tag, buf, sizeof(buf), dt);
      if (len > 0) {
        if (MDT_STRINGZ == dt)
          value = buf;
        else {
          if (api->metadb_convertToText(buf, dt, buf2, sizeof(buf2)))
            value = buf2;
        }
      }
      if (GirderLogWnd::logging()) {
        strncpy(lreqbuf, tag, sizeof(lreqbuf));
      }
      msg.reuse();
      if (NULL != value)
        msg.addString(value);
      if (GirderLogWnd::logging()) {
        logreq = "get meta-data";
        strncpy(lreqbuf, tag, sizeof(lreqbuf));
        lreqdata = lreqbuf;
        if (NULL != value)
          strncpy(lrepbuf, value, sizeof(lrepbuf));
        else
          lrepbuf[0] = '\0';
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_TRACK:
    {
      msg.reuse();
      int pos = core.getCurPlaybackNumber();
      int len = core.getNumTracks();
      msg << pos << len;
      if (GirderLogWnd::logging()) {
        logreq = "get track";
        sprintf(lrepbuf, "%d/%d", pos, len);
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_POSITION:
    {
      msg.reuse();
      int pos = core.getPosition();
      int len = core.getLength();
      msg << pos << len;
      if (GirderLogWnd::logging()) {
        logreq = "get position";
        sprintf(lrepbuf, "%d/%d", pos, len);
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_ITEM:
    {
      int pos = msg.nextInt();
      msg.reuse();
      const char *playstr = NULL;
      Playlist *playlist = getPlaylist();
      if (NULL != playlist)
        playstr = playlist->enumItem(pos);
      if (NULL != playstr)
        msg.addString(playstr);
      if (GirderLogWnd::logging()) {
        logreq = "get pl item";
        sprintf(lreqbuf, "%d", pos);
        lreqdata = lreqbuf;
        if (NULL != playstr)
          strncpy(lrepbuf, playstr, sizeof(lrepbuf));
        else
          lrepbuf[0] = '\0';
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_PLAYLIST:
    {
      msg.reuse();
      const char *name = NULL;
      Playlist *playlist = getPlaylist();
      if (NULL != playlist)
        name = playlist->getPlaylistName();
      if (NULL != name)
        msg.addString(name);
      if (GirderLogWnd::logging()) {
        logreq = "get pl name";
        if (NULL != name)
          strncpy(lrepbuf, name, sizeof(lrepbuf));
        else
          lrepbuf[0] = '\0';
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_VOLUME:
    {
      msg.reuse();
      int vol = core.getVolume();
      msg << vol;
      if (GirderLogWnd::logging()) {
        logreq = "get volume";
        sprintf(lrepbuf, "%d", vol);
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;
  case IPC_R_ATTRIB:
    {
      const char *guidstr = msg.nextString();
      const char *name = msg.nextString();
      char buf[1024];
      const char *value = NULL;
      CfgItem *cfg = getCfgItemForName(guidstr, name);
      if (NULL != cfg) {
        int len = cfg->getData(name, buf, sizeof(buf));
        if (len >= 0)
          value = buf;
      }
      if (GirderLogWnd::logging()) {
        sprintf(lreqbuf, "%s %s", guidstr, name);
      }
      msg.reuse();
      if (NULL != value)
        msg.addString(value);
      if (GirderLogWnd::logging()) {
        logreq = "get attrib";
        lreqdata = lreqbuf;
        if (NULL != value)
          strncpy(lrepbuf, value, sizeof(lrepbuf));
        else
          lrepbuf[0] = '\0';
        lrepdata = lrepbuf;
      }
      *r = (int)msg.detach();
    }
    break;

  default:
    {
      logreq = "unknown";
      sprintf(lreqbuf, "%d", umsg);
      lreqdata = lreqbuf;
    }
  }

  if (GirderLogWnd::logging()) {
    GirderLogWnd::log("recv", logreq, lreqdata);
    if (NULL != lrepdata)
      GirderLogWnd::log("reply", NULL, lrepdata);
  }
  
  return 1;                     // Message recognized, but not command.
}

BOOL GirderWnd::connect() {
  if (IPCMessage::IsInit()) 
    return TRUE;
  if (!IPCMessage::Init(gethWnd()))
    return FALSE;

  IPCMessage msg(IPC_E_HELLO);
  msg << WACNAME::prefixAttrib;
  BOOL ok = msg.send();

  if (GirderLogWnd::logging())
    GirderLogWnd::log((ok) ? "sent" : "lost", "hello", WACNAME::prefixAttrib);
  
  return ok;
}

void GirderWnd::sendPos(IPC_OPCODE op, const char *logop) {
  sendPos(op, logop, core.getPosition(), core.getLength());
}

void GirderWnd::sendPos(IPC_OPCODE op, const char *logop, int pos, int len) {
  IPCMessage msg(op);
  msg << pos << len;
  BOOL ok = msg.send();

  if (GirderLogWnd::logging()) {
    char buf[128];
    char *ep = buf;
  
    api->metadb_convertToText(&pos, MDT_TIME, ep, sizeof(buf) - (ep - buf));

    ep += strlen(ep);
    *ep++ = '/';

    api->metadb_convertToText(&len, MDT_TIME, ep, sizeof(buf) - (ep - buf));

    GirderLogWnd::log((ok) ? "sent" : "lost", logop, buf);
  }
}

Playlist *GirderWnd::getPlaylist()
{
  if (NULL == pldir) {
    pldir = SvcEnumByGuid<svc_plDir>();
    if (NULL == pldir)
      return NULL;
  }
  
  PlaylistHandle plhand = pldir->getCurrentlyOpen();
  return pldir->getPlaylist(plhand);
}

struct FinderState {
  DWORD procId;
  GUID guid;
  RootWnd *result;
};

BOOL CALLBACK FinderProc(HWND hwnd, LPARAM lparam)
{
  FinderState *state = (FinderState *)lparam;

  // Must be for same process.
  DWORD procId;
  GetWindowThreadProcessId(hwnd, &procId);
  if (procId != state->procId) return TRUE;

  // Must be a RootWnd.
  char buf[128];
  GetClassName(hwnd, buf, sizeof(buf));
  if (strcmp(buf, "BaseWindow_RootWnd")) return TRUE;
  
  // Get Winamp window from Windows window.
  RootWnd *wnd = (RootWnd *)GetWindowLong(hwnd, GWL_USERDATA);
  if (NULL == wnd) return TRUE;

  // Get matching child, if any.
  wnd = wnd->findWindowByInterface(state->guid);
  if (NULL == wnd) return TRUE;
  
  state->result = wnd;
  return FALSE;
}

// I cannot find a function to enumerate the top-level windows for all
// layouts on the Winamp side.  So, do it on the Windows side.  This
// is not portable, obviously.
RootWnd *FindTopLevelWindowByInterface(GUID interfaceGuid)
{
  FinderState state;
  state.procId = GetCurrentProcessId();
  state.guid = interfaceGuid;
  state.result = NULL;

  EnumWindows(FinderProc, (LPARAM)&state);

  return state.result;
}

void GirderWnd::nextPlaylist(int delta)
{
  if (NULL == pldir) {
    pldir = SvcEnumByGuid<svc_plDir>();
    if (NULL == pldir)
      return;
  }
  
  PlaylistHandle plcurr = pldir->getCurrentlyOpen();
  int npl = pldir->getNumPlaylists();
  for (int i = 0; i < npl; i++) {
    PlaylistHandle plhand = pldir->enumPlaylist(i);
    if (plhand == plcurr) {
      i += delta;
      if ((i >= 0) && (i < npl)) {
        plhand = pldir->enumPlaylist(i);
        pldir->setCurrentlyOpen(plhand);
#if 0        
        // Don't start playback, right?
        pldir->getPlaylist(plhand)->startPlayback(0);
#endif
        // Inform the editor if visible.
#if 0
        // This will only find windows in the same "layout".
        RootWnd *wnd = findWindowByInterface(Editor::getInterfaceGuid());
#else
        RootWnd *wnd = FindTopLevelWindowByInterface(Editor::getInterfaceGuid());
#endif
        if (NULL != wnd) {
          Editor *ed = static_cast<Editor*>(wnd->getInterface(Editor::getInterfaceGuid()));
          ed->setPlaylistByHandle(plhand);
        }
      }
      break;
    }
  }
}

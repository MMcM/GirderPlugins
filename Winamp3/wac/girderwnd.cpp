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

  setTimer(MY_TIMER, 1000);
  
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
      char buf[1024];
      const char *value = buf;
      int len = api->metadb_getMetaData(playstr, tag, buf, sizeof(buf), dt);
      msg.reuse();
      if (len > 0) {
        char buf2[1024];
        if (MDT_STRINGZ != dt) {
          if (api->metadb_convertToText(buf, dt, buf2, sizeof(buf2)))
            value = buf2;
          else
            value = NULL;
        }
        if (NULL != value)
          msg.addString(value);
      }
      else
        value = NULL;
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

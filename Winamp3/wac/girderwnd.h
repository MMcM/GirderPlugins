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

#ifndef _GIRDERWND_H
#define _GIRDERWND_H

#include <common/basewnd.h>
#include <studio/corecb.h>
#include <bfc/wnds/buttwnd.h>
#include <common/nsGUID.h>
#include <common/corehandle.h>

class svc_plDir;
class Playlist;

#define GIRDERWND_PARENT BaseWnd

class GirderWnd : public GIRDERWND_PARENT, public CoreCallbackI {
public:
  GirderWnd();
  ~GirderWnd();

  virtual int onInit();

  // Basic Events
  virtual int corecb_onStarted();
  virtual int corecb_onStopped();
  virtual int corecb_onPaused();
  virtual int corecb_onUnpaused();
  virtual int corecb_onSeeked(int newpos);
  virtual int corecb_onTitleChange(const char *title);

  virtual void timerCallback(int id);
  virtual int onUserMessage(int msg, int w, int l, int *r);

protected:
  BOOL connect();
  void sendPos(IPC_OPCODE op, const char *logop);
  void sendPos(IPC_OPCODE op, const char *logop, int pos, int len);
  Playlist *getPlaylist();

  BOOL logging();
  void log(const char *dir, const char *command, const char *data);

private:
  CoreHandle core;
  svc_plDir *pldir;
};

#endif _GIRDERWND_H

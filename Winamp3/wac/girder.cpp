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

// Girder Winamp component, based on Generic example.

#include <bfc/std.h>

#include "girder.h"
#include "ipc.h"
#include "girderwnd.h"
#include "girderlogwnd.h"

// Window creation services includes.
#include <bfc/wndcreator.h>


static WACNAME wac;
WACPARENT *the = &wac;                     

// {57FF2C53-08D7-444a-B20E-CDC520088B33}
static const GUID guid = 
{ 0x57ff2c53, 0x8d7, 0x444a, { 0xb2, 0xe, 0xcd, 0xc5, 0x20, 0x8, 0x8b, 0x33 } };

_bool WACNAME::eventAttrib("EventAttrib", TRUE);
_int WACNAME::intervalAttrib("IntervalAttrib", 1000);
_string WACNAME::prefixAttrib("PrefixAttrib", "Winamp3");

WACNAME::WACNAME() : WACPARENT("Girder") {
  registerAttribute(&eventAttrib);
  registerAttribute(&intervalAttrib);
  registerAttribute(&prefixAttrib);
  registerSkinFile("wacs/xml/girder/preferences.xml");

  registerService(new WndCreateCreatorSingle< CreateWndByGuid<GirderLogWnd> >);

#if 0
  // Don't bother with Thinger.
  registerService(new WndCreateCreatorSingle< CreateBucketItem<GirderLogWnd> >);
#endif

  // But available in Windows pop-up.
  registerAutoPopup(getGUID(), GirderLogWnd::getWindowTypeName());
}

WACNAME::~WACNAME() {
}

GUID WACNAME::getGUID() {
  return guid;
}

static GirderWnd *wnd = NULL;

void WACNAME::onCreate() {
  api->preferences_registerGroup("Girder", "Girder", getGUID());
}

void WACNAME::onDestroy() {
}

int WACNAME::onNotify(int cmd, int param1, int param2, int param3, int param4) {
  switch (cmd) {
  case WAC_NOTIFY_SERVICE_NOTIFY:
    switch (param1) {
    case SvcNotify::ONAPPRUNNING:
      // It is better to do this here, rather than in onCreate or on
      // ONSTARTUP, because an existing log window will be running by
      // now and so see the hello message.
      wnd = new GirderWnd;
      wnd->init(api->main_getRootWnd());
      break;
    case SvcNotify::ONSHUTDOWN:
      // It is better to do this here, rather than in onDestroy
      // because the hidden window's parent still exists.
      delete wnd;
      break;
    }
    break;
  }
  return WACPARENT::onNotify(cmd, param1, param2, param3, param4);
}

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

// Girder <-> Winamp communications log window: a simple scrolling list.
// Only shows events that take place while it is open.

#include <bfc/std.h>
#include <core/corehandle.h>

#include "girder.h"
#include "girderlogwnd.h"
#include "resource.h"

static GirderLogWnd *logWnd = NULL;

GirderLogWnd::GirderLogWnd() { 
  setName("Girder Log");
}

GirderLogWnd::~GirderLogWnd() { 
  if (this == logWnd)
    logWnd = NULL;
}

//  Methods required by Window Creation Services
const char *GirderLogWnd::getWindowTypeName() { return "Girder Log"; }
GUID GirderLogWnd::getWindowTypeGuid() { return the->getGUID(); }
void GirderLogWnd::setIconBitmaps(ButtonWnd *button) {
  button->setBitmaps(the->gethInstance(), IDB_TAB_NORMAL, NULL, IDB_TAB_HILITED, IDB_TAB_SELECTED);
}


int GirderLogWnd::onInit() {
  int retval = GIRDERLOGWND_PARENT::onInit();
  if (!retval) return retval;

  // Oddly enough, only addColumn takes the width and only ListColumn takes dynamic.
  addColumn("Dir", 50);
  addColumn("Command", 75);
  insertColumn(new ListColumn("Data", TRUE));

  logWnd = this;

  return 1;
}

BOOL GirderLogWnd::logging() {
  return (NULL != logWnd);
}

void GirderLogWnd::log(const char *dir, const char *command, const char *data) {
  if (NULL == logWnd)
    return;
  logWnd->doLog(dir, command, data);
}

void GirderLogWnd::doLog(const char *dir, const char *command, const char *data) {
  if (isDestroying())
    return;
  addItem(dir, 0);
  int nitems = getNumItems();
  if (nitems > 50) {            // A few should be plenty.
    deleteByPos(0);
    nitems--;
  }
  if (NULL != command)
    setSubItem(nitems-1, 1, command);
  if (NULL != data)
    setSubItem(nitems-1, 2, data);
#if 1
  // Is this a good idea?  Also, it seems to cause trouble if the main
  // window is closed while paused, logging the stop notification
  // after the window has started disappearing.
  ensureItemVisible(nitems-1);
#endif
}

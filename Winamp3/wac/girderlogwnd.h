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

#ifndef _GIRDERLOGWND_H
#define _GIRDERLOGWND_H

#include <bfc/wnds/listwnd.h>
#include <core/corecb.h>
#include <bfc/wnds/buttwnd.h>
#include <common/nsGUID.h>

class svc_plDir;
class Playlist;

#define GIRDERLOGWND_PARENT ListWnd

class GirderLogWnd : public GIRDERLOGWND_PARENT {
public:
  GirderLogWnd();
  ~GirderLogWnd();

  // Methods required by Window Creation Services
  static const char *getWindowTypeName();
  static GUID getWindowTypeGuid();
  static void setIconBitmaps(ButtonWnd *button);    

  virtual int onInit();

  static BOOL logging();
  static void log(const char *dir, const char *command, const char *data);
  void doLog(const char *dir, const char *command, const char *data);
};

#endif _GIRDERLOGWND_H

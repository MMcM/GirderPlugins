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

#ifndef _GIRDER_H
#define _GIRDER_H

#include <studio/wac.h>
#include <bfc/attribs/attrbool.h>
#include <bfc/attribs/attrint.h>
#include <bfc/attribs/attrstr.h>

class GirderWnd;

#define WACNAME WACGirder
#define WACPARENT WAComponentClient

class WACNAME : public WACPARENT {
public:
  WACNAME();
  virtual ~WACNAME();

  virtual GUID getGUID();

  virtual void onCreate();
  virtual void onDestroy();
  virtual int onNotify(int cmd, int param1, int param2, int param3, int param4);

  static _bool eventAttrib;
  static _int intervalAttrib;
  static _string prefixAttrib;
};

extern WACPARENT *the;

#endif//_GIRDER_H

// Machine generated IDispatch wrapper class(es) created with ClassWizard
// $Header$

#include "stdafx.h"
#include "AutoClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTVSpyAutoClient properties

/////////////////////////////////////////////////////////////////////////////
// CTVSpyAutoClient operations

void CTVSpyAutoClient::SetChannel(short channel)
{
  static BYTE parms[] = VTS_I2;
  InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
               channel);
}

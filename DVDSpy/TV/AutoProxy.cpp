// AutoProxy.cpp : implementation file
//

#include "stdafx.h"
#include "slinkx.h"
#include "TVSpy.h"
#include "TVSpyDlg.h"
#include "AutoProxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTVSpyAutoProxy

IMPLEMENT_DYNCREATE(CTVSpyAutoProxy, CCmdTarget)

CTVSpyAutoProxy::CTVSpyAutoProxy()
{
  EnableAutomation();
	
  // To keep the application running as long as an automation 
  //	object is active, the constructor calls AfxOleLockApp.
  AfxOleLockApp();
}

CTVSpyAutoProxy::~CTVSpyAutoProxy()
{
  AfxOleUnlockApp();
}

void CTVSpyAutoProxy::OnFinalRelease()
{
  // When the last reference for an automation object is released
  // OnFinalRelease is called.  The base class will automatically
  // deletes the object.  Add additional cleanup required for your
  // object before calling the base class.

  CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CTVSpyAutoProxy, CCmdTarget)
  //{{AFX_MSG_MAP(CTVSpyAutoProxy)
    // NOTE - the ClassWizard will add and remove mapping macros here.
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTVSpyAutoProxy, CCmdTarget)
  //{{AFX_DISPATCH_MAP(CTVSpyAutoProxy)
  DISP_FUNCTION(CTVSpyAutoProxy, "SetChannel", SetChannel, VT_EMPTY, VTS_I2)
  //}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_TVSpy to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {A2ABCDE7-D3EC-4C07-A408-5C998A787D57}
static const IID IID_TVSpy =
{ 0xa2abcde7, 0xd3ec, 0x4c07, { 0xa4, 0x8, 0x5c, 0x99, 0x8a, 0x78, 0x7d, 0x57 } };

BEGIN_INTERFACE_MAP(CTVSpyAutoProxy, CCmdTarget)
  INTERFACE_PART(CTVSpyAutoProxy, IID_TVSpy, Dispatch)
END_INTERFACE_MAP()

// {A0B48ED3-EEB4-4566-9C5A-3B47BE71F7B1}
IMPLEMENT_OLECREATE(CTVSpyAutoProxy, "TVSpy.TVSpy", 0xa0b48ed3, 0xeeb4, 0x4566, 0x9c, 0x5a, 0x3b, 0x47, 0xbe, 0x71, 0xf7, 0xb1)

/////////////////////////////////////////////////////////////////////////////
// CTVSpyAutoProxy message handlers

void CTVSpyAutoProxy::SetChannel(short channel)
{
  CTVSpyApp *pApp = (CTVSpyApp *)AfxGetApp();
  CTVSpyDlg *pDialog = (CTVSpyDlg *)pApp->m_pMainWnd;  

  if (NULL != pDialog)
    pDialog->SaveLastChan();

  pApp->SetChannel(channel);

  if (NULL != pDialog) {
    pDialog->UpdateChanDesc();
  }
}

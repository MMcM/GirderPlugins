// DVDSpyMJ.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f DVDSpyMJps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DVDSpyMJ.h"

#include "DVDSpyMJ_i.c"
#include "DVDSpyMJCtrl.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DVDSpyMJCtrl, CDVDSpyMJCtrl)
END_OBJECT_MAP()

class CDVDSpyMJApp : public CWinApp
{
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDVDSpyMJApp)
  public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CDVDSpyMJApp)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CDVDSpyMJApp, CWinApp)
  //{{AFX_MSG_MAP(CDVDSpyMJApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDVDSpyMJApp theApp;

BOOL CDVDSpyMJApp::InitInstance()
{
  _Module.Init(ObjectMap, m_hInstance, &LIBID_DVDSPYMJLib);
  return CWinApp::InitInstance();
}

int CDVDSpyMJApp::ExitInstance()
{
  _Module.Term();
  return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
  // register with Media Jukebox
  char szRegistryPath[1024];
  strcpy(szRegistryPath, REGISTRY_PATH_MJ_PLUGINS_INTERFACE);
  strcat(szRegistryPath, "DVDSpy");
	
  CRegKey reg;
  if (ERROR_SUCCESS == reg.Create(HKEY_CURRENT_USER, szRegistryPath)) {		
    OLECHAR wszCLSID[128];
    StringFromGUID2(CLSID_DVDSpyMJCtrl, wszCLSID, sizeof(wszCLSID)/sizeof(OLECHAR));
    USES_CONVERSION;

    // Stuff for MJ.
    reg.SetValue(OLE2T(wszCLSID), "CLSID");
    reg.SetValue(1, "IVersion");
    reg.SetValue("1.28 (4)", "Version");
    reg.SetValue("www.girder.nl", "URL");
    reg.SetValue(1, "PluginMode");

    // Stuff for us.
    reg.SetValue(1000, "RefreshInterval");
  }	
  // registers object, typelib and all interfaces in typelib
  return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
  return _Module.UnregisterServer(TRUE);
}

/*********************************************************************
 *  @file   : Anir2Girder.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : App class for DLL
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#include "stdafx.h"
#include "Anir2Girder.h"
#include "Settings.h"
#include "About.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAnir2GirderApp

BEGIN_MESSAGE_MAP(CAnir2GirderApp, CWinApp)
	//{{AFX_MSG_MAP(CAnir2GirderApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnir2GirderApp construction


/*!
 * Constructor
 *
 * @param none
 *
 * @return  : 
 */
CAnir2GirderApp::CAnir2GirderApp()
{
	DEBUG_MSG("CAnir2GirderApp::CAnir2GirderApp()");

	m_hWndMain = NULL;
	m_pRemote = new CAnirRemote(&m_Girder);
}




/*!
 * Destructor
 *
 * @param none
 *
 * @return  : 
 */
CAnir2GirderApp::~CAnir2GirderApp()
{
	DEBUG_MSG("CAnir2GirderApp::~CAnir2GirderApp()");

	SAFE_DELETE(m_pRemote);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAnir2GirderApp object

//! Applications object
CAnir2GirderApp theApp;




/*!
 * Open plugin and read settings from registry
 *
 * @param gir_major_ver : 
 * @param gir_minor_ver : 
 * @param gir_micro_ver : 
 * @param api_functions : Plugin functions
 *
 * @return int  : Status
 */
int CAnir2GirderApp::PluginOpen(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions api_functions)
{
	DEBUG_MSG("CAnir2GirderApp::PluginOpen(...)");
	MYTRACE("Girder v%i.%i.%i: Open Plugin\n", gir_major_ver, gir_minor_ver, gir_micro_ver);
	
	if ( api_functions->size != sizeof(s_functions) )
		return GIR_FALSE;

	m_Girder.setActions(api_functions);

	// Load Settings
	loadFromRegistry();

	return GIR_TRUE;
}




/*!
 * Close plugin
 *
 * @param none
 *
 * @return int  : Status
 */
int CAnir2GirderApp::PluginClose()
{
	DEBUG_MSG("CAnir2GirderApp::PluginClose()");

	return GIR_TRUE;
}


/*!
 * Start plugin and enable events
 *
 * @param none
 *
 * @return int  : Status
 */
int CAnir2GirderApp::PluginStart()
{
	DEBUG_MSG("CAnir2GirderApp::PluginStart()");

	if (!m_pRemote->start(m_pRemote->getPort()))
		return GIR_FALSE;

	m_Girder.enableEvents(TRUE);

	return GIR_TRUE;
}




/*!
 * Stop plugin and disable events
 *
 * @param none
 *
 * @return int  : Status
 */
int CAnir2GirderApp::PluginStop()
{
	DEBUG_MSG("CAnir2GirderApp::PluginStop()");


	if (!m_pRemote->stop())
		return GIR_FALSE;

	m_Girder.enableEvents(FALSE);

	return GIR_TRUE;
}



/*!
 * Open Settings dlg and save setting to reg if ok
 *
 * @param none
 *
 * @return void  : 
 */
void CAnir2GirderApp::PluginConfig()
{
	DEBUG_MSG("CAnir2GirderApp::PluginConfig()");
	
	CSettings dlg(m_pRemote);
	if ( IDOK == dlg.DoModal() )
	{
		// Save settings
		saveToRegistry();
		MYTRACE("Settings saved to registry!\n");
	}

}




/*!
 * Open About dlg
 *
 * @param none
 *
 * @return void  : 
 */
void CAnir2GirderApp::PluginCommandGui()
{
	DEBUG_MSG("CAnir2GirderApp::PluginCommandGui()");

	CAbout dlg;
	dlg.DoModal();
}




/*!
 * User changed command in Girder. Set new current command.
 *
 * @param command : 
 *
 * @return void  : 
 */
void CAnir2GirderApp::PluginCommandChanged(p_command command)
{
	DEBUG_MSG("CAnir2GirderApp::PluginCommandChanged()");

	m_Girder.setCurCommand(command);
}



/*!
 * Save settings to registry
 *
 * @param none
 *
 * @return void  : 
 */
void CAnir2GirderApp::saveToRegistry()
{
	DEBUG_MSG("CAnir2GirderApp::saveToRegistry()");

	CString strKey = CString(REGPATH) + CString(PLUGINNAME);
	HKEY regKey;

	RegCreateKeyEx( HKEY_LOCAL_MACHINE, strKey, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
				&regKey, NULL );
	
	HRESULT hr;
	bool bErr = FALSE;

	if ( FAILED(hr = My_WriteStringRegKey(regKey, REGVERSION, PLUGINVERSION)) )
		bErr = TRUE;
	
	if ( FAILED(hr = My_WriteIntRegKey(regKey, REGPORT, m_pRemote->getPort())) )
		bErr = TRUE;

	if ( FAILED(hr = My_WriteStringRegKey(regKey, REGNAME,
		m_pRemote->getName().GetBuffer(MAX_PATH))) )
		bErr = TRUE;

	if ( FAILED(hr = My_WriteBoolRegKey(regKey, REGUPEVENTS, m_pRemote->getEventOnUp())) )
		bErr = TRUE;

	if ( FAILED(hr = My_WriteBoolRegKey(regKey, REGREPEATEVENTS, m_pRemote->getEventRepeat())) )
		bErr = TRUE;

	if (bErr)
		reportWarning("Could not save settings.");

}



/*!
 * Load settings from registry
 *
 * @param none
 *
 * @return void  : 
 */
void CAnir2GirderApp::loadFromRegistry()
{
	DEBUG_MSG("CAnir2GirderApp::loadFromRegistry()");

	CString strKey = CString(REGPATH) + CString(PLUGINNAME);
	HKEY regKey;

	RegCreateKeyEx( HKEY_LOCAL_MACHINE, strKey, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
				&regKey, NULL );
	
	HRESULT hr;
	TCHAR str[MAX_PATH];
	DWORD dw;
	bool bErr = FALSE;
	BOOL bVal = TRUE;

	if (FAILED(hr = My_ReadIntRegKey(regKey, REGPORT, &dw, 1)) )
		bErr = TRUE;
	m_pRemote->setPort(dw);


	if (FAILED(hr = My_ReadStringRegKey(regKey, REGNAME, str, MAX_PATH, "Anir")) )
		bErr = TRUE;
	m_pRemote->setName(str);

	if (FAILED(hr = My_ReadBoolRegKey(regKey, REGUPEVENTS, &bVal, TRUE)) )
		bErr = TRUE;
	m_pRemote->setEventOnUp(bVal);

	if (FAILED(hr = My_ReadBoolRegKey(regKey, REGREPEATEVENTS, &bVal, TRUE)) )
		bErr = TRUE;
	m_pRemote->setEventRepeat(bVal);

	if (bErr)
		reportWarning("Could not load settings. Check the plugin settings");
}



/*!
 * Return window handle
 *
 * @param none
 *
 * @return HWND  : 
 */
HWND CAnir2GirderApp::getMainWindow()
{
	DEBUG_MSG("CAnir2GirderApp::getMainWindow()");

	return m_hWndMain;
}


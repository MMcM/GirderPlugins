/*********************************************************************
 *  @file   : Anir2Girder.h
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


#if !defined(AFX_ANIR2GIRDER_H__BD02F85B_2663_4CF2_A873_D09B1D7E6E3A__INCLUDED_)
#define AFX_ANIR2GIRDER_H__BD02F85B_2663_4CF2_A873_D09B1D7E6E3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "PluginIF.h"
#include "AnirRemote.h"
#include "Girder.h"


// Version and basic info
#define PLUGINVERSION		"0.3.0"
#define PLUGINNAME			"Anir Remote"
#define PLUGINDESCRIPTION	"Animax Anir Remote plugin. (C) 2002 by Tom Skoglund."
#define PLUGINDEVICEID		53

// Registry keys
#define REGPATH				"Software\\Girder3\\HardPlugins\\"
#define REGVERSION			"PluginVersion"
#define REGPORT				"Remote Port"
#define REGNAME				"Remote Name"
#define REGUPEVENTS			"Send Up Events"
#define REGREPEATEVENTS		"Send Repeat Events"



/////////////////////////////////////////////////////////////////////////////
// CAnir2GirderApp
// See Anir2Girder.cpp for the implementation of this class
//

/*!
@par CLASS
    CAnir2GirderApp: Applications class

@par USAGE
<pre> Methods: 

</pre> 

<pre> Attributes: 

</pre> 

*/

//! Application class
class CAnir2GirderApp : public CWinApp
{
public:
	HWND getMainWindow();
	CAnir2GirderApp();
	~CAnir2GirderApp();
	int PluginClose();
	int PluginOpen(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions api_functions);
	void PluginConfig();
	void PluginCommandGui();
	void PluginCommandChanged(p_command command);
	int PluginStart();
	int PluginStop();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnir2GirderApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAnir2GirderApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	void loadFromRegistry();
	void saveToRegistry();


private:
	//! Girder object
	CGirder m_Girder;
	//! Remote pointer
	CAnirRemote *m_pRemote;
	//! Window
	HWND m_hWndMain;
};



extern CAnir2GirderApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.




#endif // !defined(AFX_ANIR2GIRDER_H__BD02F85B_2663_4CF2_A873_D09B1D7E6E3A__INCLUDED_)

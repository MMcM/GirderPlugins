/*********************************************************************
 *  @file   : AnirRemote.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Communication with remote
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#include "stdafx.h"
#include "anir2girder.h"
#include "AnirRemote.h"
#include "Girder.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*!
 * Constructor
 *
 * @param *girder : 
 *
 * @return  : 
 */
CAnirRemote::CAnirRemote(CGirder *girder)
	: CAnirRemoteControl()
{
	DEBUG_MSG("CAnirRemote::CAnirRemote()");

	m_iPort = 0;
	m_sName = _T("Anir");
	m_pGirder = girder;
	m_bRunning = FALSE;
	m_bEventOnUp = TRUE;
}


/*!
 * Destructor
 *
 * @param none
 *
 * @return  : 
 */
CAnirRemote::~CAnirRemote()
{
	DEBUG_MSG("CAnirRemote::~CAnirRemote()");

	if (isRunning())
		stop();
}


/*!
 * Process data from remote: 
 * Create event string and send away
 *
 * @param nData : 
 * @param bRepeat : 
 *
 * @return void  : 
 */
void CAnirRemote::ProcessReceivedData(BYTE nData, BOOL bRepeat)
{
	// Mouse event
	if ((nData & 0x1D) == 20)
	{
		MYTRACE("MOUSE EVENT\n");
	}
	else
	// "Noramal" button
	{
		// Check if button up events should be sent
		if (!m_bEventOnUp && (nData % 2 == 1))
			return;
	}

	// Create event string
	// Format $name:$nData:$bRepeat
	CString strEvent;
	strEvent.Format(_T("%s:%d:%d"), m_sName, nData, bRepeat);
	m_pGirder->sendEvent(strEvent);
}


/*!
 * Start monitoring remote
 *
 * @param port : Port to listen to
 *
 * @return int  : Status
 */
int CAnirRemote::start(int port)
{
	DEBUG_MSG("CAnirRemote::start()");

	// Try starting it
	if (!StartMonitoring(port))
	{
		CString str;
		str.Format("No remote found at port %i\n", port);
		reportError(str);
		return FALSE;
	}

	MYTRACE("Remote control started at port %i !\n", port);
	m_iPort = port;
	m_bRunning = TRUE;
	return TRUE;
}


/*!
 * Restart plugin at current port
 *
 * @param none
 *
 * @return int  : 
 */
int CAnirRemote::restart()
{
	DEBUG_MSG("CAnirRemote::restart()");

	if (!stop())
		return FALSE;

	if (!start(m_iPort))
		return FALSE;

	return TRUE;
}


/*!
 * Stop monitoring remote
 *
 * @param none
 *
 * @return int  : Status
 */
int CAnirRemote::stop()
{
	DEBUG_MSG("CAnirRemote::stop()");


	if (!StopMonitoring())
	{
		CString str;
		str.Format("Unable to stop remote running at port %i !", GetPort());
		reportError(str);
		return FALSE;
	}

	m_bRunning = FALSE;
	return TRUE;
}


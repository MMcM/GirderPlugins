/*********************************************************************
 *  @file   : Girder.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Girder state
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#include "stdafx.h"
#include "anir2girder.h"
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
 * @param none
 *
 * @return  : 
 */
CGirder::CGirder()
{
	DEBUG_MSG("CGirder::CGirder()");

	InitializeCriticalSection(&m_EnableMutex);
	InitializeCriticalSection(&m_ActionMutex);

	enableEvents(FALSE);
	m_pCurrentCommand = NULL;
}


/*!
 * Destructor
 *
 * @param none
 *
 * @return  : 
 */
CGirder::~CGirder()
{
	DEBUG_MSG("CGirder::~CGirder()");

	DeleteCriticalSection(&m_EnableMutex);
	DeleteCriticalSection(&m_ActionMutex);
}


/*!
 * Enable/Disable events
 *
 * @param bFlag : 
 *
 * @return void  : 
 */
void CGirder::enableEvents(bool bFlag)
{
	DEBUG_MSG("CGirder::enableEvents()");

	EnterCriticalSection(&m_EnableMutex);
	m_bEnabled = bFlag;
	LeaveCriticalSection(&m_EnableMutex);
}


/*!
 * Set action pointer
 *
 * @param pf : 
 *
 * @return void  : 
 */
void CGirder::setActions(p_functions pf)
{
	DEBUG_MSG("CGirder::setActions()");

	EnterCriticalSection(&m_ActionMutex);
	m_ActionFunctions = *pf;
	LeaveCriticalSection(&m_ActionMutex);
}


/*!
 * Set current command
 *
 * @param command : 
 *
 * @return void  : 
 */
void CGirder::setCurCommand(p_command command)
{
	DEBUG_MSG("CGirder::setCurCommand()");

	// Skip if its the same
	if (command != m_pCurrentCommand)
	{
		// Do not try enter CS on NULL
		if (command)
			EnterCriticalSection(&command->critical_section);
		if (m_pCurrentCommand)
			EnterCriticalSection(&m_pCurrentCommand->critical_section);

		p_command old = m_pCurrentCommand;
		m_pCurrentCommand = command;

		// Leave CS (not on NULL)
		if (old)
			LeaveCriticalSection(&old->critical_section);
		if (m_pCurrentCommand)
			LeaveCriticalSection(&m_pCurrentCommand->critical_section);
	}
}


/*!
 * Send event to Girder
 *
 * @param strEvent : Event string
 *
 * @return int  : Status
 */
int CGirder::sendEvent(CString strEvent)
{
	DEBUG_MSG("CGirder::sendEvent()");
	MYTRACE("eventstring = %s\n", strEvent);

	EnterCriticalSection(&m_EnableMutex);
	if (m_bEnabled)
	{
		// Send data to Girder
		EnterCriticalSection(&m_ActionMutex);
		int result = m_ActionFunctions.send_event(strEvent,
			NULL,
			0,
			PLUGINDEVICEID);
		LeaveCriticalSection(&m_ActionMutex);


	}
	else
		MYTRACE("Sending events disabled\n");
	LeaveCriticalSection(&m_EnableMutex);

	return GIR_TRUE;
}

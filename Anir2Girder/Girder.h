/*********************************************************************
 *  @file   : Girder.h
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


#if !defined(AFX_GIRDER_H__D12766E1_DB2F_476D_A2AF_0A378C29526B__INCLUDED_)
#define AFX_GIRDER_H__D12766E1_DB2F_476D_A2AF_0A378C29526B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "PluginIF.h"


/*!
@par CLASS
    CGirder 

@par USAGE
<pre> Methods: 

</pre> 

<pre> Attributes: 

</pre> 

*/

//! Girder state
class CGirder  
{
public:
	int sendEvent(CString strEvent);
	void setCurCommand(p_command command);
	void setActions(p_functions pf);
	void enableEvents(bool bFlag);
	CGirder();
	virtual ~CGirder();


private:
	//! Current active command
	p_command m_pCurrentCommand;
	//! Mutex for enabling
	CRITICAL_SECTION m_EnableMutex;
	//! Events enabled
	bool m_bEnabled;
	//! Mutex for actions
	CRITICAL_SECTION m_ActionMutex;
	//! Girder functions
	t_functions m_ActionFunctions;

};

#endif // !defined(AFX_GIRDER_H__D12766E1_DB2F_476D_A2AF_0A378C29526B__INCLUDED_)

/*********************************************************************
 *  @file   : AnirRemote.h
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


#if !defined(AFX_REMOTELISTENER_H__0C20EA58_EC38_42BE_AA0A_929B39B0AF5C__INCLUDED_)
#define AFX_REMOTELISTENER_H__0C20EA58_EC38_42BE_AA0A_929B39B0AF5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ANIR32.H"

class CGirder;

/*!
@par CLASS
    CAnirRemote: Remote communication 

@par USAGE
<pre> Methods: 

</pre> 

<pre> Attributes: 

</pre> 

*/

//! Remote communication
class CAnirRemote : public CAnirRemoteControl  
{
public:
	//! Get remote port
	int getPort()				{return m_iPort;}
	//! Set remote port
	void setPort(int p)			{m_iPort=p;}
	//! Get port name
	CString getName()			{return m_sName;}
	//! Set port name
	void setName(CString n)		{m_sName = n;}
	//! Are we running
	bool isRunning()			{return m_bRunning;}
	//! Get event on button up
	BOOL getEventOnUp()			{return m_bEventOnUp;}
	//! Set event on button up
	void setEventOnUp(BOOL b)	{m_bEventOnUp = b;}
	//! Get repeat event status
	BOOL getEventRepeat()		{return m_bEventRepeat;}
	//! Set repeat event status
	void setEventRepeat(BOOL b)	{m_bEventRepeat = b;}

	int start(int port);
	int restart();
	int stop();
	CAnirRemote(CGirder *girder);
	virtual ~CAnirRemote();

protected:
	void ProcessReceivedData(BYTE nData, BOOL bRepeat);


private:
	//! Plugin running
	bool m_bRunning;
	//! Event on button up
	BOOL m_bEventOnUp;
	//! Repeat event
	BOOL m_bEventRepeat;
	//! Com port for plugin
	int m_iPort;
	//! Name for port
	CString m_sName;
	//! Pointer to Girder class
	CGirder *m_pGirder;

};

#endif // !defined(AFX_REMOTELISTENER_H__0C20EA58_EC38_42BE_AA0A_929B39B0AF5C__INCLUDED_)

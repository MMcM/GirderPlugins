/*********************************************************************
 *  @file   : Settings.h
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Settings dialog
 *
 *********************************************************************
 * $Id$
 ********************************************************************/

#if !defined(AFX_SETTINGS_H__E5078EAF_8BA8_4D6A_B2FD_CCA2C64F6005__INCLUDED_)
#define AFX_SETTINGS_H__E5078EAF_8BA8_4D6A_B2FD_CCA2C64F6005__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Settings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog


class CAnirRemote;


/*!
@par CLASS
    CSettings 

@par USAGE
<pre> Methods: 

</pre> 

<pre> Attributes: 

</pre> 

*/

//! Settings dialog
class CSettings : public CDialog
{
// Construction
public:
	CSettings(CAnirRemote *rem);

// Dialog Data
	//{{AFX_DATA(CSettings)
	enum { IDD = IDD_SETTINGS };
	CStatic	m_cStatus;
	CButton	m_cApply;
	CString	m_sName;
	int		m_iPort;
	BOOL	m_bSendUpEvent;
	BOOL	m_bSendRepeat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettings)
	afx_msg void OnAbout();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnApply();
	virtual void OnCancel();
	afx_msg void OnChangeSetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void updateUI();


private:
	//! Remote pointer
	CAnirRemote *m_pRemote;	
	//! Settings changed?
	bool m_bChanged;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGS_H__E5078EAF_8BA8_4D6A_B2FD_CCA2C64F6005__INCLUDED_)

/*********************************************************************
 *  @file   : About.h
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Declaration of class for about box
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#if !defined(AFX_ABOUT_H__B29B3C43_F76D_4482_B46B_972A31A17318__INCLUDED_)
#define AFX_ABOUT_H__B29B3C43_F76D_4482_B46B_972A31A17318__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAbout dialog

/*!
@par CLASS
    CAbout 

@par USAGE
<pre> Methods: 

</pre> 

<pre> Attributes: 

</pre> 

*/

//! Aboud dialog
class CAbout : public CDialog
{
// Construction
public:
	CAbout(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAbout)
	enum { IDD = IDD_ABOUT };
	CString	m_strVersion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAbout)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAbout)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUT_H__B29B3C43_F76D_4482_B46B_972A31A17318__INCLUDED_)

/*********************************************************************
 *  @file   : About.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : About box
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#include "stdafx.h"
#include "anir2girder.h"
#include "About.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAbout dialog



/*!
 * Constructor
 *
 * @param 
 *
 * @return  : 
 */
CAbout::CAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CAbout::IDD, pParent)
{
	DEBUG_MSG("CAbout::CAbout");

	//{{AFX_DATA_INIT(CAbout)
	m_strVersion = _T("");
	//}}AFX_DATA_INIT
}



/*!
 * Dialog data exchange
 *
 * @param pDX : 
 *
 * @return void  : 
 */
void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAbout)
	DDX_Text(pDX, IDC_STATIC_VERSION, m_strVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
	//{{AFX_MSG_MAP(CAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAbout message handlers


/*!
 * OK button pressed
 *
 * @param none
 *
 * @return void  : 
 */
void CAbout::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}


/*!
 * Initial dialog settings
 *
 * @param none
 *
 * @return BOOL  : 
 */
BOOL CAbout::OnInitDialog() 
{
	// Window text
	CString str;
	str.Format(_T("About - %s %s"), PLUGINNAME, PLUGINVERSION);
	SetWindowText(str);
	m_strVersion = PLUGINVERSION;

	// Window icon
	SetIcon(AfxGetApp()->LoadIcon(IDI_IR1), FALSE);
	

	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*********************************************************************
 *  @file   : Settings.cpp
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


#include "stdafx.h"
#include "anir2girder.h"
#include "Settings.h"
#include "About.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog



/*!
 * Constructor
 *
 * @param *rem : 
 *
 * @return  : 
 */
CSettings::CSettings(CAnirRemote *rem)
	: CDialog(CSettings::IDD, NULL)
{
	DEBUG_MSG("CSettings::CSettings");

	//{{AFX_DATA_INIT(CSettings)
	m_bSendUpEvent = TRUE;
	m_bSendRepeat = TRUE;
	//}}AFX_DATA_INIT

	m_pRemote = rem;
	m_bChanged = FALSE;


}



/*!
 * Dialog data exchange
 *
 * @param pDX : 
 *
 * @return void  : 
 */
void CSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettings)
	DDX_Control(pDX, IDC_STATIC_STATUS, m_cStatus);
	DDX_Control(pDX, IDB_APPLY, m_cApply);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_PORT, m_iPort);
	DDX_Check(pDX, IDC_CHECK_SENDUP, m_bSendUpEvent);
	DDX_Check(pDX, IDC_CHECK_REPEAT, m_bSendRepeat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettings, CDialog)
	//{{AFX_MSG_MAP(CSettings)
	ON_BN_CLICKED(IDB_ABOUT, OnAbout)
	ON_BN_CLICKED(IDB_APPLY, OnApply)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnChangeSetting)
	ON_EN_CHANGE(IDC_EDIT_PORT, OnChangeSetting)
	ON_BN_CLICKED(IDC_CHECK_SENDUP, OnChangeSetting)
	ON_BN_CLICKED(IDC_CHECK_REPEAT, OnChangeSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettings message handlers


/*!
 * About button pressed.
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::OnAbout() 
{
	CAbout dlg;
	dlg.DoModal();
}


/*!
 * Initial dialog settings
 *
 * @param none
 *
 * @return BOOL  : 
 */
BOOL CSettings::OnInitDialog() 
{
	// Window text
	CString str;
	str.Format(_T("Settings - %s %s"), PLUGINNAME, PLUGINVERSION);
	SetWindowText(str);

	// Window icon
	SetIcon(AfxGetApp()->LoadIcon(IDI_IR1), FALSE);


	// Populate data
	m_iPort = m_pRemote->getPort();
	m_sName = m_pRemote->getName();
	m_bSendUpEvent = m_pRemote->getEventOnUp();
	m_bSendRepeat = m_pRemote->getEventRepeat();
	
	CDialog::OnInitDialog();

	updateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/*!
 * OK button pressed
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::OnOK() 
{
	DEBUG_MSG("CSettings::OnOK()");
	
	if (m_bChanged)
		OnApply();

	CDialog::OnOK();
}


/*!
 * Apply button pressed
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::OnApply() 
{
	DEBUG_MSG("CSettings::OnApply()");

	UpdateData(TRUE);
	
	// Test new remote port
	m_pRemote->stop();
	m_pRemote->start(m_iPort);
	// Override (start don't set port on error)
	m_pRemote->setPort(m_iPort);

	// Name
	m_pRemote->setName(m_sName);

	// Event on button up?
	m_pRemote->setEventOnUp(m_bSendUpEvent);

	// Repeat event enabled?
	m_pRemote->setEventRepeat(m_bSendRepeat);

	m_bChanged = FALSE;
	updateUI();
}


/*!
 * Cancel button pressed
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::OnCancel() 
{
	DEBUG_MSG("CSettings::OnCancel()");

	CDialog::OnCancel();
}


/*!
 * Update UI (status icon and apply button)
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::updateUI()
{
	DEBUG_MSG("CSettings::updateUI()");

	// Updtae Status icon
	if (m_pRemote->isRunning())
		m_cStatus.SetIcon(AfxGetApp()->LoadIcon(IDI_OK));
	else
		m_cStatus.SetIcon(AfxGetApp()->LoadIcon(IDI_ERR));

	// Apply button enable
	m_cApply.EnableWindow(m_bChanged);
}


/*!
 * Settings are being changed
 *
 * @param none
 *
 * @return void  : 
 */
void CSettings::OnChangeSetting() 
{
	m_bChanged = TRUE;
	updateUI();
}

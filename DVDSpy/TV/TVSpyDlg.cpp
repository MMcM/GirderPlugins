// TVSpyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Slinkx.h"
#include "TVSpy.h"
#include "TVSpyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

  // Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTVSpyDlg dialog

CTVSpyDlg::CTVSpyDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CTVSpyDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CTVSpyDlg)
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTVSpyDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CTVSpyDlg)
  DDX_Control(pDX, IDC_CHAN_DESC, m_chanDesc);
  DDX_Control(pDX, IDC_VOLUME, m_volume);
  DDX_Control(pDX, IDC_MUTE, m_mute);
  DDX_Control(pDX, IDC_SAP, m_sap);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTVSpyDlg, CDialog)
//{{AFX_MSG_MAP(CTVSpyDlg)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_CHAN_UP, OnChanUp)
ON_BN_CLICKED(IDC_CHAN_DOWN, OnChanDown)
ON_BN_CLICKED(IDC_CHAN_BACK, OnChanBack)
ON_BN_CLICKED(IDC_CHAN_ENTER, OnChanEnter)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_VOLUME, OnReleasedcaptureVolume)
ON_BN_CLICKED(IDC_MUTE, OnMute)
ON_BN_CLICKED(IDC_SAP, OnSAP)
ON_WM_TIMER()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTVSpyDlg message handlers

BOOL CTVSpyDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL) {
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty()) {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);       // Set big icon
  SetIcon(m_hIcon, FALSE);      // Set small icon
	
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();

  ((CButton *)GetDlgItem(IDC_CHAN_UP))
    ->SetBitmap(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAN_UP)));
  ((CButton *)GetDlgItem(IDC_CHAN_DOWN))
    ->SetBitmap(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAN_DOWN)));
  ((CButton *)GetDlgItem(IDC_CHAN_BACK))
    ->SetBitmap(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAN_BACK)));
  ((CButton *)GetDlgItem(IDC_CHAN_ENTER))
    ->SetBitmap(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAN_ENTER)));

  if (!app->CanSetAudio()) {
    GetDlgItem(IDC_VOLUMEL)->ShowWindow(SW_HIDE);
    m_volume.ShowWindow(SW_HIDE);
    m_mute.ShowWindow(SW_HIDE);
    m_sap.ShowWindow(SW_HIDE);
  }
  else {
    m_volume.SetRange(0, 63);
    m_volume.SetPageSize(8);
    m_volume.SetTicFreq(8);
    m_volume.SetPos(app->m_volume);
    m_mute.SetCheck(app->m_mute);
    m_sap.SetCheck(app->m_sap);
  }
  
  SaveLastChan();
  UpdateChanDesc();

  m_bRemote = (NULL != DetectI2CDevice) && (NULL != ReadI2C) && 
    DetectI2CDevice(REMOTE_ADDR);

  m_pendingDigit = 0;
  m_pendingDigitTime = 0;

  SetTimer(1, 100, NULL);

  return TRUE;                  // return TRUE  unless you set the focus to a control
}

void CTVSpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
      CAboutDlg dlgAbout;
      dlgAbout.DoModal();
  }
  else {
    CDialog::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTVSpyDlg::OnPaint() 
{
  if (IsIconic()) {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else {
    CDialog::OnPaint();
  }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTVSpyDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

void CTVSpyDlg::SaveLastChan() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  m_lastChan = app->m_channel;
}

void CTVSpyDlg::UpdateChanDesc() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  m_chanDesc.SetWindowText(app->m_chanDesc);
}

void CTVSpyDlg::OnChanUp() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  SaveLastChan();
  app->NextChannel(FALSE);
  UpdateChanDesc();
}

void CTVSpyDlg::OnChanDown() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  SaveLastChan();
  app->NextChannel(TRUE);
  UpdateChanDesc();
}

void CTVSpyDlg::OnChanBack() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  int ochan = m_lastChan;
  SaveLastChan();
  app->SetChannel(ochan);
  UpdateChanDesc();
}

void CTVSpyDlg::OnChanEnter() 
{
  if (m_pendingDigit) {
    CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
    int chan = (m_pendingDigit - '0');
    m_pendingDigit = 0;
    SaveLastChan();
    app->SetChannel(chan);
    UpdateChanDesc();
  }	
}

void CTVSpyDlg::OnDigit(char digit)
{
  if (m_pendingDigit) {
    int chan = (m_pendingDigit - '0') * 10 + (digit - '0');
	m_pendingDigit = 0;
    CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
    SaveLastChan();
    app->SetChannel(chan);
    UpdateChanDesc();
  }
  else {
    m_pendingDigit = digit;
    m_pendingDigitTime = GetTickCount();
  }
}

void CTVSpyDlg::OnReleasedcaptureVolume(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  app->SetVolume(m_volume.GetPos());
	
  *pResult = 0;
}

void CTVSpyDlg::OnVolUp() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  int vol = app->m_volume;
  if (vol < 63)
    vol++;
  app->SetVolume(vol);
  m_volume.SetPos(app->m_volume);
}

void CTVSpyDlg::OnVolDown() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  int vol = app->m_volume;
  if (vol > 0)
    vol--;
  app->SetVolume(vol);
  m_volume.SetPos(app->m_volume);
}

void CTVSpyDlg::OnMute() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  app->SetMute(m_mute.GetCheck());
}

void CTVSpyDlg::OnSAP() 
{
  CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
  app->SetSAP(m_sap.GetCheck());
}

void CTVSpyDlg::OnTimer(UINT nIDEvent) 
{
  if (nIDEvent == 1) {
    CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();

    if (m_bRemote)
      CheckRemote();

    if (m_pendingDigit && 
        (GetTickCount() - m_pendingDigitTime > 1500)) {
      int chan = (m_pendingDigit - '0');
      m_pendingDigit = 0;
      SaveLastChan();
      app->SetChannel(chan);
      UpdateChanDesc();
    }

    app->CheckProgramEnd();
  }
  else
    CDialog::OnTimer(nIDEvent);
}

/*** ASUS Remote Layout
     80    88    90

     08  10  18  60
     20  28  30  68
     38  40  48  70
     50  00  58  78

            A0
        B0      B8
            A8

     C0  C8  D0  D8
     E0  E8  F0  F8 
 ***/

void CTVSpyDlg::CheckRemote()
{
  UINT cmd = ReadI2C(REMOTE_ADDR);
  if (cmd & 4)
    return;                     // Repeat
  cmd &= 0xF8;

  switch (cmd) {
  case 0x88:
    {
      CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
      app->SetMute(!app->m_mute);
      m_mute.SetCheck(app->m_mute);
    }
    break;
  case 0xC8:
    {
      CTVSpyApp *app = (CTVSpyApp *)AfxGetApp();
      app->SetSAP(!app->m_sap);
      m_sap.SetCheck(app->m_sap);
    }
    break;
  case 0x90:
    EndDialog(0);
    break;

  case 0x00:
    OnDigit('0');
    break;
  case 0x08:
    OnDigit('1');
    break;
  case 0x10:
    OnDigit('2');
    break;
  case 0x18:
    OnDigit('3');
    break;
  case 0x20:
    OnDigit('4');
    break;
  case 0x28:
    OnDigit('5');
    break;
  case 0x30:
    OnDigit('6');
    break;
  case 0x38:
    OnDigit('7');
    break;
  case 0x40:
    OnDigit('8');
    break;
  case 0x48:
    OnDigit('9');
    break;
  case 0x50:
    OnChanBack();
    break;
  case 0x58:
    OnChanEnter();
    break;
  case 0x60:
    OnChanUp();
    break;
  case 0x68:
    OnChanDown();
    break;
  case 0x70:
    OnVolUp();
    break;
  case 0x78:
    OnVolDown();
    break;
  }
}

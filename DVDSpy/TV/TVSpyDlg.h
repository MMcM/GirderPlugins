// TVSpyDlg.h : header file
//

#if !defined(AFX_TVSPYDLG_H__819C32B8_30D7_48C5_914F_D8124464826D__INCLUDED_)
#define AFX_TVSPYDLG_H__819C32B8_30D7_48C5_914F_D8124464826D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTVSpyDlg dialog

class CTVSpyDlg : public CDialog
{
  // Construction
public:
  CTVSpyDlg(CWnd* pParent = NULL);	// standard constructor

  // Dialog Data
  //{{AFX_DATA(CTVSpyDlg)
  enum { IDD = IDD_TVSPY_DIALOG };
  CStatic m_chanDesc;
  CSliderCtrl m_volume;
  CButton m_mute;
  CButton m_sap;
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTVSpyDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  //}}AFX_VIRTUAL

  // Operations
public:
  void SaveLastChan();
  void UpdateChanDesc();

  // Implementation
protected:
  void CheckRemote();

  HICON m_hIcon;
  BOOL m_bRemote;
  int m_lastChan;
  char m_pendingDigit;
  DWORD m_pendingDigitTime;

  // Generated message map functions
  //{{AFX_MSG(CTVSpyDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnChanUp();
  afx_msg void OnChanDown();
  afx_msg void OnChanBack();
  afx_msg void OnChanEnter();
  afx_msg void OnReleasedcaptureVolume(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnMute();
  afx_msg void OnSAP();
  afx_msg void OnTimer(UINT nIDEvent);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
  void OnDigit(char digit);
  void OnVolUp();
  void OnVolDown();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVSPYDLG_H__819C32B8_30D7_48C5_914F_D8124464826D__INCLUDED_)

// AutoProxy.h : header file
// $Header$

#if !defined(AFX_AUTOPROXY_H__3B78E31C_36C0_42B9_A3C3_55E4DDA73FA8__INCLUDED_)
#define AFX_AUTOPROXY_H__3B78E31C_36C0_42B9_A3C3_55E4DDA73FA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTVSpyDlg;

/////////////////////////////////////////////////////////////////////////////
// CTVSpyAutoProxy command target

class CTVSpyAutoProxy : public CCmdTarget
{
  DECLARE_DYNCREATE(CTVSpyAutoProxy)

  CTVSpyAutoProxy();         // protected constructor used by dynamic creation

  // Attributes
public:


  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTVSpyAutoProxy)
public:
  virtual void OnFinalRelease();
  //}}AFX_VIRTUAL

  // Implementation
protected:
  virtual ~CTVSpyAutoProxy();

  // Generated message map functions
  //{{AFX_MSG(CTVSpyAutoProxy)
  // NOTE - the ClassWizard will add and remove member functions here.
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
  DECLARE_OLECREATE(CTVSpyAutoProxy)

  // Generated OLE dispatch map functions
  //{{AFX_DISPATCH(CTVSpyAutoProxy)
  afx_msg void SetChannel(short channel);
  //}}AFX_DISPATCH
  DECLARE_DISPATCH_MAP()
  DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOPROXY_H__3B78E31C_36C0_42B9_A3C3_55E4DDA73FA8__INCLUDED_)

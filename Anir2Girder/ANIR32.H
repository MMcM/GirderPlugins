/*
Module : ANIR32.H
Purpose: Interface for an MFC class that encapsulates access
         to Animax's Multimedia Magic Remote Control Device
Created: PJN / 28-01-1998
History: None

Copyright (c) 1997 by PJ Naughter.  
All rights reserved.

*/


////////////////////////////////// Macros ///////////////////////////

#ifndef __ANIR32_H__
#define __ANIR32_H__



/////////////////////////// Classes /////////////////////////////////

//forward declaration
class CAnirRemoteControl;

class CAnirRemoteNotifyWnd : public CFrameWnd
{
public:
	CAnirRemoteNotifyWnd();
  void SetRemoteControl(CAnirRemoteControl* pRemoteControl) { m_pRemoteControl = pRemoteControl; };

protected:
	//{{AFX_VIRTUAL(CAnirRemoteNotifyWnd)
	//}}AFX_VIRTUAL

	DECLARE_DYNCREATE(CAnirRemoteNotifyWnd)
	virtual ~CAnirRemoteNotifyWnd();

	//{{AFX_MSG(CAnirRemoteNotifyWnd)
	//}}AFX_MSG
  afx_msg LRESULT OnRemoteMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

  CAnirRemoteControl* m_pRemoteControl;
};


class CAnirRemoteControl
{
public:
//Constructors / Destructors
  CAnirRemoteControl();
  ~CAnirRemoteControl();

  BOOL StartMonitoring(int nPort);
  BOOL IsStarted() const { return m_pRemoteThread != NULL; };
  BOOL StopMonitoring();
  int  GetPort() { return m_nPort; };

protected:
  //instance data
  HANDLE m_hPort;
  OVERLAPPED m_Overlapped;
  CEvent m_StopEvent;
  static __int64 sm_TimerFrequency;
  __int64 m_PrevTimeStamp;
  BYTE m_LastData;
  CArray<int, int&> m_nStates;
  int m_bPrevState;
  CWinThread* m_pRemoteThread;
  int m_nPort;
  BOOL m_bOpenedOK;
  CEvent m_OpenEvent;
  CAnirRemoteNotifyWnd* m_pNotifyWnd;

  //thread related
  void DoPossibleInterpret();
  virtual void ProcessReceivedData(BYTE data, BOOL bRepeat);
  static UINT MonitorRemoteControlThread(LPVOID pParam);
  BOOL WaitForStateChange();
  BOOL HandleStateChange();
  UINT Run();

  //signal detection functions
  inline BOOL IsPreSignal(int nIndex);
  inline BOOL IsBitSignal(int nIndex, BOOL& bBit);
  inline BOOL IsRepeatSignal(int nIndex);
  inline BOOL IsByteSignal(int nIndex, BYTE& Byte);
  inline void AddNewLineState(int nState);
  inline BOOL IsByteAnirID(int nIndex);

  //helper functions to determine various key states
  static inline BOOL IsKeyPressDown(BYTE data);
  static inline BOOL IsKeyPressUp(BYTE data);
  static inline BOOL IsKeyMouse(BYTE data);

  //opening / closing
  BOOL Open(int nPort);
  BOOL Close();

//status 
  BOOL IsOpen() const;


  //allow the notify class access to our internals
  friend class CAnirRemoteNotifyWnd;
};


#endif //__ANIR32_H__



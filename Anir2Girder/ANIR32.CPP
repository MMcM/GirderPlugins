/*
Module : ANIR32.CPP
Purpose: Implementation for an MFC class that encapsulates access
         to Animax's Multimedia Magic Remote Control Device
Created: PJN / 28-01-1998
History: PJN / 24-03-1998 1) Now using PostMessage instead of SendMessage
                             to transition from background thread to foreground
                          2) Code now compiles cleanly when built for UNICODE

Copyright (c) 1998 by PJ Naughter.  
All rights reserved.

*/




/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "anir32.h"


///////////////////////////////// Statics  & Locals ///////////////////////////
__int64 CAnirRemoteControl::sm_TimerFrequency = 0;
static const UINT nMsgRemote = ::RegisterWindowMessage(_T("AnirRemoteMessage"));


///////////////////////////////// defines /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////// Implementation ///////////////////////////////

CAnirRemoteControl::CAnirRemoteControl()
{
  m_hPort = INVALID_HANDLE_VALUE;
  m_PrevTimeStamp = 0;
  m_LastData = 0;
  m_nStates.SetSize(0, 100);
  m_bPrevState = -1;
  m_pRemoteThread = NULL;
  ZeroMemory(&m_Overlapped, sizeof(OVERLAPPED));
  m_nPort = 0;
}

CAnirRemoteControl::~CAnirRemoteControl()
{
  if (IsOpen())
    Close();
}

BOOL CAnirRemoteControl::Open(int nPort)
{
  //determine the timer frequency
  if (!sm_TimerFrequency)
  {
    LARGE_INTEGER Frequency;
    if (!QueryPerformanceFrequency(&Frequency))
    {
      TRACE(_T("CAnirRemoteControl::Open: Failed in call to get high resolution timer\n"));
      return FALSE;
    }
    sm_TimerFrequency = Frequency.QuadPart;
  }

  //open the serial port (using Overlapped IO)
  CString sPort;
  sPort.Format(_T("COM%d"), nPort);
  m_hPort = CreateFile(sPort, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if (m_hPort == INVALID_HANDLE_VALUE)
  {
    TRACE(_T("CAnirRemoteControl::Open: Failed to open serial port\n"));
    return FALSE;
  }

  //set up the serial port settings
  DCB dcb;
  ZeroMemory(&dcb, sizeof(DCB));
  if (!GetCommState(m_hPort, &dcb))
  {
    TRACE(_T("CAnirRemoteControl::Open: Failed in call to get comms state\n"));
    Close();
    return FALSE;
  }

  // Update DCB rate.
  dcb.fDtrControl = DTR_CONTROL_ENABLE;  //set the DTR line high to power up the receiver
  dcb.fRtsControl = RTS_CONTROL_ENABLE;  //set the RTS line high to power up the receiver  

  // Set new state.
  if (!SetCommState(m_hPort, &dcb))
  {
    TRACE(_T("CAnirRemoteControl::Open: Failed in call to set comms state\n"));
    Close();
    return FALSE;
  } 

  //tell the comms port to monitor changes in the DCD line
  if (!SetCommMask(m_hPort, EV_RLSD))
  {
    TRACE(_T("CAnirRemoteControl::Open: Failed to setup monitor on serial port\n"));
    Close();
    return FALSE;
  }

  //create a manual reset event for the OVERLAPPED IO structure
  m_Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  return TRUE;
}

BOOL CAnirRemoteControl::Close()
{
  if (!IsOpen())
    return FALSE;

  //destroy the overlapped event handle
  CloseHandle(m_Overlapped.hEvent);

  //close the comms port
  BOOL bSuccess = CloseHandle(m_hPort);

  //set the other values back to their default values
  m_hPort = INVALID_HANDLE_VALUE;
  m_PrevTimeStamp = 0;
  m_Overlapped.hEvent = INVALID_HANDLE_VALUE;

  return bSuccess;
}

BOOL CAnirRemoteControl::IsOpen() const
{
  return (m_hPort != INVALID_HANDLE_VALUE);
}

BOOL CAnirRemoteControl::WaitForStateChange()
{
  if (!IsOpen())
    return FALSE;

  //sit around waiting for changes in DCD line or 
  //the stop event to become signalled. The reason we need
  //to use overlapped IO is that this gives us a chance to 
  //wait on 2 synchronisation objects (via WaitForMultipleObjects)
  //instead of just the 1 if we were to open the comms port in
  //normal non-overlapped fashion
  DWORD dwEvtMask;
  BOOL bSuccess = WaitCommEvent(m_hPort, &dwEvtMask, &m_Overlapped);
  if (!bSuccess && GetLastError() != ERROR_IO_PENDING)
  {
    TRACE1("Failed in call to WaitCommEvent, Error: %d\n", GetLastError());
    return FALSE;
  }

  HANDLE hHandles[2];
  hHandles[0] = m_Overlapped.hEvent;
  hHandles[1] = m_StopEvent.m_hObject;
  DWORD dwRet = WaitForMultipleObjects(2, hHandles, FALSE, INFINITE);

  //if the stop event rather than the Overlapped event caused 
  //WaitForMultipleObjects to return then return FALSE to cause the 
  //worker thread to be exited
  return (dwRet == WAIT_OBJECT_0);
}

BOOL CAnirRemoteControl::HandleStateChange()
{
  if (!IsOpen())
    return FALSE;

  //retreive the status
  DWORD dwModemStat;
  if (!GetCommModemStatus(m_hPort, &dwModemStat))
    return FALSE;

  //act on it by adding it to the array of states
  if ((dwModemStat & MS_RLSD_ON))
    AddNewLineState(0);
  else
    AddNewLineState(1);

  //convert the array we have into something meaninful
  DoPossibleInterpret();

  return TRUE;
}

void CAnirRemoteControl::DoPossibleInterpret()
{
  //search for the first repeat signal
  BOOL bFoundRepeatSignal = FALSE;
  int nFoundRepeatIndex = -1;
  for (int i=0; i<m_nStates.GetSize() && !bFoundRepeatSignal; i++)
  {
    if (IsRepeatSignal(i))
    {
      bFoundRepeatSignal = TRUE;
      nFoundRepeatIndex = i;
    }
  }
  if (bFoundRepeatSignal)
  {
    //we have found the repeat signal somewhere in the array so
    //discard all the data received prior to it
    m_nStates.RemoveAt(0, nFoundRepeatIndex+4);

    //call the function to do something about it. We do it through the
    //notification wnd to get back to the calling thread
    m_pNotifyWnd->PostMessage(nMsgRemote, m_LastData, TRUE);
  }

  //never allow the states array to grow too large if there
  //is a pre signal in it or not 
  if (m_nStates.GetSize() > 100)
    m_nStates.RemoveAll();

  //search for the first pre signal
  BOOL bFoundPreSignal = FALSE;
  int nFoundIndex = -1;
  for (int j=0; j<m_nStates.GetSize() && !bFoundPreSignal; j++)
  {
    if (IsPreSignal(j))
    {
      bFoundPreSignal = TRUE;
      nFoundIndex = j;
    }
  }
  if (bFoundPreSignal)
  {
    //we have found the presignal somewhere in the array so
    //discard all the data received prior to it
    m_nStates.RemoveAt(0, nFoundIndex);
  }
  else
    return;

  //now try to interpret the information following the pre signal
  BYTE data;
  BYTE notdata;
  BOOL bFound = IsByteAnirID(2) &&
                IsByteSignal(18, data) &&
                IsByteSignal(34, notdata) && 
                (notdata == (BYTE) ~data);
  if (bFound)
  {
    //now that we have found a signal we can remove its data
    //from the states array
    m_nStates.RemoveAt(0, 26);

    //call the virtual function which does the actual action for 
    //the key pressed. We do it through the
    //notification wnd to get back to the calling thread
    m_LastData = data;
    ASSERT(m_pNotifyWnd);
    m_pNotifyWnd->PostMessage(nMsgRemote, data, FALSE);
  }
}

void CAnirRemoteControl::AddNewLineState(BOOL bState)
{
  if (bState == m_bPrevState)
    return;

  LARGE_INTEGER Counter;
  if (QueryPerformanceCounter(&Counter))
  {
    __int64 TimeStamp = (Counter.QuadPart * 1000000 / sm_TimerFrequency);
    int nValue;
    if (m_PrevTimeStamp)
      nValue = (int) (TimeStamp - m_PrevTimeStamp);
    else
      nValue = 0;
    m_nStates.Add(nValue);
    m_PrevTimeStamp = TimeStamp;
    m_bPrevState = bState;
  }
}

BOOL CAnirRemoteControl::IsPreSignal(int nIndex)
{
  //need at least 2 bits
  if (nIndex+1 > m_nStates.GetUpperBound())
    return FALSE;

  int nNowWidth = m_nStates[nIndex];
  int nNextWidth = m_nStates[nIndex+1];

  return ( (((nNowWidth >= 4000) && (nNowWidth <= 5000)) ||
            ((nNowWidth >= 2000) && (nNowWidth <= 2500))) &&
           ((nNextWidth >= 4000) && (nNextWidth <= 5000)));
}

BOOL CAnirRemoteControl::IsRepeatSignal(int nIndex)
{
  //need at least 4 bits
  if (nIndex+3 > m_nStates.GetUpperBound())
    return FALSE;

  int nWidth1 = m_nStates[nIndex];
  int nWidth2 = m_nStates[nIndex+1];
  int nWidth3 = m_nStates[nIndex+2];
  int nWidth4 = m_nStates[nIndex+3];

/*
  return   (nWidth1 <= 1000) &&
           ((nWidth2 >= 10000) && (nWidth2 <= 130000)) &&
           ((nWidth3 >= 1500)  && (nWidth3 <= 2700)) &&
           ((nWidth4 >= 1500)  && (nWidth4 <= 2700));

  int val =(nWidth1 <= 1200) &&
           ((nWidth2 >= 7500) && (nWidth2 <= 130000)) &&
           ((nWidth3 >= 1500)  && (nWidth3 <= 2700)) &&
           ((nWidth4 >= 1500)  && (nWidth4 <= 2700));

  if (val)
	  MYTRACE("%d %d %d %d\n",
	  	  nWidth1, nWidth2, nWidth3, nWidth4);
  return val;
*/
  return   (nWidth1 <= 1200) &&
           ((nWidth2 >= 7500) && (nWidth2 <= 130000)) &&
           ((nWidth3 >= 1500)  && (nWidth3 <= 2700)) &&
           ((nWidth4 >= 1500)  && (nWidth4 <= 2700));

}

BOOL CAnirRemoteControl::IsBitSignal(int nIndex, BOOL& bBit)
{
  //need at least 2 bits
  if (nIndex+1 > m_nStates.GetUpperBound())
    return FALSE;

  int nNowWidth = m_nStates[nIndex];
  int nNextWidth = m_nStates[nIndex+1];

  BOOL bNowInRange = (nNowWidth  <= 1200);
  BOOL bNextIsZero = (nNextWidth <= 1200);
  BOOL bNextIsOne =  ((nNextWidth > 1200) && (nNextWidth <= 2000));
  BOOL bSuccess = (bNowInRange && (bNextIsZero || bNextIsOne));
  if (bSuccess)
    bBit = bNextIsOne;

  return bSuccess;
}

BOOL CAnirRemoteControl::IsByteSignal(int nIndex, BYTE& Byte)
{
  //Convert the list of bits into a byte
  BOOL bBit1;
  BOOL bBit2;
  BOOL bBit3;
  BOOL bBit4;
  BOOL bBit5;
  BOOL bBit6;
  BOOL bBit7;
  BOOL bBit8;

  BOOL bSuccess = IsBitSignal(nIndex, bBit1) &&
                  IsBitSignal(nIndex+2, bBit2) &&
                  IsBitSignal(nIndex+4, bBit3) &&
                  IsBitSignal(nIndex+6, bBit4) &&
                  IsBitSignal(nIndex+8, bBit5) &&
                  IsBitSignal(nIndex+10, bBit6) &&
                  IsBitSignal(nIndex+12, bBit7) &&
                  IsBitSignal(nIndex+14, bBit8);

  if (bSuccess)
    Byte = (BYTE) ((bBit1 << 7) | (bBit2 << 6) | (bBit3 << 5) | (bBit4 << 4) |
                   (bBit5 << 3) | (bBit6 << 2) | (bBit7 << 1) | bBit8);


  return bSuccess;
}

BOOL CAnirRemoteControl::IsByteAnirID(int nIndex)
{
  //is the digit started at offset nIndex the AnirID
  BYTE data=0;
  return IsByteSignal(nIndex, data) && (data == 0x0F);
}

BOOL CAnirRemoteControl::IsKeyPressDown(BYTE data)
{
  return (data%2 == 1);
}

BOOL CAnirRemoteControl::IsKeyPressUp(BYTE data)
{
  return (data%2 == 0);
}

BOOL CAnirRemoteControl::IsKeyMouse(BYTE data)
{
  return (data & 0x1D) == 0x9;
}

void CAnirRemoteControl::ProcessReceivedData(BYTE /*data*/, BOOL /*bRepeat*/)
{
  //derive your own class from CAnirRemoteControl to do something 
  //with the received data
}

UINT CAnirRemoteControl::Run()
{
  //open up the port and signal that occurence to the calling code
  m_bOpenedOK = Open(m_nPort);
  m_OpenEvent.SetEvent();
  if (!m_bOpenedOK)
  {
    TRACE(_T("Failed to open serial port for monitoring remote control\n"));
    return 1;
  }

  //loop around handling the port events or requests to exit
  BOOL bContinueLoop = TRUE;
  while (bContinueLoop)
  {
    bContinueLoop = WaitForStateChange();
    if (bContinueLoop)
      HandleStateChange();
  }

  //close up when we are out of the loop
  Close();

  return 0;
}

UINT CAnirRemoteControl::MonitorRemoteControlThread(LPVOID pParam)
{
  //get the pointer to the port class and call its run method
  CAnirRemoteControl* pPort = (CAnirRemoteControl*) pParam;
  ASSERT(pPort);

  return pPort->Run();
}

BOOL CAnirRemoteControl::StartMonitoring(int nPort)
{
  TRACE(_T("Starting the remote control background thread\n"));

  //create the hidden window which will be used to relay messages back to 
  //this thread from the background thread
  m_pNotifyWnd = new CAnirRemoteNotifyWnd;
  if (!m_pNotifyWnd->Create(NULL, _T("")))
    return FALSE;
  m_pNotifyWnd->SetRemoteControl(this);

  BOOL bSuccess = FALSE;

  //store away the port and opened values
  m_nPort = nPort;
  m_bOpenedOK = FALSE;

  //spin off the background thread to handle the remote control
  m_pRemoteThread = AfxBeginThread(CAnirRemoteControl::MonitorRemoteControlThread, this, THREAD_PRIORITY_TIME_CRITICAL);
  if (m_pRemoteThread)
  {
    //wait for the event which signifies that the comms port has 
    //been opened successfully or otherwise
    CSingleLock sl(&m_OpenEvent, TRUE);
    bSuccess = m_bOpenedOK;
  }

  if (!bSuccess)
    m_nPort = 0;

  return bSuccess;
}

BOOL CAnirRemoteControl::StopMonitoring()
{
  TRACE(_T("Stopping the remote control background thread\n"));

  if (!IsStarted())
    return FALSE;

  //bring down the notify window
  m_pNotifyWnd->SendMessage(WM_CLOSE);
  m_pNotifyWnd = NULL;

  //signal the stop event and wait for the background
  //thread to exit
  m_StopEvent.SetEvent();
  WaitForSingleObject(m_pRemoteThread->m_hThread, INFINITE);

  //reset the port number back to an invalid value
  m_nPort = 0;

  return TRUE;
}

IMPLEMENT_DYNCREATE(CAnirRemoteNotifyWnd, CFrameWnd)

CAnirRemoteNotifyWnd::CAnirRemoteNotifyWnd()
{
}

CAnirRemoteNotifyWnd::~CAnirRemoteNotifyWnd()
{
}

BEGIN_MESSAGE_MAP(CAnirRemoteNotifyWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CAnirRemoteNotifyWnd)
	//}}AFX_MSG_MAP
  ON_REGISTERED_MESSAGE(nMsgRemote, OnRemoteMessage)
END_MESSAGE_MAP()

LRESULT CAnirRemoteNotifyWnd::OnRemoteMessage(WPARAM wParam, LPARAM lParam)
{
  ASSERT(m_pRemoteControl);
  m_pRemoteControl->ProcessReceivedData((BYTE) wParam, (BOOL) lParam);
  return 0L;
}
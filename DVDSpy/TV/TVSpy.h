// TVSpy.h : main header file for the TVSPY application
//

#if !defined(AFX_TVSPY_H__778AEA94_9F76_45ED_AFC9_8A2655DF0A0A__INCLUDED_)
#define AFX_TVSPY_H__778AEA94_9F76_45ED_AFC9_8A2655DF0A0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTVSpyApp:
// See TVSpy.cpp for the implementation of this class
//

class CChannel;

class CTVSpyApp : public CWinApp
{
public:
  CTVSpyApp();
  ~CTVSpyApp();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTVSpyApp)
public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  //}}AFX_VIRTUAL

  // Implementation
  void LoadProfileSettings();
  void SaveProfileSettings();
  void LoadChannels();
  BOOL InitI2C();
  BOOL InitATIDE();
  BOOL InitDVDSpy();
  BOOL InitSlinke();

  BOOL CanSetAudio();
  void SetMute(BOOL mute);
  void SetSAP(BOOL sap);
  void SetVolume(int volume);
  void SetFrequency(double freq);
  void SetChannel(int channel);
  void NextChannel(BOOL down);
  void CheckProgramEnd();
  enum switch_t { TV, CABLE };
  void SetSwitch(switch_t sw);
  void SetSTB(int channel);

  BOOL m_i2c;
  BOOL m_mute;
  BOOL m_sap;
  int m_volume;
  enum cable_t { ANTENNA, CATV_IRC, CATV_HRC } m_cable; // What is the tuner input?
  BOOL m_preferSTB;             // Over the tuner.
  int m_chanMinDigits;
  int m_chanDigitPause;
  CString m_cableEnterCommand;
  int m_channel;
  CString m_chanDesc;
  DWORD m_programEnd;
  CString m_channelFile;
  CChannel **m_channels;
  HWND m_dvdspy;
  CSlinkx m_slinke;
  CString m_cableDeviceFile;
  CString m_switchDeviceFile;
  CString m_switchCableCommand;
  CString m_switchTVCommand;
  BOOL m_ati;
  int m_guideOffset;
  int m_guideOffsetOnce;

  //{{AFX_MSG(CTVSpyApp)
  // NOTE - the ClassWizard will add and remove member functions here.
  //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

typedef void (*WriteI2C_t)(BYTE,BYTE);
typedef void (*WriteI2CByte_t)(BYTE,BYTE,BYTE);
typedef BOOL (*DetectI2CDevice_t)(BYTE);
typedef unsigned short (*ReadI2C_t)(BYTE);

extern WriteI2C_t WriteI2C;
extern WriteI2CByte_t WriteI2CByte;
extern DetectI2CDevice_t DetectI2CDevice;
extern ReadI2C_t ReadI2C;

const BYTE SWITCH_ADDR = 0x48;
const BYTE SWITCH_SOURCE = 0x02;
const BYTE SWITCH_TUNER = 0xE0;
const BYTE SWITCH_COMPOSITE = 0xE3;
const BYTE SWITCH_SVIDEO = 0xE6;
const BYTE TUNER_ADDR = 0xC0;
const BYTE TUNER_BAND = 0x8E;
const BYTE AUDIO_ADDR = 0x82;
const BYTE AUDIO_LEVEL_LEFT = 0x00;
const BYTE AUDIO_LEVEL_RIGHT = 0x01;
const BYTE AUDIO_BASS = 0x02;
const BYTE AUDIO_TREBLE = 0x03;
const BYTE AUDIO_SWITCH = 0x08;
const BYTE AUDIO_SWITCH_MUTE = 0xEF;
const BYTE AUDIO_SWITCH_UNMUTE = 0xCF;
const BYTE REMOTE_ADDR = 0x42;
const BYTE REMOTE_CONTROL = 0xFC;
const BYTE REMOTE_CONTROL_OFF = 0xE9;
const BYTE REMOTE_CONTROL_ON = 0xEB;

typedef BOOL (PASCAL *GetChannelName_t)(long chan, char *buf, size_t buflen);
typedef BOOL (PASCAL *GetProgramDetails_t)(long chan, const char *datetime,
                                           char *buf, size_t buflen);

extern GetChannelName_t GetChannelName;
extern GetProgramDetails_t GetProgramDetails;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVSPY_H__778AEA94_9F76_45ED_AFC9_8A2655DF0A0A__INCLUDED_)

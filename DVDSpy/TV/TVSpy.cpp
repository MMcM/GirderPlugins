/* TVSpy -- DVDSpy helper application.
 * $Header$
 * Get program information for display.
 * Switch external tuner using Slink-e.
 */

// tvspy.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "slinkx.h"
#include "TVSpy.h"
#include "TVSpyDlg.h"
#include "AutoProxy.h"
#include "AutoClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CChannel
{
public:
  CChannel(int number, const CString& name) 
    : m_number(number), m_name(name) {
    m_antenna = m_cable = m_stb = m_guide = 0;
  }
  ~CChannel() {}
  
  int m_number;
  CString m_name;
  int m_antenna, m_cable, m_stb, m_guide;
};

/////////////////////////////////////////////////////////////////////////////
// CTVSpyApp

BEGIN_MESSAGE_MAP(CTVSpyApp, CWinApp)
//{{AFX_MSG_MAP(CTVSpyApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG
ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTVSpyApp construction

CTVSpyApp::CTVSpyApp()
{
  m_channels = NULL;
}

CTVSpyApp::~CTVSpyApp()
{
  if (NULL != m_channels) {
    for (int i = 0; i < 256; i++)
      delete m_channels[i];
    delete [] m_channels;
  }
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTVSpyApp object

CTVSpyApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTVSpyApp initialization

BOOL CTVSpyApp::InitInstance()
{
  if (!AfxOleInit()) {
    AfxMessageBox("Failed to init OLE");
    return FALSE;
  }

  enum { NORMAL, SERVER, SET_CHANNEL } mode = NORMAL;
  int newChannel = 0;
  for (int i = 1; i < __argc; i++) {
    LPCTSTR pszParam = __targv[i];
    if (pszParam[0] == '-' || pszParam[0] == '/') {
      pszParam++;
      if (!strcmp(pszParam, "server"))
        mode = SERVER;
    }
    else {
      newChannel = atoi(pszParam);
      mode = SET_CHANNEL;
    }
  }

  if (SET_CHANNEL == mode) {
    // Try to rendezvous with existing server.
    CTVSpyAutoClient existing;
    COleException exc;
    if (existing.CreateDispatch(CTVSpyAutoProxy::guid, &exc)) {
      existing.SetChannel(newChannel);
      return FALSE;
    }
  }

  LoadProfileSettings();

  LoadChannels();

  InitI2C();
  InitATIDE();
  InitDVDSpy();
  InitSlinke();

  SetSAP(m_sap);
  //SetMute(m_mute); // Handled inside SetChannel
  SetVolume(m_volume);
  switch (mode) {
  case NORMAL:
    SetChannel(m_channel);
    break;
  case SET_CHANNEL:
    m_guideOffset = m_guideOffsetOnce;
    SetChannel(newChannel);
    return FALSE;
  }
  
#ifdef _AFXDLL
  Enable3dControls();         // Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  COleTemplateServer::RegisterAll();

  CTVSpyDlg dlg;
  m_pMainWnd = &dlg;
  dlg.DoModal();

  if (NULL != WriteI2CByte)
    WriteI2CByte(AUDIO_ADDR, AUDIO_SWITCH, AUDIO_SWITCH_MUTE);

  SaveProfileSettings();

  // Since the dialog has been closed, return FALSE so that we exit
  // the application, rather than start the application's message pump.
  return FALSE;
}

int CTVSpyApp::ExitInstance()
{
  m_slinke.ReleaseDispatch();
  return CWinApp::ExitInstance();
}

void CTVSpyApp::LoadProfileSettings()
{
  m_channelFile = GetProfileString("Settings", "ChannelFile", ""); // lineup.csv
  m_cable = (cable_t)GetProfileInt("Settings", "Cable", 0);
  m_preferSTB = GetProfileInt("Settings", "PreferSTB", TRUE);
  m_chanMinDigits = GetProfileInt("Settings", "ChannelMinDigits", 0);
  m_chanDigitPause = GetProfileInt("Settings", "ChannelDigitPause", 0);
  m_cableEnterCommand = GetProfileString("Settings", "CableEnterCommand", "");
  m_cableDeviceFile = GetProfileString("Settings", "CableDeviceFile", ""); // cable.cde
  m_switchDeviceFile = GetProfileString("Settings", "SwitchDeviceFile", ""); // switch.cde
  m_switchCableCommand = GetProfileString("Settings", "SwitchCableCommand", "cable");
  m_switchTVCommand = GetProfileString("Settings", "SwitchTVCommand", "tv");
  m_ati = GetProfileInt("Settings", "ATIGuide", TRUE);
  m_guideOffset = GetProfileInt("Settings", "GuideOffset", 0);
  m_guideOffsetOnce = GetProfileInt("Settings", "GuideOffsetOnce", 300);

  m_i2c = GetProfileInt("Settings", "I2C", FALSE);
  m_channel = GetProfileInt("Settings", "Channel", 2);
  m_volume = GetProfileInt("Settings", "Volume", 32);
  m_mute = GetProfileInt("Settings", "Mute", FALSE);
  m_sap = GetProfileInt("Settings", "SAP", FALSE);
}

void CTVSpyApp::SaveProfileSettings()
{
  WriteProfileString("Settings", "ChannelFile", m_channelFile);
  WriteProfileInt("Settings", "Cable", m_cable);
  WriteProfileInt("Settings", "PreferSTB", m_preferSTB);
  WriteProfileInt("Settings", "ChannelMinDigits", m_chanMinDigits);
  WriteProfileInt("Settings", "ChannelDigitPause", m_chanDigitPause);
  WriteProfileString("Settings", "CableEnterCommand", m_cableEnterCommand);
  WriteProfileString("Settings", "CableDeviceFile", m_cableDeviceFile);
  WriteProfileString("Settings", "SwitchDeviceFile", m_switchDeviceFile);
  WriteProfileString("Settings", "SwitchCableCommand", m_switchCableCommand);
  WriteProfileString("Settings", "SwitchTVCommand", m_switchTVCommand);
  WriteProfileInt("Settings", "ATIGuide", m_ati);
  WriteProfileInt("Settings", "GuideOffset", m_guideOffset);
  WriteProfileInt("Settings", "GuideOffsetOnce", m_guideOffsetOnce);

  WriteProfileInt("Settings", "I2C", m_i2c);
  WriteProfileInt("Settings", "Channel", m_channel);
  WriteProfileInt("Settings", "Volume", m_volume);
  WriteProfileInt("Settings", "Mute", m_mute);
  WriteProfileInt("Settings", "SAP", m_sap);
}

void CTVSpyApp::LoadChannels()
{
  if (m_channelFile.IsEmpty()) return;

  m_channels = new CChannel*[256];
  memset(m_channels, 0, sizeof(CChannel*) * 256);

  FILE *file = fopen(m_channelFile, "r");
  if (NULL == file) {
#ifdef _DEBUG
    AfxMessageBox("Cannot open channel file");
#endif
    return;
  }
  while (TRUE) {
    char line[1024];
    char *lp = fgets(line, sizeof(line), file);
    if (NULL == lp)
      break;
    char *np = strchr(lp, ',');
    if (NULL == np)
      continue;
    *np++ = '\0';
    unsigned long n = strtoul(np, &np, 10);
    if ((n == 0) || (n > 256))
      continue;
    CChannel *chan = new CChannel(n, lp);
    m_channels[n] = chan;
    for (int i = 1; i <= 4; i++) {
      np++;
      n = strtoul(np, &np, 10);
      if ((n == 0) || (n > 256))
        continue;
      switch (i) {
      case 1:
        chan->m_antenna = n;
        break;
      case 2:
        chan->m_cable = n;
        break;
      case 3:
        chan->m_stb = n;
        break;
      case 4:
        chan->m_guide = n;
        break;
      }
    }
  }
  fclose(file);
}

BOOL CTVSpyApp::CanSetAudio()
{
  return (NULL != WriteI2CByte);
}

void CTVSpyApp::SetMute(BOOL mute)
{
  m_mute = mute;
  if (NULL != WriteI2CByte) {
    WriteI2CByte(AUDIO_ADDR, AUDIO_SWITCH, 
                 (m_mute) ? AUDIO_SWITCH_MUTE : AUDIO_SWITCH_UNMUTE);
  }
}

void CTVSpyApp::SetSAP(BOOL sap)
{
  m_sap = sap;
  if (NULL != WriteI2C) {
    WriteI2C(0x42,
             (m_sap) ? 0xFF : 0xFE);
  }
}

void CTVSpyApp::SetVolume(int volume)
{
  m_volume = volume;
  if (NULL != WriteI2CByte) {
    BYTE varg = 0xC0 + volume;
    WriteI2CByte(AUDIO_ADDR, AUDIO_LEVEL_LEFT, varg);
    WriteI2CByte(AUDIO_ADDR, AUDIO_LEVEL_RIGHT, varg);
  }
}

void CTVSpyApp::SetFrequency(double freq)
{
  if (NULL == WriteI2CByte)
    return;

  unsigned long farg = (unsigned long)((freq+45.75+0.03125)*16.0);
  WriteI2CByte(TUNER_ADDR, (BYTE)(farg >> 8) & 0x7F, (BYTE)(farg & 0xFF));
  BYTE band;
  if (freq <= 160.00) band = 0xA0;
  else if (freq <= 454.00) band = 0x90;
  else band = 0x30;
  WriteI2CByte(TUNER_ADDR, TUNER_BAND, band);
}

#define countof(x) (sizeof(x)/sizeof(x[0]))

const double NTSC_M[] = {
  0.0,   55.25,  61.25,  67.25, 77.25,
  83.25, 175.25, 181.25, 187.25, 193.25,
  199.25, 205.25, 211.25, 471.25, 477.25,
  483.25, 489.25, 495.25, 501.25, 507.25,
  513.25, 519.25, 525.25, 531.25, 537.25,
  543.25, 549.25, 555.25, 561.25, 567.25,
  573.25, 579.25, 585.25, 591.25, 597.25,
  603.25, 609.25, 615.25, 621.25, 627.25,
  633.25, 639.25, 645.25, 651.25, 657.25,
  663.25, 669.25, 675.25, 681.25, 687.25,
  693.25, 699.25, 705.25, 711.25, 717.25,
  723.25, 729.25, 735.25, 741.25, 747.25,
  753.25, 759.25, 765.25, 771.25, 777.25,
  783.25, 789.25, 795.25, 801.25
};

// Incrementally Related Carrier (lower edge gets round number)
const double NTSC_M_cable_IRC[] = {
  73.25,  55.25,  61.25,  67.25, 77.25,
  83.25, 175.25, 181.25, 187.25, 193.25,
  199.25, 205.25, 211.25, 121.25, 127.25,
  133.25, 139.25, 145.25, 151.25, 157.25,
  163.25, 169.25, 217.25, 223.25, 229.25,
  235.25, 241.25, 247.25, 253.25, 259.25,
  265.25, 271.25, 277.25, 283.25, 289.25,
  295.25, 301.25, 307.25, 313.25, 319.25,
  325.25, 331.25, 337.25, 343.25, 349.25,
  355.25, 361.25, 367.25, 373.25, 379.25,
  385.25, 391.25, 397.25, 403.25, 409.25,
  415.25, 421.25, 427.25, 433.25, 439.25,
  445.25, 451.25, 457.25, 463.25, 469.25,
  475.25, 481.25, 487.25, 493.25, 499.25,
  505.25, 511.25, 517.25, 523.25, 529.25,
  535.25, 541.25, 547.25, 553.25, 559.25,
  565.25, 571.25, 577.25, 583.25, 589.25,
  595.25, 601.25, 607.25, 613.25, 619.25,
  625.25, 631.25, 637.25, 643.25,  91.25,
   97.25, 103.25, 109.25, 115.25, 649.25,
  655.25, 661.25, 667.25, 673.25, 679.25,
  685.25, 691.25, 697.25, 703.25, 709.25,
  715.25, 721.25, 727.25, 733.25, 739.25,
  745.25, 751.25, 757.25, 763.25, 769.25,
  775.25, 781.25, 787.25, 793.25, 799.25
};

// Harmonically Related Carrier (video carrier gets round number)
// Note that channels 5 and 6 go the opposite direction.  The rest are 1.25MHz off IRC.
const double NTSC_M_cable_HRC[] = {
  72.00,  54.00,  60.00,  66.00, 78.00,
  84.00, 174.00, 180.00, 186.00, 192.00,
  198.00, 204.00, 210.00, 120.00, 126.00,
  132.00, 138.00, 144.00, 150.00, 156.00,
  162.00, 168.00, 216.00, 222.00, 228.00,
  234.00, 240.00, 246.00, 252.00, 258.00,
  264.00, 270.00, 276.00, 282.00, 288.00,
  294.00, 300.00, 306.00, 312.00, 318.00,
  324.00, 330.00, 336.00, 342.00, 348.00,
  354.00, 360.00, 366.00, 372.00, 378.00,
  384.00, 390.00, 396.00, 402.00, 408.00,
  414.00, 420.00, 426.00, 432.00, 438.00,
  444.00, 450.00, 456.00, 462.00, 468.00,
  474.00, 480.00, 486.00, 492.00, 498.00,
  504.00, 510.00, 516.00, 522.00, 528.00,
  534.00, 540.00, 546.00, 552.00, 558.00,
  564.00, 570.00, 576.00, 582.00, 588.00,
  594.00, 600.00, 606.00, 612.00, 618.00,
  624.00, 630.00, 636.00, 642.00,  90.00,
   96.00, 102.00, 108.00, 114.00, 648.00,
  654.00, 660.00, 666.00, 672.00, 678.00,
  684.00, 690.00, 696.00, 702.00, 708.00,
  714.00, 720.00, 726.00, 732.00, 738.00,
  744.00, 750.00, 756.00, 762.00, 768.00,
  774.00, 780.00, 786.00, 792.00, 798.00
};

void CTVSpyApp::SetChannel(int channel)
{
  m_channel = channel;
  m_chanDesc.Format("%d", channel);
  m_programEnd = 0;

  CChannel *pchan = NULL;
  if ((NULL != m_channels) && (channel > 0) && (channel < 256))
    pchan = m_channels[channel];

  CString chanName;
  if (NULL != pchan) {
    chanName = pchan->m_name;
  }
  else if (NULL != GetChannelName) {
    LPSTR buf = chanName.GetBuffer(256);
    if (!GetChannelName(channel, buf, 256))
      *buf = '\0';
    chanName.ReleaseBuffer();
  }
  if (!chanName.IsEmpty()) {
    m_chanDesc += ' ';
    m_chanDesc += chanName;
  }
  
  if (NULL != m_dvdspy) {
    char buf[256];
    strcpy(buf, "TV.Chan");

    LPSTR nbuf = buf + strlen(buf) + 1;
    *nbuf++ = (chanName.IsEmpty()) ? 1 : 2;
    sprintf(nbuf, "%d", channel);
    nbuf += strlen(nbuf) + 1;
    if (!chanName.IsEmpty()) {
      strncpy(nbuf, chanName, sizeof(buf) - (nbuf - buf));
      nbuf += strlen(nbuf) + 1; 
    }
  
    COPYDATASTRUCT cd;
    cd.dwData = 0;
    cd.lpData = buf;
    cd.cbData = nbuf - buf;
    SendMessage(m_dvdspy, WM_COPYDATA, 0, (LPARAM)&cd);
  
    int guideChan = channel;
    if (NULL != pchan)
      guideChan = pchan->m_guide;
    if ((NULL != GetProgramDetails) && guideChan) {
      char guidetstr[16];
      if (0 == m_guideOffset)
        guidetstr[0] = '\0'; // Now
      else {
        time_t guidetime;
        time(&guidetime);
        guidetime += m_guideOffset;
        struct tm *guidetm = localtime(&guidetime);
        strftime(guidetstr, sizeof(guidetstr), "%Y%m%d%H%M", guidetm);
      }
      strcpy(buf, "TV.Program");
      nbuf = buf + strlen(buf) + 1;
      if (GetProgramDetails(guideChan, guidetstr, 
                            nbuf+1, sizeof(buf) - (nbuf - buf) - 1)) {
        *nbuf++ = 1;
        // The title (first field) might have a comma in it; work backwards.
        LPSTR ebuf = nbuf;
        for (int i = 0; i < 5; i++) {
          ebuf = strrchr(nbuf, ',');
          if (NULL == ebuf)
            break;
          *ebuf++ = '\0';
        }
        m_chanDesc += '\n';
        m_chanDesc += nbuf;

        if ((ebuf[12] == '\0') &&
            (ebuf[12+1+12] == '\0')) {
          // When the program ends, need to refresh details.
          time_t nowtime, starttime, endtime;
          struct tm starttm, endtm;
          DWORD nowticks;
          char ibuf[8];

          memset(&starttm, 0, sizeof(starttm));
          memcpy(ibuf, ebuf+10, 2);
          ibuf[2] = '\0';
          starttm.tm_min = atoi(ibuf);
          memcpy(ibuf, ebuf+8, 2);
          ibuf[2] = '\0';
          starttm.tm_hour = atoi(ibuf);
          memcpy(ibuf, ebuf+6, 2);
          ibuf[2] = '\0';
          starttm.tm_mday = atoi(ibuf);
          memcpy(ibuf, ebuf+4, 2);
          ibuf[2] = '\0';
          starttm.tm_mon = atoi(ibuf) - 1;
          memcpy(ibuf, ebuf, 4);
          ibuf[4] = '\0';
          starttm.tm_year = atoi(ibuf) - 1900;
          starttm.tm_isdst = -1;
          starttime = mktime(&starttm);

          memset(&endtm, 0, sizeof(endtm));
          memcpy(ibuf, ebuf+12+1+10, 2);
          ibuf[2] = '\0';
          endtm.tm_min = atoi(ibuf);
          memcpy(ibuf, ebuf+12+1+8, 2);
          ibuf[2] = '\0';
          endtm.tm_hour = atoi(ibuf);
          memcpy(ibuf, ebuf+12+1+6, 2);
          ibuf[2] = '\0';
          endtm.tm_mday = atoi(ibuf);
          memcpy(ibuf, ebuf+12+1+4, 2);
          ibuf[2] = '\0';
          endtm.tm_mon = atoi(ibuf) - 1;
          memcpy(ibuf, ebuf+12+1, 4);
          ibuf[4] = '\0';
          endtm.tm_year = atoi(ibuf) - 1900;
          endtm.tm_isdst = -1;
          endtime = mktime(&endtm);

          *(nbuf-1) = 2;
          nbuf += strlen(nbuf) + 1;
          strftime(nbuf, sizeof(buf) - (nbuf - buf), "%#I:%M", &starttm);
          nbuf += strlen(nbuf);
          *nbuf++ = '-';
          strftime(nbuf, sizeof(buf) - (nbuf - buf), "%#I:%M%p", &endtm);

          time(&nowtime);
          nowticks = GetTickCount();
          m_programEnd = nowticks + (endtime - nowtime) * 1000;
        }
        nbuf += strlen(nbuf) + 1; 
        cd.cbData = nbuf - buf;
        SendMessage(m_dvdspy, WM_COPYDATA, 0, (LPARAM)&cd);
      }
    }
  }

  BOOL stb = m_preferSTB;
  if (NULL != pchan) {
    if (m_cable)
      channel = pchan->m_cable;
    else
      channel = pchan->m_antenna;
    if ((stb || !channel) && pchan->m_stb)
      channel = pchan->m_stb;
    else
      stb = FALSE;
  }

  if (stb) {
    SetSTB(channel);
    SetSwitch(CABLE);
    if (NULL != WriteI2CByte) {
      WriteI2CByte(REMOTE_ADDR, REMOTE_CONTROL, REMOTE_CONTROL_OFF);
      WriteI2CByte(SWITCH_ADDR, SWITCH_SOURCE, SWITCH_SVIDEO);
    }
    return;
  }

  if (NULL != WriteI2CByte) {
    WriteI2CByte(AUDIO_ADDR, AUDIO_SWITCH, AUDIO_SWITCH_MUTE);
    WriteI2CByte(REMOTE_ADDR, REMOTE_CONTROL, REMOTE_CONTROL_ON);
    WriteI2CByte(SWITCH_ADDR, SWITCH_SOURCE, SWITCH_TUNER);
  }
  SetSwitch(TV);
  switch (m_cable) {
  case ANTENNA:
    if ((channel > 0) && (channel < countof(NTSC_M)))
      SetFrequency(NTSC_M[channel-1]);
    break;
  case CATV_IRC:
    if ((channel > 0) && (channel < countof(NTSC_M_cable_IRC)))
      SetFrequency(NTSC_M_cable_IRC[channel-1]);
    break;
  case CATV_HRC:
    if ((channel > 0) && (channel < countof(NTSC_M_cable_HRC)))
      SetFrequency(NTSC_M_cable_HRC[channel-1]);
    break;
  }
  if ((NULL != WriteI2CByte) && !m_mute)
    WriteI2CByte(AUDIO_ADDR, AUDIO_SWITCH, AUDIO_SWITCH_UNMUTE);
}

void CTVSpyApp::NextChannel(BOOL down)
{
  int chan = m_channel;
  while (TRUE) {
    if (down) {
      if (chan == 2)
        chan = 99;
      else
        chan--;
    }
    else {
      if (chan == 99)
        chan = 2;
      else
        chan++;
    }
    if (chan == m_channel)
      break;
    if ((NULL == m_channels) || (NULL != m_channels[chan]))
      break;
  }
  SetChannel(chan);
}

void CTVSpyApp::CheckProgramEnd()
{
  if (m_programEnd == 0)
    return;
  if (GetTickCount() > m_programEnd)
    SetChannel(m_channel);
}

WriteI2C_t WriteI2C = NULL;
WriteI2CByte_t WriteI2CByte = NULL;
DetectI2CDevice_t DetectI2CDevice = NULL;
ReadI2C_t ReadI2C = NULL;

BOOL CTVSpyApp::InitI2C()
{
  if (!m_i2c) return FALSE;

  HMODULE hmod = LoadLibrary("I2C.DLL");
  if (NULL == hmod) {
#ifdef _DEBUG
    AfxMessageBox("No I2C library");
#endif
    return FALSE;
  }

  WriteI2C = (WriteI2C_t)GetProcAddress(hmod, "WriteI2C");
  WriteI2CByte = (WriteI2CByte_t)GetProcAddress(hmod, "WriteI2CByte");
  DetectI2CDevice = (DetectI2CDevice_t)GetProcAddress(hmod, "DetectI2CDevice");
  ReadI2C = (ReadI2C_t)GetProcAddress(hmod, "ReadI2C");

  if (NULL == WriteI2CByte) {
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId) {
      WriteI2C = (WriteI2C_t)GetProcAddress(hmod, "WriteI2CNT");
      WriteI2CByte = (WriteI2CByte_t)GetProcAddress(hmod, "WriteI2CByteNT");
      DetectI2CDevice = (DetectI2CDevice_t)GetProcAddress(hmod, "DetectI2CDeviceNT");
      ReadI2C = (ReadI2C_t)GetProcAddress(hmod, "ReadI2CNT");
    }
    else {
      WriteI2C = (WriteI2C_t)GetProcAddress(hmod, "WriteI2C9X");
      WriteI2CByte = (WriteI2CByte_t)GetProcAddress(hmod, "WriteI2CByte9X");
      DetectI2CDevice = (DetectI2CDevice_t)GetProcAddress(hmod, "DetectI2CDevice9X");
      ReadI2C = (ReadI2C_t)GetProcAddress(hmod, "ReadI2C9X");
    }
    if (NULL == WriteI2CByte) {
#ifdef _DEBUG
      AfxMessageBox("I2C entry not found");
#endif
      return FALSE;
    }
  }

  return TRUE;
}

GetChannelName_t GetChannelName = NULL;
GetProgramDetails_t GetProgramDetails = NULL;

BOOL CTVSpyApp::InitATIDE()
{
  if (!m_ati) return FALSE;

  HKEY kpath;
  char buf[256];
  DWORD dtype, len;
  len = sizeof(buf);
  if ((ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Gemstar\\ETV\\PATH", 
                                     0, KEY_READ, &kpath)) ||
      (ERROR_SUCCESS != RegQueryValueEx(kpath, "ATIDATAENG_DLL", NULL,
                                        &dtype, (LPBYTE)buf, &len)) ||
      (REG_SZ != dtype)) {
#ifdef _DEBUG
    AfxMessageBox("Guide+ for ATI not installed properly");
#endif
    return FALSE;
  }
  RegCloseKey(kpath);
    
  HMODULE hlib = LoadLibrary(buf);
  if (NULL == hlib) {
#ifdef _DEBUG
    AfxMessageBox("DATAENG library not found");
#endif
    return FALSE;
  }

  GetChannelName = (GetChannelName_t)GetProcAddress(hlib, "de_GetChannelName");
  GetProgramDetails = (GetProgramDetails_t)GetProcAddress(hlib, 
                                                          "de_GetProgramDetails");
  if ((NULL == GetChannelName) || (NULL == GetProgramDetails)) {
#ifdef _DEBUG
    AfxMessageBox("DATAENG entry not found");
#endif
    return FALSE;
  }
  return TRUE;
}

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

BOOL CTVSpyApp::InitDVDSpy()
{
  m_dvdspy = NULL;

  EnumWindows(FindMonitorWindow, (LPARAM)&m_dvdspy);

  if (NULL == m_dvdspy) {
#ifdef _DEBUG
    AfxMessageBox("Girder DVDSpy not running");
#endif
    return FALSE;
  }

  return TRUE;
}

BOOL CTVSpyApp::InitSlinke()
{
  if (m_cableDeviceFile.IsEmpty() && m_switchDeviceFile.IsEmpty())
    return FALSE;
      
  if (!m_slinke.Create()) {
#ifdef _DEBUG
    AfxMessageBox("Could not create Slink-e control");
#endif
    return FALSE;
  }
  if (m_slinke.GetNumSlinkes() < 1) {
#ifdef _DEBUG
    AfxMessageBox("No active Slink-e");
#endif
    return FALSE;
  }
  // All IR outputs.
  if (!m_cableDeviceFile.IsEmpty()) {
    if (m_slinke.AddDevice("switch", m_switchDeviceFile, 1, 1, 0xFF0) < 0) {
#ifdef _DEBUG
      AfxMessageBox("Could not open Slink-e device file");
#endif
      return FALSE;
    }
  }
  if (!m_switchDeviceFile.IsEmpty()) {
    if (m_slinke.AddDevice("cable", m_cableDeviceFile, 1, 1, 0xFF0) < 0) {
#ifdef _DEBUG
      AfxMessageBox("Could not open Slink-e device file");
#endif
      return FALSE;
    }
  }
  return TRUE;
}

void CTVSpyApp::SetSwitch(switch_t sw)
{
  if (m_switchDeviceFile.IsEmpty()) return;

  m_slinke.Send("switch", (CABLE == sw) ? m_switchCableCommand : m_switchTVCommand);
}

void CTVSpyApp::SetSTB(int channel)
{
  if (m_cableDeviceFile.IsEmpty()) return;

  if (m_chanDigitPause > 0) Sleep(m_chanDigitPause);
  char buf1[8], buf2[8];
  sprintf(buf1, "%d", channel);
  int nd = strlen(buf1);
  while (nd < m_chanMinDigits) {
    m_slinke.Send("cable", "0");
    if (m_chanDigitPause > 0) Sleep(m_chanDigitPause);
    nd++;
  }
  char *pc = buf1;
  while (TRUE) {
    char ch = *pc++;
    if (ch == '\0')
      break;
    buf2[0] = ch;
    buf2[1] = '\0';
    m_slinke.Send("cable", buf2);
    if (m_chanDigitPause > 0) Sleep(m_chanDigitPause);
  }
  if (!m_cableEnterCommand.IsEmpty()) {
    m_slinke.Send("cable", m_cableEnterCommand);
    if (m_chanDigitPause > 0) Sleep(m_chanDigitPause);
  }
}

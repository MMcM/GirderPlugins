/* Multimedia Mixer Monitor
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

static UINT g_nmixers = 0;
static HMIXER* g_mixers = NULL;

void MixerMonitorInit(HWND hwndMonitor)
{
  g_nmixers = waveOutGetNumDevs();
  g_mixers = new HMIXER[g_nmixers];
  for (UINT i = 0; i < g_nmixers; i++) {
    if (MMSYSERR_NOERROR != mixerOpen(g_mixers + i, i, (DWORD_PTR)hwndMonitor,
                                      NULL, CALLBACK_WINDOW | MIXER_OBJECTF_WAVEOUT)) {
      g_mixers[i] = NULL;
    }
  }
}

void MixerMonitorMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (MM_MIXM_CONTROL_CHANGE != uMsg) return;

  HMIXER hMixer = (HMIXER)wParam;
  DWORD dwControlID = (DWORD)lParam;

  MIXERCAPS mxcaps;
  if (MMSYSERR_NOERROR != mixerGetDevCaps((UINT)hMixer, &mxcaps, sizeof(mxcaps)))
    return;
  
  MIXERCONTROL mxc;
  mxc.cbStruct = sizeof(mxc);
  MIXERLINECONTROLS mxlc;
  mxlc.cbStruct = sizeof(mxlc);
  mxlc.dwControlID = dwControlID;
  mxlc.cControls = 1;
  mxlc.cbmxctrl = sizeof(mxc);
  mxlc.pamxctrl = &mxc;
  if (MMSYSERR_NOERROR != mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, 
                                               MIXER_GETLINECONTROLSF_ONEBYID | 
                                               MIXER_OBJECTF_HMIXER))
    return;

  char szEvent[256];
  strncpy(szEvent, "Mixer.", sizeof(szEvent));
  if (g_nmixers > 1) {
    strncat(szEvent, mxcaps.szPname, sizeof(szEvent));
    strncat(szEvent, ".", sizeof(szEvent));
  }
  strncat(szEvent, mxc.szShortName  , sizeof(szEvent));
  
  union {
    MIXERCONTROLDETAILS_BOOLEAN b;
    MIXERCONTROLDETAILS_SIGNED s;
    MIXERCONTROLDETAILS_UNSIGNED u;
  } mxv;
  MIXERCONTROLDETAILS mxcd;
  mxcd.cbStruct = sizeof(mxcd);
  mxcd.dwControlID = dwControlID;
  mxcd.cChannels = 1;
  mxcd.cMultipleItems = mxc.cMultipleItems;
  mxcd.cbDetails = sizeof(mxv);
  mxcd.paDetails = &mxv;
  if (MMSYSERR_NOERROR != mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd,
                                                 MIXER_GETCONTROLDETAILSF_VALUE |
                                                 MIXER_OBJECTF_HMIXER))
    return;

  char szPayload[64];
  PCHAR pszPayload = szPayload;
  *pszPayload++ = 1;
  *pszPayload = '\0';
  switch (mxc.dwControlType & MIXERCONTROL_CT_UNITS_MASK) {
  case MIXERCONTROL_CT_UNITS_BOOLEAN:
    strcpy(pszPayload, (mxv.b.fValue) ? "TRUE" : "FALSE");
    break;
  case MIXERCONTROL_CT_UNITS_SIGNED:
    sprintf(pszPayload, "%ld", mxv.s.lValue);
    break;
  case MIXERCONTROL_CT_UNITS_UNSIGNED:
    sprintf(pszPayload, "%lu", mxv.u.dwValue);
    break;
  default:
    return;
  }

  GirderEvent(szEvent, szPayload, strlen(pszPayload) + 2);
}

void MixerMonitorClose()
{
  if (NULL == g_mixers) return;

  for (UINT i = 0; i < g_nmixers; i++) {
    if (NULL != g_mixers[i]) {
      mixerClose(g_mixers[i]);
    }
  }
  delete [] g_mixers;
  g_mixers = NULL;
}

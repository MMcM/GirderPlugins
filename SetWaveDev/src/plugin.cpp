/* Girder entry functions. */

#include "stdafx.h"
#include "plugin.h"

HINSTANCE g_hInstance;
s_functions SF;

extern "C" void WINAPI
gir_version(PCHAR buffer, BYTE length)
{
  strncpy(buffer, PLUGINVERSION, length);
}

extern "C" void WINAPI
gir_name(PCHAR buffer, BYTE length)
{
  strncpy(buffer, PLUGINNAME, length);
}

extern "C" void WINAPI
gir_description(PCHAR buffer, BYTE length)
{
  strncpy(buffer, "Change sound playback / record device.  Written by Mike McMahon (MMcM).", length);
}

extern "C" int WINAPI
gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" int WINAPI
gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI
gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
{
  if (p->size != sizeof(SF)) {
    return GIR_FALSE;
  }
  memcpy(&SF, p, p->size);
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_close()
{
  CloseCommandUI();
  return GIR_TRUE;
}

#if 0
extern "C" void WINAPI
gir_config()
{
}

extern "C" int WINAPI
gir_start()
{
}

extern "C" int WINAPI
gir_stop()
{
}

extern "C" int WINAPI
gir_compare(PCHAR orig, PCHAR recv)
{
  // Unlikely to need this.
  return strcmp(orig, recv);
}

extern "C" int WINAPI
gir_learn_event(char *oldevent, char *newevent, int len)
{
  return GIR_FALSE;
}
#endif

extern "C" int WINAPI
gir_event(p_command command, 
          char *eventString, void *payload, int len,
          char *status, int statuslen)
{
  switch (command->ivalue1) {
  case 0:
    {
      UINT ndevs = waveOutGetNumDevs();
      for (UINT dev = 0; dev < ndevs; dev++) {
        WAVEOUTCAPS caps;
        if (MMSYSERR_NOERROR != waveOutGetDevCaps(dev, &caps, sizeof(caps)))
          continue;
        
        if (!strcmp(caps.szPname, command->svalue1)) {
          waveOutMessage((HWAVEOUT)WAVE_MAPPER, DRVM_MAPPER+0x16, dev, 0);
          strncpy(status, "Playback device selected.", statuslen);
          return retContinue;
        }
      }
    }
    strncpy(status, "Playback device not found.", statuslen);
    break;
  case 1:
    {
      UINT ndevs = waveInGetNumDevs();
      for (UINT dev = 0; dev < ndevs; dev++) {
        WAVEINCAPS caps;
        if (MMSYSERR_NOERROR != waveInGetDevCaps(dev, &caps, sizeof(caps)))
          continue;
        if (!strcmp(caps.szPname, command->svalue1)) {
          waveInMessage((HWAVEIN)WAVE_MAPPER, DRVM_MAPPER+0x16, dev, 0);
          strncpy(status, "Record device selected.", statuslen);
          return retContinue;
        }
      }
    }
    strncpy(status, "Record device not found.", statuslen);
    break;
  default:
    MessageBox(0, "Unknown device selection", "Error", MB_ICONERROR);
    break;
  }
  return retContinue;
}

extern "C" void WINAPI
gir_command_gui()
{
  OpenCommandUI();
}

extern "C" void WINAPI
gir_command_changed(p_command command)
{
  UpdateCommandUI(command);
}

#if 0
extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  return GIR_TRUE;
}
#endif

/* Called by windows */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,  LPVOID lpReserved)
{
  switch(dwReason) {
  case DLL_PROCESS_ATTACH:
    g_hInstance = (HINSTANCE)hModule;
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

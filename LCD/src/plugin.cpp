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
  strncpy(buffer, "LCD display.  Written by Mike McMahon (MMcM).", length);
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
  DisplayCommand(command, 
                 status, statuslen);
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

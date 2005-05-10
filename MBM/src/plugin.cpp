/* Girder entry functions. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

HINSTANCE g_hInstance;
sFunctions3 SF;

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
  strncpy(buffer, "Girder MBM plugin", length);
}

extern "C" int WINAPI
gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" int WINAPI
gir_requested_api(int maxapi)
{
  return 3;
}

extern "C" int WINAPI
gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, pFunctions3 p)
{
  if (p->size != sizeof(SF)) {
    return GIR_FALSE;
  }
  memcpy(&SF, p, p->size);
  if (!DUIOpen())
    return GIR_FALSE;
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_close()
{
  DUIClose();
  CloseSharedMemory();
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_start()
{
  return StartMonitor();
}

extern "C" int WINAPI
gir_stop()
{
  StopMonitor();
  return GIR_TRUE;
}

#if 0
extern "C" int WINAPI
gir_compare(PCHAR orig, PCHAR recv)
{
  // Unlikely to need this.
  return strcmp(orig, recv);
}

extern "C" int WINAPI
gir_event(p_command command, 
          char *eventString, void *payload, int len,
          char *status, int statuslen)
{
  return retStopProcessing;
}

extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  return GIR_TRUE;
}
#endif

extern "C" void * WINAPI
gir_dynamic_ui(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
               int val1, int val2, void *userdata)
{
  switch (val1) {
  case duOnHookConfig:		
    DUIOpenConfig(tree);
    break;

  case duOnUnHookConfig:
    DUICloseConfig(tree);
    break;

  case duOnHookCommand:
    DUIOpenCommand(tree);
    break;

  case duOnUnHookCommand:
    DUICloseCommand(tree);
    break;
  }
  return NULL;
}

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

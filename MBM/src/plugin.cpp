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
gir_event(PFTreeNode node, CRITICAL_SECTION *cs, PEventElement event, 
          char *status, int statuslen)
{
  PCommand command;

  EnterCriticalSection(cs);

  command = (PCommand)node->Data;
  int result;

  switch (command->ActionSubType) {
  default:
    strncpy(status, "Unknown command subtype", statuslen);
    result = retStopProcessing;
  }

  LeaveCriticalSection(cs);
  return result;
}

extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  return GIR_TRUE;
}
#endif

extern "C" void * WINAPI
gir_dui(PFTree tree, PFTreeNode node, PBaseNode base, int duiIndex,
        int msg, WPARAM wParam, LPARAM lParam, void *userData)
{
  switch (msg) {
  case duOnHookConfig:		
    DUIOpenConfig(tree);
    break;

  case duOnUnHookConfig:
    DUICloseConfig(tree);
    break;
  }
  return NULL;
}

extern "C" PCHAR WINAPI
gir_eventstrings_advise()
{
  PCHAR result;
  size_t nb;
  for (int pass = 1; pass <= 2; pass++) {
    nb = 0;
    for (size_t i = 0; i < NSENSORS; i++) {
      LPCSTR event = GetEventName(i);
      if (NULL != event) {
        if (pass == 2)
          MemCopy(result + nb, event, strlen(event) + 1);
        nb += strlen(event) + 1;
      }
    }
    if (pass == 1)
      result = (PCHAR)Malloc(nb + 1);
    else
      result[nb++] = '\0';
  }
  return result;
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

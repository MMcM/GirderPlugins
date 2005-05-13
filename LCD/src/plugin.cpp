/* Girder entry functions. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

/* Global variables */
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
  return 3;
}

gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, pFunctions3 p)
{
  if (p->size != sizeof(SF)) {
    return GIR_FALSE;
  }
  memcpy(&SF, p, p->size);
  if (!DUIOpen())
    return GIR_FALSE;
  DisplayInitCS();
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_close()
{
  CloseConfigUI();
  DUIClose();
  DisplayClose();
  DisplayUnload();
  DisplayDeleteCS();
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_start()
{
  return DisplayEnableInput();
}

extern "C" int WINAPI
gir_stop()
{
  DisplayDisableInput();
  return GIR_TRUE;
}

#if 0
extern "C" int WINAPI
gir_compare(PCHAR orig, PCHAR recv)
{
  // Unlikely to need this.
  return strcmp(orig, recv);
}
#endif

extern "C" int WINAPI
gir_event(PFTreeNode node, CRITICAL_SECTION *cs, PEventElement event, 
          char *status, int statuslen)
{
  PCommand command;

  EnterCriticalSection(cs);

  command = (PCommand)node->Data;
  DisplayCommand(command, 
                 status, statuslen);
  
  LeaveCriticalSection(cs);
  return retContinue;
}

extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  switch (message) {
  case GIRINFO_SCRIPT_AFTER_STARTED:
    FunctionsOpen();
    break;
  case GIRINFO_SCRIPT_BEFORE_STOPPED:
    FunctionsClose();
    break;
  case GIRINFO_POWERBROADCAST:
    {
      static PVOID state = NULL;
      switch (wparam) {
      case PBT_APMQUERYSUSPEND:
        state = DisplaySave();
        /* falls through */
      case PBT_APMSUSPEND:
        DisplayClose();
        // TODO: This causes trouble if the config UI is active.
        // Perhaps just skip everything if it's open.
        DisplayUnload();
        break;
      case PBT_APMQUERYSUSPENDFAILED:
        if (NULL == state)
          break;
        /* else falls through */
      case PBT_APMRESUMESUSPEND:
      case PBT_APMRESUMEAUTOMATIC:
      case PBT_APMRESUMECRITICAL:
        DisplayRestore(state);
        state = NULL;
        break;
      }
    }
    break;
  }
  return GIR_TRUE;
}

extern "C" void * WINAPI
gir_dynamic_ui(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
               int val1, int val2, void *userdata)
{
  switch (val1) {
#if 0
  case duOnHookConfig:		
    DUIOpenConfig(tree);
    break;

  case duOnUnHookConfig:
    DUICloseConfig(tree);
    break;
#endif

  case duOnHookCommand:
    DUIOpenCommand(tree);
    break;

  case duOnUnHookCommand:
    DUICloseCommand(tree);
    break;
  }
  return NULL;
}

#if 0
extern "C" PCHAR WINAPI
gir_eventstrings_advise()
{
  // TODO: Could get button names.
}
#endif

#if 1
/** OLD STYLE **/
extern "C" void WINAPI
gir_config()
{
  OpenConfigUI();
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

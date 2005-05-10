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
  strncpy(buffer, "Girder PowerMate plugin", length);
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
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_start()
{
  return DeviceStart();
}

extern "C" int WINAPI
gir_stop()
{
  return DeviceStop();
}

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
  case ACTION_REPEAT:
    {
      PCHAR buf = ParseString("[pld1]");
      int n = strtol(buf, NULL, 10);
      SafeFree(buf);
      while (n-- > 0) {
        int ret = TriggerNodeEx(command->Action.lValue1, &command->Action.FileGUID1);
        if (retContinue != ret) return ret;
      }
    }
    result = retContinue;

  case ACTION_REQUEST:
    {
      PCHAR buf = ParseString(command->Action.sValue1);
      USHORT nVal = (USHORT)strtoul(buf, NULL, 0);
      SafeFree(buf);
      if (DeviceRequest((RequestType_t)strtol(command->Action.iValue1, NULL, 10), nVal))
        strncpy(status, "Request completed", statuslen);
      else
        strncpy(status, "Request failed", statuslen);
    }
    result = retContinue;

  default:
    strncpy(status, "Unknown command subtype", statuslen);
    result = retStopProcessing;
  }

  LeaveCriticalSection(cs);
  return result;
}

#if 0
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

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
  strncpy(buffer, "DVD Display Spy.  Written by Mike McMahon (MMcM).", length);
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

extern "C" PCHAR WINAPI
gir_eventstrings_advise()
{
  PCHAR result;
  size_t nb;

  char drives[128];
  GetLogicalDriveStrings(sizeof(drives), drives);

  HKEY hKey = NULL;
  RegOpenKey(HKEY_LOCAL_MACHINE, 
             DVDSPY_KEY "\\Events", 
             &hKey);

  size_t nmatches = DS_GetMatchCount();

  for (int pass = 1; pass <= 2; pass++) {
    nb = 0;
    {
      LPSTR pb = drives;
      while ('\0' != *pb) {
        if (DRIVE_CDROM == GetDriveType(pb)) {
          char cd = *pb;
          if ((cd >= 'a') && (cd <= 'z'))
            cd -= 'a' - 'A';  // Happens sometimes, maybe on XP?
          for (int i = 0; i < 3; i++) {
            char buf[256];
            strncpy(buf, "Disc.", sizeof(buf)-3);
            switch (i) {
            case 0:
              strncat(buf, "Contents", sizeof(buf)-3);
              break;
            case 1:
              strncat(buf, "Insert", sizeof(buf)-3);
              break;
            case 2:
              strncat(buf, "Eject", sizeof(buf)-3);
              break;
            }
            LPSTR pe = buf + strlen(buf);
            *pe++ = '.';
            *pe++ = cd;
            *pe++ = '\0';
            if (pass == 2)
              MemCopy(result + nb, (void *)buf, (pe - buf));
            nb += (pe - buf);
          }
        }
        pb += strlen(pb) + 1;
      }
    }
    if (NULL != hKey) {
      DWORD dwIndex = 0;
      while (TRUE) {
        char szName[128], szValue[128];
        DWORD dwType, dwNLen = sizeof(szName), dwLen = sizeof(szValue);
        if (ERROR_SUCCESS != RegEnumValue(hKey, dwIndex++, szName, &dwNLen,
                                          NULL, &dwType, (LPBYTE)szValue, &dwLen))
          break;
        if (REG_SZ != dwType) continue;
        if (dwLen > 1) {
          strncat(szName, " (", sizeof(szName));
          strncat(szName, szValue, sizeof(szName));
          strncat(szName, ")", sizeof(szName));
        }
        if (pass == 2)
          MemCopy(result + nb, szName, strlen(szName) + 1);
        nb += strlen(szName) + 1;
      }
    }
    for (size_t i = 0; i < nmatches; i++) {
      // Do ones not mentioned in the registry.
      size_t nindex = DS_GetMatchIndexCount(i);
      for (size_t j = 0; j < nindex; j++) {
        char szName[128];
        DS_GetName(i, j, szName, sizeof(szName));
        if ((NULL == hKey) ||
            (ERROR_SUCCESS != RegQueryValueEx(hKey, szName, 
                                              NULL, NULL, NULL, NULL))) {
          if (pass == 2)
            MemCopy(result + nb, szName, strlen(szName) + 1);
          nb += strlen(szName) + 1;
        }
      }
    }
    if (pass == 1)
      result = (PCHAR)Malloc(nb + 1);
    else
      result[nb++] = '\0';
  }

  if (NULL != hKey)
    RegCloseKey(hKey);

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

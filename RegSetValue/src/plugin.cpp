/* Girder entry functions. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

HINSTANCE g_hInstance;
s_functions SF;

PCHAR DecodeKeyValue(PCHAR value, PHKEY phkey)
{
  char buf[1024];
  strncpy(buf, value, sizeof(buf));
  
  LPSTR pkey, pval;
  pkey = strchr(buf, '\\');
  if (NULL != pkey)
    *pkey++ = '\0';
  else
    pkey = buf + strlen(buf);

  HKEY hkey;
  if (!strcmp(buf, "HKCU") || !strcmp(buf, "HKEY_CURRENT_USER"))
    hkey = HKEY_CURRENT_USER;
  else if (!strcmp(buf, "HKLM") || !strcmp(buf, "HKEY_LOCAL_MACHINE"))
    hkey = HKEY_LOCAL_MACHINE;
  else {
    sprintf(buf, "Unknown register root: %s", value);
    MessageBox(0, buf, "Error", MB_ICONERROR);
    return NULL;
  }

  pval = strrchr(pkey, '\\');
  if (NULL != pval)
    *pval++ = '\0';
  else
    pval = pkey + strlen(pkey);
  if (!strcmp(pval, "@"))       // Allow RegEdit convention for unnamed value.
    pval++;

  if (ERROR_SUCCESS != RegOpenKey(hkey, pkey, &hkey)) {
    sprintf(buf, "Key does not exist: %s", value);
    MessageBox(0, buf, "Error", MB_ICONERROR);
    return NULL;
  }
  
  *phkey = hkey;
  return value + (pval - buf);
}

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
  strncpy(buffer, "Change Windows registry value.  Written Mike McMahon (MMcM).", length);}

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
  HKEY hkey;

  strncpy(status, "Registry update error.", statuslen);

  EnterCriticalSection(&command->critical_section);

  PCHAR pval = DecodeKeyValue(command->svalue1, &hkey);
  if (NULL == pval) {
    LeaveCriticalSection(&command->critical_section);
    return retContinue;
  }

  DWORD dwType, dwValue, cbData;
  LPBYTE lpData;
  char buf[1024];

  dwType = command->ivalue1;
  if (!dwType && command->bvalue1) {
    // Old style numeric: no variables.
    dwValue = command->lvalue1;
    dwType = REG_DWORD;
  }
  else {
    SF.parse_reg_string(command->svalue2, buf, sizeof(buf));
    if (!dwType)
      dwType = REG_SZ;          // Old style string.
    else if (REG_DWORD == dwType) {
      // New style numeric: parse after expand; allow negative or
      // unsigned and 0x hex prefix.
      if ('-' == buf[0])
        dwValue = strtol(buf, NULL, 0);
      else
        dwValue = strtoul(buf, NULL, 0);
    }
  }
  if (REG_DWORD == dwType) {
    lpData = (LPBYTE)&dwValue;
    cbData = sizeof(dwValue);
  }
  else {
    lpData = (LPBYTE)buf;
    cbData = strlen(buf);
  }

  LONG rc = RegSetValueEx(hkey, pval, 0, dwType, lpData, cbData);
  
  LeaveCriticalSection(&command->critical_section);
  
  RegCloseKey(hkey);

  if (ERROR_SUCCESS != rc) {
    LeaveCriticalSection(&command->critical_section);
    MessageBox(0, "Error setting registry value", "Error", MB_ICONERROR);
    return retContinue;
  }

  strncpy(status, "Registry updated.", statuslen);
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

/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.1 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define GIRDER_CPP

#include <windows.h>
#include "plugin.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>



extern "C" int WINAPI  gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
{
	
  p_functions h = (p_functions )	p;
  if ( h->size != sizeof( s_functions ) )
    {
      return GIR_FALSE;
    }
		
  memcpy((void *)&SF, p, sizeof ( s_functions ));


  return GIR_TRUE;

}



extern "C" int WINAPI gir_close()
{

   Close_Config();

   return GIR_TRUE;
}




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



extern "C" int WINAPI gir_event(p_command command, char *eventstring, void * payload, int len, char * status,  int statuslen)
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


extern "C" void WINAPI gir_command_changed(p_command command)
{

  if ( command == NULL ) 
    {
      Enable_Config(FALSE);
    }
  else
    {
      Enable_Config(TRUE);
      CurCommand = command;	


      Update_Config();
    }

}

extern "C" void WINAPI gir_command_gui()
{

  Show_Config();
  
}


extern "C" int WINAPI gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" void WINAPI gir_description(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "Change Windows registry value.  Written Mike McMahon (MMcM).", Length);
}

extern "C" void WINAPI gir_name(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINNAME, Length);
}

extern "C" void WINAPI gir_version(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINVERSION, Length);
}




/* Called by windows */
BOOL WINAPI DllMain( HANDLE hModule, DWORD fdwreason,  LPVOID lpReserved )
{
  UNREFERENCED_PARAMETER(lpReserved);
  
  switch(fdwreason) {
  case DLL_PROCESS_ATTACH:
    hInstance=HINSTANCE(hModule);
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

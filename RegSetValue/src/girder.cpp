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
#include "girder.h"
#include "ui.h"
#include <stdio.h>


#ifdef _MSC_VER
  
#define __export

#else

#define __export _export

#endif



/*  This is the function which identifies this
  plugin as an software plugin. It gets called
  as soon as the plugin is loaded.

  In:
  Out:
  Return: Integer, the version of structure that this plugin uses.

*/

extern "C" int WINAPI __export init_dllex()
{

  return 5;  // We want version 5 structure

}

/* This function gets called right after the init_dllex has
   requested a specific type of header, this is then passed here.

   In: void *s : This hold a pointer to the requested struct
   Out:
   Return : integer : the plugin number
*/

extern "C" int WINAPI __export setsupportex(void *s)
{

  TFunctionsEx5 *p;

  p = (TFunctionsEx5 *) s;

  if ( p->dwSize != sizeof ( TFunctionsEx5 ) )
  {

    return GIR_ERROR; // ERROR 

  }

  memcpy( (void *)&SF, p, sizeof ( TFunctionsEx5));

  return PLUGINNUM;

}

/* Called right before Girder unloads the plugin, make 
   sure you cleaned everything up !!

*/

extern "C" void WINAPI __export close_dll()
{
  Close_Config();
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

/* This function will be called when an action should be executed!
  
  In: TCommand *command: The details of the command to be executed
  Out: PCHAR ReturnBuf: an reallocated buffer with 1000 bytes of space

  */
extern "C" void WINAPI __export command_execute(TCommand *command, PCHAR ReturnBuf)
{
  HKEY hkey;
  PCHAR pval = DecodeKeyValue(command->svalue1, &hkey);
  if (NULL == pval)
    return;
  LONG rc;
  if (command->bvalue1) {
    DWORD value = command->lvalue1;
    rc = RegSetValueEx(hkey, pval, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
  }
  else {
    char buf[1024];
    SF.ParseRegStringEx(command->svalue2, buf, sizeof(buf));
    rc = RegSetValueEx(hkey, pval, 0, REG_SZ, (LPBYTE)buf, strlen(buf));
  }
  RegCloseKey(hkey);
  if (ERROR_SUCCESS != rc) {
    MessageBox(0, "Error setting registry value", "Error", MB_ICONERROR);
    return;
  }
  strncpy(ReturnBuf, "Registry updated.", 1000);
}


/* When the user selects a different command in the treeview the
   plugin will be notified of this event via this function

   In : TCommand *command: the details of the new plugin
*/
extern "C" void WINAPI __export command_changed(TCommand  *command)
{
  CurCommand = command;
  Update_Config();
}


/* You should open a configuration dialog for the plugin
   when this function gets called.
  
  In : TCommand *command: the details of the new plugin
*/
extern "C" void WINAPI __export command_gui(TCommand *command)
{

  CurCommand = command;

  Show_Config();

}


/* You should disable / enable all control on the config
   dialog based on te bValue */

extern "C" void WINAPI __export command_controls(_BOOL bValue)
{
  Enable_Config(bValue);
}


/* You should erase all values from the configuration
   dialog. */
extern "C" void WINAPI __export command_clear()
{
  Empty_Config();

}


/* Return the pluginname 

   In: integer length : the max len of the name
   Out PCHAR buffer : the buffer you should fill

*/
extern "C" void WINAPI __export name_dll(PCHAR buffer, int length)
{

  strncpy(buffer, PLUGINNAME, length);

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

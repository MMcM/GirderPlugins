/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.0 Plugin                                                                  */
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
#include <mmsystem.h>
// From mmddk.h
#define DRVM_MAPPER 0x2000

#ifdef _MSC_VER
  
#define __export

#else

#define __export _export

#endif



/*  This is the function which identifies this
  plugin as an software plugin. It gets called
  as soon as the plugin is loaded. BUT when init_dllex
  is defined this function gets called with NULL as the argument
  As init_dllex adds a better method of using the plugin. 

  In:
  TFunctions : The functions that girder exports for usage in the plugin
  
  Out:
  None
   
  Return:
  Int : the Plugin device number
  

*/

extern "C" int WINAPI __export init_dll( TFunctions *gf )
{
  if ( gf!=NULL) {
    MessageBox(0, "This Plugin requires Girder 3.0.22+", "Error", MB_ICONERROR);
    return GIR_ERROR; 
  }
  else {
    return PLUGINNUM;
  }
}


/* If this function is defined init_dll will never be called BUT
   init_dll has to be in here for the plugin to work!!!!! 

  In:
  Out:
  Return: Integer, the version of structure that this plugin uses.

*/

extern "C" int WINAPI __export init_dllex()
{

  return 3;  // We want version 3 structure

}

/* This function gets called right after the init_dllex has
   requested a specific type of header, this is then passed here.

   In: void *s : This hold a pointer to the requested struct
   Out:
   Return : integer : the plugin number
*/

extern "C" int WINAPI __export setsupportex(void *s)
{
  TFunctionsEx3 *p;

  p = (TFunctionsEx3 *) s;

  if ( p->dwSize != sizeof ( TFunctionsEx3 ) )
  {
  
    return GIR_ERROR; // ERROR 

  }
  

  SetCommand    = p->SetCommand;
  ReallocPchar  = p->ReallocPchar;
  ShowOSD      = p->ShowOSD;
  TargetEnum    = p->TargetEnum;
  I18NTranslate  = p->I18NTranslate;
  HideOSD      = p->HideOSD;
  StartOSDDraw  = p->StartOSDDraw;
  StopOSDDraw    = p->StopOSDDraw;
  TreePickerShow  = p->TreePickerShow;
  TargetHWND    = p->TargetHWND;
  SetGirderReg  = p->SetGirderReg;
  GetGirderReg  = p->GetGirderReg;
  ParseRegString  = p->ParseRegString;
  GetLinkName     = p->GetLinkName;
  RegisterCB    = p->RegisterCB;

  return PLUGINNUM;
}

/* Called right before Girder unloads the plugin, make 
   sure you cleaned everything up !!

*/

extern "C" void WINAPI __export close_dll()
{
  Close_Config();
}

/* This function will be called when an action should be executed!
  
  In: TCommand *command: The details of the command to be executed
  Out: PCHAR ReturnBuf: an reallocated buffer with 1000 bytes of space

  */
extern "C" void WINAPI __export command_execute(TCommand *command, PCHAR ReturnBuf)
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
          strncpy(ReturnBuf, "Playback device selected.", 1000);
          return;
        }
      }
    }
    MessageBox(0, "Playback device not found", "Error", MB_ICONERROR);
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
          strncpy(ReturnBuf, "Record device selected.", 1000);
          return;
        }
      }
    }
    MessageBox(0, "Playback device not found", "Error", MB_ICONERROR);
    break;
  default:
    MessageBox(0, "Unknown device selection", "Error", MB_ICONERROR);
    break;
  }
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
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
#include <mmsystem.h>
// From mmddk.h
#define DRVM_MAPPER 0x2000



extern "C" int WINAPI gir_event(p_command command, PCHAR eventstring, void *payload, int len, PCHAR status, int statuslen)
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
          strncpy(status, "Playback device selected.", statuslen);
          return retContinue;
        }

      }
    }
    strncpy(status, "Playback device not found.", statuslen);
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
          strncpy(status, "Record device selected.", statuslen);
          return retContinue;
        }
      }
    }
    strncpy(status, "Record device not found.", statuslen);
    break;
  default:
    MessageBox(0, "Unknown device selection", "Error", MB_ICONERROR);
    break;
  }

  return retContinue;
}




extern "C" void WINAPI gir_command_changed(p_command  command)
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


/* You should open a configuration dialog for the plugin
   when this function gets called.
	
	In : TCommand *command: the details of the new plugin
*/
extern "C" void WINAPI gir_command_gui()
{

  Show_Config();

}


extern "C" int WINAPI  gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI  gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" void WINAPI  gir_description(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "Change sound playback / record device.  Written by Mike McMahon (MMcM).", Length);
}

extern "C" void WINAPI  gir_name(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINNAME, Length);
}

extern "C" void WINAPI gir_version(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINVERSION, Length);
}



extern "C" int WINAPI gir_close()
{

	
  Close_Config();

  return GIR_TRUE;
}




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

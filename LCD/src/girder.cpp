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
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include "lcdriver.h"

#ifdef _MSC_VER
  
#define __export

#else

#define __export _export

#endif

int nDisplayCols = 0, nDisplayRows = 0;
LPSTR pDisplayBuf = NULL;
BOOL bDisplayOpen = FALSE;

BOOL DisplayOpen(PCHAR retbuf)
{
  if (!bDisplayOpen) {
    if (LCD_RESULT_NOERROR != lcdRequest()) {
      strcpy(retbuf, "Could not open LCD.");
      return FALSE; 
    }

    nDisplayCols = lcdGetWidth();
    nDisplayRows = lcdGetHeight();
    pDisplayBuf = (LPSTR)malloc(nDisplayCols * nDisplayRows);
    memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);

    lcdClearDisplay();            // Clear
    lcdSetWrapMode(0, 0);         // Turn off wrap mode
    lcdSetCursor(0);              // Hide cursor

    bDisplayOpen = TRUE;
  }
  return bDisplayOpen;
}

void DisplayClose()
{
  if (bDisplayOpen) {
    free(pDisplayBuf);
    pDisplayBuf = NULL;

    lcdFree();

    bDisplayOpen = FALSE;
  }
}

void DisplayClose(TCommand *command, PCHAR retbuf)
{
  DisplayClose();
  strcpy(retbuf, "Display closed.");
}

void DisplayClear(TCommand *command, PCHAR retbuf)
{
  if (!bDisplayOpen && !DisplayOpen(retbuf)) 
    return;
  lcdClearDisplay();
  memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);
  strcpy(retbuf, "Display cleared.");
}

void DisplayCommon(LPCSTR str, TCommand *command, PCHAR retbuf)
{
  if (!DisplayOpen(retbuf)) 
    return;
  strncpy(retbuf, str, 1000);

  int row = command->lvalue1 % nDisplayRows;
  int col = command->lvalue2 % nDisplayCols;
  int width = nDisplayCols - col;
  if ((command->lvalue3 > 0) && (command->lvalue3 < width))
    width = command->lvalue3;
  LPSTR start = NULL, end;
  LPSTR bp = pDisplayBuf + (row * nDisplayCols) + col;
  while (width-- > 0) {
    char ch = *str;
    if ('\0' == ch)
      ch = ' ';
    else
      str++;
    if (ch != *bp) {
      if (NULL == start)
        start = bp;
      end = bp;
      *bp = ch;
    }
    else if (NULL == start)
      col++;
    bp++;
  }
  if (NULL == start) 
    return;    // No change

  end++;
  lcdSetCursorPos(col, row);
  lcdSendString(start, (end - start));

#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: @%d,%d: '", col, row);
    char *ep = dbuf + strlen(dbuf);
    size_t nb = end - start;
    if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
      nb = sizeof(dbuf) - (ep - dbuf) - 3;
    memcpy(ep, start, nb);
    strcpy(ep + nb, "'\n");
    OutputDebugString(dbuf);
  }
#endif
}

void DisplayString(TCommand *command, PCHAR retbuf)
{
  char buf[256];
  strncpy(buf, ParseRegString(command->svalue1), sizeof(buf));
  DisplayCommon(buf, command, retbuf);
}

void DisplayStringRegister(TCommand *command, PCHAR retbuf)
{
  DisplayCommon(GetGirderStrReg(command->ivalue2), command, retbuf);
}

void DisplayCurrentTime(TCommand *command, PCHAR retbuf)
{
  const char *fmt = command->svalue1;
  if ((NULL == fmt) || ('\0' == *fmt))
    fmt = "%H:%M:%S";

  time_t ltime;
  struct tm *now;
  char buf[128];
  
  time(&ltime);
  now = localtime(&ltime);
  strftime(buf, sizeof(buf), fmt, now);
  DisplayCommon(buf, command, retbuf);
}

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

  return 4;  // We want version 4 structure

}

/* This function gets called right after the init_dllex has
   requested a specific type of header, this is then passed here.

   In: void *s : This hold a pointer to the requested struct
   Out:
   Return : integer : the plugin number
*/

static char ArgPrefix[] = { (char)255, 0 };

extern "C" int WINAPI __export setsupportex(void *s)
{
  TFunctionsEx4 *p;

  p = (TFunctionsEx4 *) s;

  if ( p->dwSize != sizeof ( TFunctionsEx4 ) )
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
  SetGirderStrReg = p->SetGirderStrReg;
  GetGirderStrReg = p->GetGirderStrReg;

  return PLUGINNUM;
}

/* Called right before Girder unloads the plugin, make 
   sure you cleaned everything up !!

*/

extern "C" void WINAPI __export close_dll()
{
  DisplayClose();

  Close_Config();
}

/* This function will be called when an action should be executed!
  
  In: TCommand *command: The details of the command to be executed
  Out: PCHAR ReturnBuf: an reallocated buffer with 1000 bytes of space

  */
extern "C" void WINAPI __export command_execute(TCommand *command, PCHAR ReturnBuf)
{
  (*DisplayActions[command->ivalue1].function)(command, ReturnBuf);
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

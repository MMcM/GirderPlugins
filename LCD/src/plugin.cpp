/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.2 Plugin                                                                  */
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
#include <malloc.h>
#include <time.h>

#include "lcdriver.h"


int nDisplayCols = 0, nDisplayRows = 0;
LPSTR pDisplayBuf = NULL;
BOOL bDisplayOpen = FALSE;

class DisplayCommandState
{
public:
  p_command m_command;
  PCHAR m_eventstring;
  void *m_payload;
  int m_len;
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(p_command command, PCHAR eventstring, 
                      void *payload, int len, 
                      PCHAR status, int statuslen)
    : m_command(command), m_eventstring(eventstring), 
      m_payload(payload), m_len(len),
      m_status(status), m_statuslen(statuslen) 
  {
    EnterCriticalSection(&m_command->critical_section);    
  }

  ~DisplayCommandState() 
  {
    LeaveCriticalSection(&m_command->critical_section);
  }
  
  void SetStatus(LPCSTR status)
  {
    strncpy(m_status, status, m_statuslen);
  }

  PCHAR GetPayloadString(int index);
};

PCHAR DisplayCommandState::GetPayloadString(int index)
{
  PCHAR pld = (PCHAR)m_payload;
  int len = m_len;
  if (len-- <= 0) 
    return NULL;
  int nstr = *pld++;
  if ((index < 1) || (index > nstr))
    return NULL;
  while (--index > 0) {
    while (TRUE) {
      if (len-- <= 0)
        return NULL;
      if ('\0' == *pld++)
        break;
    }
  }
  return pld;
}

BOOL DisplayOpen(DisplayCommandState& state)
{
  if (!bDisplayOpen) {
    if (LCD_RESULT_NOERROR != lcdRequest()) {
      state.SetStatus("Could not open LCD.");
      return FALSE; 
    }

    nDisplayCols = lcdGetWidth();
    nDisplayRows = lcdGetHeight();
    pDisplayBuf = (LPSTR)malloc(nDisplayCols * nDisplayRows);
    memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);

    lcdSetCursor(0);              // Hide cursor
    lcdSetWrapMode(0, 0);         // Turn off wrap mode
    lcdClearDisplay();            // Clear

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

void DisplayClose(DisplayCommandState& state)
{
  DisplayClose();
  state.SetStatus("Display closed.");
}

void DisplayClear(DisplayCommandState& state)
{
  if (!bDisplayOpen && !DisplayOpen(state)) 
    return;
  lcdClearDisplay();
  memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);
  state.SetStatus("Display cleared.");
}

void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) 
    return;
  state.SetStatus(str);

  int row = state.m_command->lvalue1 % nDisplayRows;
  int col = state.m_command->lvalue2 % nDisplayCols;
  int width = nDisplayCols - col;
  if ((state.m_command->lvalue3 > 0) && (state.m_command->lvalue3 < width))
    width = state.m_command->lvalue3;
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

void DisplayString(DisplayCommandState& state)
{
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  DisplayCommon(state, buf);
}

void DisplayStringRegister(DisplayCommandState& state)
{
  char buf[1024];
  char regname[100];
  sprintf(regname, "treg%d",state.m_command->ivalue2);
  SF.get_string_var(regname, buf, sizeof(buf));

  DisplayCommon(state, buf);
}

void DisplayPayload(DisplayCommandState& state)
{
  PCHAR pld = state.GetPayloadString(state.m_command->ivalue2);
  if (NULL != pld)
    DisplayCommon(state, pld);
}

void DisplayFilenamePayload(DisplayCommandState& state)
{
  PCHAR pld = state.GetPayloadString(state.m_command->ivalue2);
  if (NULL == pld)
    return;
  PCHAR sp = strrchr(pld, '\\');
  if (NULL != sp)
    sp++;
  else if (NULL != strstr(pld, "://")) // A URL
    sp = strrchr(pld, '/') + 1;
  else
    sp = pld;
  PCHAR ep = strrchr(sp, '.');
  if (NULL != ep)
    *ep = '\0';
  DisplayCommon(state, sp);
}

void DisplayCurrentTime(DisplayCommandState& state)
{
  const char *fmt = state.m_command->svalue1;
  if ((NULL == fmt) || ('\0' == *fmt))
    fmt = "%H:%M:%S";

  time_t ltime;
  struct tm *now;
  char buf[128];
  
  time(&ltime);
  now = localtime(&ltime);
  strftime(buf, sizeof(buf), fmt, now);
  DisplayCommon(state, buf);
}



static char ArgPrefix[] = { (char)255, 0 };


extern "C" int WINAPI gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
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
  DisplayClose();

  Close_Config();

  return GIR_TRUE;

}

/* This function will be called when an action should be executed!
  
  In: TCommand *command: The details of the command to be executed
  Out: PCHAR ReturnBuf: an reallocated buffer with 1000 bytes of space

  */
extern "C" int WINAPI gir_event(p_command command, PCHAR eventstring, void *payload, int len, PCHAR status, int statuslen)
{
  DisplayCommandState state(command, eventstring,
                            payload, len,
                            status, statuslen);
  (*DisplayActions[command->ivalue1].function)(state);
  return retContinue;
}

/* When the user selects a different command in the treeview the
   plugin will be notified of this event via this function

   In : TCommand *command: the details of the new plugin
*/

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
  strncpy(Buffer, "LCD display.  Written by Mike McMahon (MMcM).", Length);
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
